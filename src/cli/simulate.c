/* SPDX-License-Identifier: MIT */
/**
 * @file simulate.c
 * @brief `battleship-cli simulate`: measures how strong each AI level is.
 *
 * Two experiments, both fully determined by the seed:
 *  - solo: an AI fires at a random fleet until it is sunk; fewer shots is
 *    better. This isolates the quality of the strategy.
 *  - duel: two AIs play real matches with the "hit, shoot again" rule; each
 *    pairing is played from both seats so the first-move advantage cancels.
 */
#include "cli.h"

#include "battleship/battleship.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* A solo run needs between BS_FLEET_DECKS and BS_CELL_COUNT shots. */
#define HISTOGRAM_SIZE (BS_CELL_COUNT + 1)

typedef struct solo_result {
    long histogram[HISTOGRAM_SIZE]; /* histogram[n]: games finished in n shots */
    int games;
} solo_result;

/* Shots an AI needs to sink a random fleet, or -1 on an engine error. */
static int solo_game(bs_difficulty level, bs_rng *rng)
{
    bs_board board;
    if (bs_board_place_fleet_random(&board, rng) != BS_OK) {
        return -1;
    }
    for (int shots = 1; shots <= BS_CELL_COUNT; shots++) {
        bs_knowledge knowledge;
        bs_coord target;
        bs_shot_report report;

        bs_board_get_knowledge(&board, &knowledge);
        if (bs_ai_choose_target(level, &knowledge, rng, &target) != BS_OK ||
            bs_board_fire(&board, target, &report) != BS_OK) {
            return -1;
        }
        if (report.fleet_destroyed) {
            return shots;
        }
    }
    return -1;
}

static bool run_solo(bs_difficulty level, int games, uint64_t seed, solo_result *result)
{
    bs_rng rng;
    /* One independent stream per level, so that levels can be run separately. */
    bs_rng_seed(&rng, seed + (uint64_t)level);

    *result = (solo_result){.games = games};
    for (int game = 0; game < games; game++) {
        const int shots = solo_game(level, &rng);
        if (shots < 0) {
            return false;
        }
        result->histogram[shots]++;
    }
    return true;
}

/* Smallest shot count reached by at least the given share of the games. */
static int percentile(const solo_result *result, double share)
{
    const double wanted = share * (double)result->games;
    long cumulated = 0;
    for (int shots = 0; shots < HISTOGRAM_SIZE; shots++) {
        cumulated += result->histogram[shots];
        if ((double)cumulated >= wanted && cumulated > 0) {
            return shots;
        }
    }
    return HISTOGRAM_SIZE - 1;
}

static void print_solo_row(bs_difficulty level, const solo_result *result)
{
    double sum = 0.0;
    double sum_of_squares = 0.0;
    int minimum = HISTOGRAM_SIZE;
    int maximum = 0;

    for (int shots = 0; shots < HISTOGRAM_SIZE; shots++) {
        const double count = (double)result->histogram[shots];
        if (result->histogram[shots] == 0) {
            continue;
        }
        sum += count * shots;
        sum_of_squares += count * shots * shots;
        if (shots < minimum) {
            minimum = shots;
        }
        maximum = shots;
    }

    const double mean = sum / result->games;
    const double variance = sum_of_squares / result->games - mean * mean;
    printf("%-8s %7.2f %7.2f %5d %5d %5d %5d\n", bs_difficulty_name(level), mean,
           sqrt(variance > 0.0 ? variance : 0.0), minimum, percentile(result, 0.50),
           percentile(result, 0.90), maximum);
}

/*
 * One match between two AIs. @p first opens; a hit grants another shot.
 * Returns the seat (0 or 1) of the winner, or -1 on an engine error.
 */
