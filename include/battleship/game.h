/* SPDX-License-Identifier: MIT */
/**
 * @file game.h
 * @brief A full match between a human and the computer.
 *
 * bs_game is the single entry point a front-end needs: it owns both boards,
 * the turn order and the score. It is a small state machine:
 *
 *                  miss                       miss
 *      +------------------------+  +------------------------+
 *      v                        |  |                        v
 *   HUMAN_TURN --hit--> HUMAN_TURN  COMPUTER_TURN <--hit-- COMPUTER_TURN
 *      |                                                    |
 *      +--------- last enemy ship sunk ----> OVER <---------+
 *
 * A hit grants another shot; a miss passes the turn. The human always opens.
 * The fleet layout may be redrawn until the first shot is fired.
 *
 * The game never blocks and never calls back: each function performs exactly
 * one step and reports what happened, which lets any front-end (GTK, terminal,
 * tests) drive it at its own pace.
 */
#ifndef BATTLESHIP_GAME_H
#define BATTLESHIP_GAME_H

#include "battleship/ai.h"
#include "battleship/board.h"
#include "battleship/rng.h"
#include "battleship/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Whose move it is. */
typedef enum bs_phase {
    BS_PHASE_HUMAN_TURN = 0,
    BS_PHASE_COMPUTER_TURN,
    BS_PHASE_OVER
} bs_phase;

/** Running score of one side. */
typedef struct bs_stats {
    unsigned shots;      /**< Shots fired. */
    unsigned hits;       /**< Shots that hit a deck (sinking shots included). */
    unsigned ships_sunk; /**< Enemy ships sunk. */
} bs_stats;

/** What a single shot changed. */
typedef struct bs_turn_report {
    bs_player shooter;
    bs_coord target;
    bs_shot_result result;
    bs_phase next_phase; /**< Phase after the shot. */
} bs_turn_report;

/** A match. Zero-allocation; initialise with bs_game_init(). */
typedef struct bs_game {
    bs_board boards[BS_PLAYER_COUNT]; /**< boards[p] is the fleet owned by p. */
    bs_stats stats[BS_PLAYER_COUNT];  /**< stats[p] counts the shots fired by p. */
    bs_difficulty difficulty;
    bs_phase phase;
    bs_player winner; /**< Meaningful only when phase is BS_PHASE_OVER. */
    bs_rng rng;
} bs_game;

/**
 * Starts a new match: both fleets are placed at random and the human is to
 * move.
 * @param seed any value; equal seeds and inputs replay the same match.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT or BS_ERR_PLACEMENT_FAILED.
 */
bs_status bs_game_init(bs_game *game, bs_difficulty difficulty, uint64_t seed);

/** Current phase. */
bs_phase bs_game_phase(const bs_game *game);

/** True once a fleet has been destroyed. */
bool bs_game_is_over(const bs_game *game);

/** Winner of a finished match; BS_PLAYER_HUMAN while it is still running. */
bs_player bs_game_winner(const bs_game *game);

/** Level of the computer opponent. */
bs_difficulty bs_game_difficulty(const bs_game *game);

/** Board holding the fleet of @p owner. NULL for invalid input. */
const bs_board *bs_game_board(const bs_game *game, bs_player owner);

/** Score of @p shooter. NULL for invalid input. */
const bs_stats *bs_game_stats(const bs_game *game, bs_player shooter);

/** True until the first shot of the match is fired. */
bool bs_game_can_reshuffle(const bs_game *game);

/**
 * Redraws the human fleet at random.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT, BS_ERR_WRONG_PHASE once a shot has
 *         been fired, or BS_ERR_PLACEMENT_FAILED.
 */
bs_status bs_game_reshuffle(bs_game *game);

/**
 * The human fires at the computer's board.
 * @param report optional; filled on success.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT, BS_ERR_WRONG_PHASE,
 *         BS_ERR_OUT_OF_BOUNDS or BS_ERR_ALREADY_SHOT. The game is unchanged
 *         on error.
 */
bs_status bs_game_human_fire(bs_game *game, bs_coord target, bs_turn_report *report);

/**
 * The computer fires one shot at the human's board, chosen by its AI.
 * @param report optional; filled on success.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT or BS_ERR_WRONG_PHASE.
 */
bs_status bs_game_computer_fire(bs_game *game, bs_turn_report *report);

/**
 * Suggests a shot for the human, as an AI of the given level would play it.
 * Used by the terminal hint command and by the automated UI smoke test.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT or BS_ERR_ALREADY_SHOT.
 */
bs_status bs_game_suggest(bs_game *game, bs_difficulty level, bs_coord *target);

#ifdef __cplusplus
}
#endif

#endif /* BATTLESHIP_GAME_H */
