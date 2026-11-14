/* SPDX-License-Identifier: MIT */
#include "battleship/ai.h"
#include "bs_test.h"

#include <stdlib.h>

static bs_coord at(int row, int col)
{
    return bs_coord_make(row, col);
}

/* Knowledge of an untouched board holding the classic fleet. */
static bs_knowledge fresh_knowledge(void)
{
    bs_knowledge knowledge;
    memset(&knowledge, 0, sizeof knowledge);
    knowledge.afloat_by_length[4] = 1;
    knowledge.afloat_by_length[3] = 2;
    knowledge.afloat_by_length[2] = 3;
    knowledge.afloat_by_length[1] = 4;
    return knowledge;
}

static void set(bs_knowledge *knowledge, int row, int col, bs_cell_state state)
{
    knowledge->state[bs_coord_index(at(row, col))] = (uint8_t)state;
}

/* Marks every cell the given state, to carve out small scenarios. */
static void fill(bs_knowledge *knowledge, bs_cell_state state)
{
    memset(knowledge->state, (int)state, sizeof knowledge->state);
}

/*
 * Lets one AI clear a random board alone and returns the number of shots it
 * needed, or -1 if it broke a rule (repeated a cell, ran out of moves...).
 */
static int shots_to_clear(bs_difficulty level, uint64_t seed, bool forbid_excluded)
{
    bs_board board;
    bs_rng rng;
    bs_rng_seed(&rng, seed);
    if (bs_board_place_fleet_random(&board, &rng) != BS_OK) {
        return -1;
    }

    for (int shots = 1; shots <= BS_CELL_COUNT; shots++) {
        bs_knowledge knowledge;
        bs_coord target;
        bs_shot_report report;

        bs_board_get_knowledge(&board, &knowledge);
        if (bs_ai_choose_target(level, &knowledge, &rng, &target) != BS_OK) {
            return -1;
        }
        if (forbid_excluded && bs_board_cell(&board, target) != BS_CELL_UNKNOWN) {
            return -1;
        }
        if (bs_board_fire(&board, target, &report) != BS_OK) {
            return -1;
        }
        if (report.fleet_destroyed) {
            return shots;
        }
    }
    return -1;
}

static double mean_shots(bs_difficulty level, int games)
{
    long total = 0;
    for (int game = 0; game < games; game++) {
        const int shots = shots_to_clear(level, (uint64_t)game, false);
        if (shots < 0) {
            return -1.0;
        }
        total += shots;
    }
    return (double)total / games;
}

/* -------------------------------------------------------------------- names */

TEST(difficulty_names_round_trip)
{
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        bs_difficulty parsed = BS_DIFFICULTY_EASY;
        const char *name = bs_difficulty_name((bs_difficulty)level);
        CHECK(bs_difficulty_parse(name, &parsed));
        CHECK_EQ(level, parsed);
    }
    bs_difficulty untouched = BS_DIFFICULTY_MEDIUM;
    CHECK(!bs_difficulty_parse("impossible", &untouched));
    CHECK(!bs_difficulty_parse(NULL, &untouched));
    CHECK(!bs_difficulty_parse("easy", NULL));
    CHECK_EQ(BS_DIFFICULTY_MEDIUM, untouched);
    CHECK(strcmp(bs_difficulty_name((bs_difficulty)42), "unknown") == 0);
}

/* -------------------------------------------------------------- rule safety */

TEST(rejects_invalid_arguments)
{
    bs_knowledge knowledge = fresh_knowledge();
    bs_coord target;
    bs_rng rng;
    bs_rng_seed(&rng, 1);

    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_ai_choose_target(BS_DIFFICULTY_HARD, NULL, &rng, &target));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT,
             bs_ai_choose_target(BS_DIFFICULTY_HARD, &knowledge, NULL, &target));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT,
             bs_ai_choose_target(BS_DIFFICULTY_HARD, &knowledge, &rng, NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT,
             bs_ai_choose_target((bs_difficulty)9, &knowledge, &rng, &target));
    CHECK_EQ(0, bs_ai_density(NULL, NULL));
}