static int duel_game(const bs_difficulty levels[2], bs_rng *rng)
{
    bs_board boards[2]; /* boards[i] is the fleet of seat i */
    if (bs_board_place_fleet_random(&boards[0], rng) != BS_OK ||
        bs_board_place_fleet_random(&boards[1], rng) != BS_OK) {
        return -1;
    }

    int seat = 0;
    for (int shot = 0; shot < 2 * BS_CELL_COUNT; shot++) {
        bs_board *enemy = &boards[1 - seat];
        bs_knowledge knowledge;
        bs_coord target;
        bs_shot_report report;

        bs_board_get_knowledge(enemy, &knowledge);
        if (bs_ai_choose_target(levels[seat], &knowledge, rng, &target) != BS_OK ||
            bs_board_fire(enemy, target, &report) != BS_OK) {
            return -1;
        }
        if (report.fleet_destroyed) {
            return seat;
        }
        if (report.result == BS_SHOT_MISS) {
            seat = 1 - seat;
        }
    }
    return -1;
}

/* Share of matches (in percent) won by @p row against @p column. */
static double duel_win_rate(bs_difficulty row, bs_difficulty column, int games, uint64_t seed)
{
    bs_rng rng;
    bs_rng_seed(&rng, seed ^ (UINT64_C(0xD1CE) + (uint64_t)row * 16 + (uint64_t)column));

    int wins = 0;
    for (int game = 0; game < games; game++) {
        /* Alternate seats: the side that opens has a small edge. */
        const bool row_opens = game % 2 == 0;
        const bs_difficulty levels[2] = {row_opens ? row : column, row_opens ? column : row};
        const int winner = duel_game(levels, &rng);
        if (winner < 0) {
            return -1.0;
        }
        wins += (winner == 0) == row_opens;
    }
    return 100.0 * wins / games;
}

static int print_csv(const cli_options *options)
{
    puts("level,shots,games");
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        if (options->level_given && (bs_difficulty)level != options->level) {
            continue;
        }
        solo_result result;
        if (!run_solo((bs_difficulty)level, options->games, options->seed, &result)) {
            return EXIT_FAILURE;
        }
        for (int shots = BS_FLEET_DECKS; shots < HISTOGRAM_SIZE; shots++) {
            printf("%s,%d,%ld\n", bs_difficulty_name((bs_difficulty)level), shots,
                   result.histogram[shots]);
        }
    }
    return EXIT_SUCCESS;
}

int cli_simulate(const cli_options *options)
{
    if (options->csv) {
        return print_csv(options);
    }

    printf("Battleship %s - AI benchmark\n", bs_version());
    printf("%d games per level, seed %llu\n\n", options->games, (unsigned long long)options->seed);

    puts("Shots needed to sink a random fleet (20 decks on 100 cells; lower is better)");
    puts("level       mean  stddev   min   p50   p90   max");
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        if (options->level_given && (bs_difficulty)level != options->level) {
            continue;
        }
        solo_result result;
        if (!run_solo((bs_difficulty)level, options->games, options->seed, &result)) {
            fprintf(stderr, "battleship-cli: simulation failed\n");
            return EXIT_FAILURE;
        }
        print_solo_row((bs_difficulty)level, &result);
    }

    if (!options->duel || options->level_given) {
        return EXIT_SUCCESS;
    }

    puts("\nHead-to-head: matches won by the row level against the column level (%)");
    printf("%-8s", "");
    for (int column = 0; column < BS_DIFFICULTY_COUNT; column++) {
        printf(" %7s", bs_difficulty_name((bs_difficulty)column));
    }
    putchar('\n');
    for (int row = 0; row < BS_DIFFICULTY_COUNT; row++) {
        printf("%-8s", bs_difficulty_name((bs_difficulty)row));
        for (int column = 0; column < BS_DIFFICULTY_COUNT; column++) {
            const double rate = duel_win_rate((bs_difficulty)row, (bs_difficulty)column,
                                              options->games, options->seed);
            if (rate < 0.0) {
                fprintf(stderr, "\nbattleship-cli: simulation failed\n");
                return EXIT_FAILURE;
            }
            printf(" %7.1f", rate);
        }
        putchar('\n');
    }
    return EXIT_SUCCESS;
}
