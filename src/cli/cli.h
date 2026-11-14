/* SPDX-License-Identifier: MIT */
/**
 * @file cli.h
 * @brief Commands of the terminal front-end.
 */
#ifndef BATTLESHIP_CLI_H
#define BATTLESHIP_CLI_H

#include "battleship/ai.h"

#include <stdbool.h>
#include <stdint.h>

/** Upper bound for --games, to keep a typo from running for hours. */
#define CLI_MAX_GAMES 10000000

/** Options shared by the commands, as parsed from the command line. */
typedef struct cli_options {
    bs_difficulty level; /**< Computer level; in simulate, the only level run. */
    bool level_given;
    uint64_t seed;
    bool seed_given;
    int games; /**< simulate: games per level. */
    bool csv;  /**< simulate: machine-readable histogram instead of tables. */
    bool duel; /**< simulate: also run the head-to-head matches. */
} cli_options;

/** Interactive game on standard input/output. Returns a process exit code. */
int cli_play(const cli_options *options);

/** AI benchmark. Returns a process exit code. */
int cli_simulate(const cli_options *options);

#endif /* BATTLESHIP_CLI_H */