TEST(reports_when_no_cell_is_left)
{
    bs_knowledge knowledge = fresh_knowledge();
    bs_coord target;
    bs_rng rng;
    bs_rng_seed(&rng, 1);
    fill(&knowledge, BS_CELL_MISS);

    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        CHECK_EQ(BS_ERR_ALREADY_SHOT,
                 bs_ai_choose_target((bs_difficulty)level, &knowledge, &rng, &target));
    }
}

TEST(every_level_clears_any_board_without_breaking_rules)
{
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        for (uint64_t seed = 0; seed < 300; seed++) {
            const int shots = shots_to_clear((bs_difficulty)level, seed, false);
            REQUIRE(shots >= BS_FLEET_DECKS);
            REQUIRE(shots <= BS_CELL_COUNT);
        }
    }
}

/* Medium and hard reason about the board: they never waste a shot on a cell
 * the rules prove to be water. Easy is allowed to. */
TEST(medium_and_hard_never_fire_at_excluded_cells)
{
    for (uint64_t seed = 0; seed < 300; seed++) {
        REQUIRE(shots_to_clear(BS_DIFFICULTY_MEDIUM, seed, true) > 0);
        REQUIRE(shots_to_clear(BS_DIFFICULTY_HARD, seed, true) > 0);
    }
}

TEST(falls_back_to_excluded_cells_when_nothing_else_is_left)
{
    bs_knowledge knowledge = fresh_knowledge();
    bs_coord target;
    bs_rng rng;
    bs_rng_seed(&rng, 3);
    fill(&knowledge, BS_CELL_MISS);
    set(&knowledge, 6, 2, BS_CELL_EXCLUDED);

    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        REQUIRE(bs_ai_choose_target((bs_difficulty)level, &knowledge, &rng, &target) == BS_OK);
        CHECK_EQ(6, target.row);
        CHECK_EQ(2, target.col);
    }
}

/* ---------------------------------------------------------------- targeting */

static bool is_orthogonal_neighbour(bs_coord a, bs_coord b)
{
    return abs(a.row - b.row) + abs(a.col - b.col) == 1;
}

/* Only the hard level goes after a ship it has wounded. */
TEST(after_a_hit_the_next_shot_is_adjacent)
{
    static const bs_difficulty levels[] = {BS_DIFFICULTY_HARD};
    bs_rng rng;
    bs_rng_seed(&rng, 11);

    for (size_t l = 0; l < sizeof levels / sizeof levels[0]; l++) {
        for (int trial = 0; trial < 200; trial++) {
            bs_knowledge knowledge = fresh_knowledge();
            bs_coord target;
            set(&knowledge, 4, 4, BS_CELL_HIT);
            set(&knowledge, 3, 3, BS_CELL_EXCLUDED);
            set(&knowledge, 3, 5, BS_CELL_EXCLUDED);
            set(&knowledge, 5, 3, BS_CELL_EXCLUDED);
            set(&knowledge, 5, 5, BS_CELL_EXCLUDED);

            REQUIRE(bs_ai_choose_target(levels[l], &knowledge, &rng, &target) == BS_OK);
            CHECK(is_orthogonal_neighbour(target, at(4, 4)));
        }
    }
}

