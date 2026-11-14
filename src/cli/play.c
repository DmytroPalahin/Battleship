/* SPDX-License-Identifier: MIT */
/**
 * @file play.c
 * @brief `battleship-cli play`: the game in a terminal.
 *
 * A second view and controller for the same model as the GTK application:
 * it renders with printf and reads its events from standard input.
 */
#include "cli.h"

#include "battleship/battleship.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 64

/* --------------------------------------------------------------------- view */

/* Character for a cell. Ships are only drawn on the player's own board. */
static char cell_symbol(const bs_board *board, bs_coord cell, bool reveal_ships)
{
    switch (bs_board_cell(board, cell)) {
    case BS_CELL_MISS:
        return 'o';
    case BS_CELL_HIT:
        return 'X';
    case BS_CELL_SUNK:
        return '#';
    case BS_CELL_EXCLUDED:
        return '-';
    case BS_CELL_UNKNOWN:
        break;
    }
    return reveal_ships && bs_board_has_ship(board, cell) ? 'S' : '.';
}

static void print_header(void)
{
    printf("   ");
    for (int col = 0; col < BS_BOARD_SIZE; col++) {
        printf(" %c", 'A' + col);
    }
}

static void print_boards(const bs_game *game)
{
    const bs_board *own = bs_game_board(game, BS_PLAYER_HUMAN);
    const bs_board *enemy = bs_game_board(game, BS_PLAYER_COMPUTER);

    printf("\n%-27s%s\n", "   YOUR FLEET", "   ENEMY WATERS");
    print_header();
    printf("    ");
    print_header();
    putchar('\n');

    for (int row = 0; row < BS_BOARD_SIZE; row++) {
        printf("%3d", row + 1);
        for (int col = 0; col < BS_BOARD_SIZE; col++) {
            printf(" %c", cell_symbol(own, bs_coord_make(row, col), true));
        }
        printf("    %3d", row + 1);
        for (int col = 0; col < BS_BOARD_SIZE; col++) {
            printf(" %c", cell_symbol(enemy, bs_coord_make(row, col), false));
        }
        putchar('\n');
    }
    printf("\n   S ship   X hit   # sunk   o miss   - cannot hold a ship\n");
    printf("   ships afloat: you %d, enemy %d\n\n", own->ships_afloat, enemy->ships_afloat);
}

static const char *result_text(bs_shot_result result)
{
    switch (result) {
    case BS_SHOT_MISS:
        return "miss";
    case BS_SHOT_HIT:
        return "hit!";
    case BS_SHOT_SUNK:
        return "hit and sunk!";
    }
    return "?";
}

static void print_report(const bs_turn_report *report)
{
    printf("%s %c%d: %s\n", report->shooter == BS_PLAYER_HUMAN ? "You fire at" : "Enemy fires at",
           'A' + report->target.col, report->target.row + 1, result_text(report->result));
}

static void print_help(void)
{
    puts("  B7       fire at column B, row 7\n"
         "  hint     let the computer suggest a shot\n"
         "  shuffle  redraw your fleet (only before the first shot)\n"
         "  help     show this list\n"
         "  quit     leave the game");
}

/* --------------------------------------------------------------- controller */

/* Removes surrounding blanks and lower-cases the line in place. */
static char *normalise(char *line)
{
    while (isspace((unsigned char)*line)) {
        line++;
    }
    size_t length = strlen(line);
    while (length > 0 && isspace((unsigned char)line[length - 1])) {
        line[--length] = '\0';
    }
    for (char *c = line; *c != '\0'; c++) {
        *c = (char)tolower((unsigned char)*c);
    }
    return line;
}

/* Parses "b7" / "j10" into a coordinate. */
static bool parse_cell(const char *text, bs_coord *out)
{
    if (text[0] < 'a' || text[0] >= 'a' + BS_BOARD_SIZE || !isdigit((unsigned char)text[1])) {
        return false;
    }
    char *end = NULL;
    const long row = strtol(text + 1, &end, 10);
    if (*end != '\0' || row < 1 || row > BS_BOARD_SIZE) {
        return false;
    }
    *out = bs_coord_make((int)row - 1, text[0] - 'a');
    return true;
}

static void run_computer_turn(bs_game *game)
{
    while (bs_game_phase(game) == BS_PHASE_COMPUTER_TURN) {
        bs_turn_report report;
        if (bs_game_computer_fire(game, &report) != BS_OK) {
            return;
        }
        print_report(&report);
    }
}

static void fire_and_report(bs_game *game, bs_coord target)
{
    bs_turn_report report;
    const bs_status status = bs_game_human_fire(game, target, &report);
    if (status != BS_OK) {
        printf("Cannot fire there: %s.\n", bs_status_str(status));
        return;
    }
    print_report(&report);
    run_computer_turn(game);
    print_boards(game);
}

int cli_play(const cli_options *options)
{
    bs_game game;
    const bs_status status = bs_game_init(&game, options->level, options->seed);
    if (status != BS_OK) {
        fprintf(stderr, "battleship-cli: %s\n", bs_status_str(status));
        return EXIT_FAILURE;
    }

    printf("Battleship %s - level %s, seed %llu\n", bs_version(),
           bs_difficulty_name(options->level), (unsigned long long)options->seed);
    puts("Sink the ten enemy ships before yours are sunk. Type 'help' for the commands.");
    print_boards(&game);

    char buffer[LINE_SIZE];
    while (!bs_game_is_over(&game)) {
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, sizeof buffer, stdin) == NULL) {
            putchar('\n');
            return EXIT_SUCCESS; /* end of input: leave quietly */
        }

        const char *line = normalise(buffer);
        bs_coord target;
        if (*line == '\0') {
            continue;
        }
        if (strcmp(line, "quit") == 0 || strcmp(line, "q") == 0) {
            return EXIT_SUCCESS;
        }
        if (strcmp(line, "help") == 0) {
            print_help();
        } else if (strcmp(line, "shuffle") == 0) {
            const bs_status shuffled = bs_game_reshuffle(&game);
            if (shuffled == BS_OK) {
                print_boards(&game);
            } else {
                printf("Cannot shuffle: %s.\n", bs_status_str(shuffled));
            }
        } else if (strcmp(line, "hint") == 0) {
            if (bs_game_suggest(&game, BS_DIFFICULTY_HARD, &target) == BS_OK) {
                printf("Try %c%d.\n", 'A' + target.col, target.row + 1);
            }
        } else if (parse_cell(line, &target)) {
            fire_and_report(&game, target);
        } else {
            puts("Not understood. Type a cell such as B7, or 'help'.");
        }
    }

    const bs_stats *stats = bs_game_stats(&game, BS_PLAYER_HUMAN);
    if (bs_game_winner(&game) == BS_PLAYER_HUMAN) {
        printf("Victory! Enemy fleet sunk in %u shots (%u hits).\n", stats->shots, stats->hits);
    } else {
        printf("Defeat. Your fleet is gone after %u of your shots (%u hits).\n", stats->shots,
               stats->hits);
    }
    return EXIT_SUCCESS;
}