TEST(two_aligned_hits_are_extended_along_the_line)
{
    static const bs_difficulty levels[] = {BS_DIFFICULTY_HARD};
    bs_rng rng;
    bs_rng_seed(&rng, 12);

    for (size_t l = 0; l < sizeof levels / sizeof levels[0]; l++) {
        bool seen_left = false;
        bool seen_right = false;
        for (int trial = 0; trial < 200; trial++) {
            bs_knowledge knowledge = fresh_knowledge();
            bs_coord target;
            set(&knowledge, 4, 4, BS_CELL_HIT);
            set(&knowledge, 4, 5, BS_CELL_HIT);
            for (int col = 3; col <= 6; col++) {
                set(&knowledge, 3, col, BS_CELL_EXCLUDED);
                set(&knowledge, 5, col, BS_CELL_EXCLUDED);
            }

            REQUIRE(bs_ai_choose_target(levels[l], &knowledge, &rng, &target) == BS_OK);
            CHECK_EQ(4, target.row);
            CHECK(target.col == 3 || target.col == 6);
            seen_left = seen_left || target.col == 3;
            seen_right = seen_right || target.col == 6;
        }
        CHECK(seen_left && seen_right); /* both ends are tried, not always the same */
    }
}

TEST(a_blocked_end_leaves_only_the_other_one)
{
    static const bs_difficulty levels[] = {BS_DIFFICULTY_HARD};
    bs_rng rng;
    bs_rng_seed(&rng, 13);

    for (size_t l = 0; l < sizeof levels / sizeof levels[0]; l++) {
        for (int trial = 0; trial < 50; trial++) {
            bs_knowledge knowledge = fresh_knowledge();
            bs_coord target;
            set(&knowledge, 0, 0, BS_CELL_HIT); /* vertical ship along the left edge */
            set(&knowledge, 1, 0, BS_CELL_HIT);
            set(&knowledge, 0, 1, BS_CELL_EXCLUDED);
            set(&knowledge, 1, 1, BS_CELL_EXCLUDED);
            set(&knowledge, 2, 1, BS_CELL_EXCLUDED);

            REQUIRE(bs_ai_choose_target(levels[l], &knowledge, &rng, &target) == BS_OK);
            CHECK_EQ(2, target.row);
            CHECK_EQ(0, target.col);
        }
    }
}

/* ------------------------------------------------------------------ density */

TEST(density_prefers_the_centre_of_an_empty_board)
{
    bs_knowledge knowledge = fresh_knowledge();
    uint32_t scores[BS_CELL_COUNT];
    const uint32_t best = bs_ai_density(&knowledge, scores);

    CHECK(best > 0);
    CHECK(scores[bs_coord_index(at(4, 4))] > scores[bs_coord_index(at(0, 0))]);
    CHECK(scores[bs_coord_index(at(4, 4))] > scores[bs_coord_index(at(0, 4))]);
    /* The board and the fleet are symmetric, so the heat map must be too. */
    for (int row = 0; row < BS_BOARD_SIZE; row++) {
        for (int col = 0; col < BS_BOARD_SIZE; col++) {
            const uint32_t here = scores[bs_coord_index(at(row, col))];
            CHECK(here == scores[bs_coord_index(at(col, row))]);
            CHECK(here == scores[bs_coord_index(at(BS_BOARD_SIZE - 1 - row, col))]);
        }
    }
}

TEST(density_is_zero_where_no_ship_can_be)
{
    bs_knowledge knowledge = fresh_knowledge();
    uint32_t scores[BS_CELL_COUNT];
    set(&knowledge, 2, 2, BS_CELL_MISS);
    set(&knowledge, 7, 7, BS_CELL_EXCLUDED);
    set(&knowledge, 5, 5, BS_CELL_SUNK);

    bs_ai_density(&knowledge, scores);
    CHECK_EQ(0, scores[bs_coord_index(at(2, 2))]);
    CHECK_EQ(0, scores[bs_coord_index(at(7, 7))]);
    CHECK_EQ(0, scores[bs_coord_index(at(5, 5))]);
    CHECK(scores[bs_coord_index(at(0, 0))] > 0);
}

/* With only the four-decker left, a gap of three cells cannot hide it. */
TEST(density_accounts_for_the_ships_still_afloat)
{
    bs_knowledge knowledge;
    uint32_t scores[BS_CELL_COUNT];
    memset(&knowledge, 0, sizeof knowledge);
    knowledge.afloat_by_length[4] = 1;
    fill(&knowledge, BS_CELL_MISS);
    for (int col = 0; col < 3; col++) {
        set(&knowledge, 0, col, BS_CELL_UNKNOWN); /* too short */
    }
    for (int col = 0; col < 5; col++) {
        set(&knowledge, 9, col, BS_CELL_UNKNOWN); /* two possible positions */
    }

    const uint32_t best = bs_ai_density(&knowledge, scores);
    CHECK_EQ(2, best);
    for (int col = 0; col < 3; col++) {
        CHECK_EQ(0, scores[bs_coord_index(at(0, col))]);
    }
    CHECK_EQ(1, scores[bs_coord_index(at(9, 0))]);
    CHECK_EQ(2, scores[bs_coord_index(at(9, 1))]);
    CHECK_EQ(2, scores[bs_coord_index(at(9, 3))]);
    CHECK_EQ(1, scores[bs_coord_index(at(9, 4))]);
}

TEST(hard_fires_at_the_most_likely_cell)
{
    bs_knowledge knowledge;
    bs_rng rng;
    bs_rng_seed(&rng, 21);
    memset(&knowledge, 0, sizeof knowledge);
    knowledge.afloat_by_length[4] = 1;
    fill(&knowledge, BS_CELL_MISS);
    for (int col = 0; col < 5; col++) {
        set(&knowledge, 9, col, BS_CELL_UNKNOWN);
    }

    for (int trial = 0; trial < 100; trial++) {
        bs_coord target;
        REQUIRE(bs_ai_choose_target(BS_DIFFICULTY_HARD, &knowledge, &rng, &target) == BS_OK);
        CHECK_EQ(9, target.row);
        CHECK(target.col >= 1 && target.col <= 3);
    }
}

/* ----------------------------------------------------------------- strength */

/*
 * The levels must be ordered by strength, with a clear margin. The games are
 * seeded, so this is a regression test of the strategies and cannot flake.
 */
TEST(levels_are_ordered_by_strength)
{
    enum {
        GAMES = 400
    };
    const double easy = mean_shots(BS_DIFFICULTY_EASY, GAMES);
    const double medium = mean_shots(BS_DIFFICULTY_MEDIUM, GAMES);
    const double hard = mean_shots(BS_DIFFICULTY_HARD, GAMES);

    printf("# mean shots to sink a fleet over %d games: easy %.1f, medium %.1f, hard %.1f\n", GAMES,
           easy, medium, hard);
    REQUIRE(easy > 0.0 && medium > 0.0 && hard > 0.0);
    CHECK(medium < easy - 20.0);
    CHECK(hard < medium - 5.0);
}

TEST(choices_are_reproducible_for_a_seed)
{
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        for (uint64_t seed = 0; seed < 20; seed++) {
            CHECK_EQ(shots_to_clear((bs_difficulty)level, seed, false),
                     shots_to_clear((bs_difficulty)level, seed, false));
        }
    }
}

static const bs_test_case TESTS[] = {
    BS_CASE(difficulty_names_round_trip),
    BS_CASE(rejects_invalid_arguments),
    BS_CASE(reports_when_no_cell_is_left),
    BS_CASE(every_level_clears_any_board_without_breaking_rules),
    BS_CASE(medium_and_hard_never_fire_at_excluded_cells),
    BS_CASE(falls_back_to_excluded_cells_when_nothing_else_is_left),
    BS_CASE(after_a_hit_the_next_shot_is_adjacent),
    BS_CASE(two_aligned_hits_are_extended_along_the_line),
    BS_CASE(a_blocked_end_leaves_only_the_other_one),
    BS_CASE(density_prefers_the_centre_of_an_empty_board),
    BS_CASE(density_is_zero_where_no_ship_can_be),
    BS_CASE(density_accounts_for_the_ships_still_afloat),
    BS_CASE(hard_fires_at_the_most_likely_cell),
    BS_CASE(levels_are_ordered_by_strength),
    BS_CASE(choices_are_reproducible_for_a_seed),
};

BS_TEST_MAIN(TESTS)
