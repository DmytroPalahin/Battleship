/* SPDX-License-Identifier: MIT */
/**
 * @file ai.h
 * @brief Computer opponent: three strategies behind one function.
 *
 * The AI is a pure function of the public knowledge about the enemy board and
 * of a random generator. It keeps no state between calls, so it can be
 * swapped, replayed or run against itself at will, and it cannot peek at the
 * ship positions because it never receives them.
 *
 * Each level adds one skill to the previous one:
 *
 * | Level  | Strategy                                                        |
 * |--------|-----------------------------------------------------------------|
 * | Easy   | Fires at random; only remembers where it has already fired.     |
 * | Medium | Also skips the cells the rules prove to be water (diagonals of  |
 * |        | a hit, surroundings of a sunk ship).                            |
 * | Hard   | Also finishes off wounded ships and, when hunting, fires at the |
 * |        | most likely cell: it counts every position of every ship still  |
 * |        | afloat that is consistent with what is known.                   |
 */
#ifndef BATTLESHIP_AI_H
#define BATTLESHIP_AI_H

#include "battleship/board.h"
#include "battleship/rng.h"
#include "battleship/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Strength of the computer opponent. */
typedef enum bs_difficulty {
    BS_DIFFICULTY_EASY = 0,
    BS_DIFFICULTY_MEDIUM,
    BS_DIFFICULTY_HARD
} bs_difficulty;

/** Number of difficulty levels; handy for array sizes and loops. */
#define BS_DIFFICULTY_COUNT 3

/** Lower-case English name of a level ("easy", "medium", "hard"). */
const char *bs_difficulty_name(bs_difficulty difficulty);

/**
 * Parses a level name as produced by bs_difficulty_name().
 * @return true and sets @p out on success, false otherwise.
 */
bool bs_difficulty_parse(const char *name, bs_difficulty *out);

/**
 * Picks the next cell to fire at.
 *
 * @param difficulty strategy to apply.
 * @param knowledge  public view of the enemy board.
 * @param rng        source of randomness (tie-breaking and random levels).
 * @param target     receives a cell that has not been fired at yet.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT, or BS_ERR_ALREADY_SHOT when every
 *         cell of the board has already been fired at.
 */
bs_status bs_ai_choose_target(bs_difficulty difficulty, const bs_knowledge *knowledge, bs_rng *rng,
                              bs_coord *target);

/**
 * Computes the heat map used by the hard level: for each cell, the number of
 * ship placements consistent with @p knowledge that cover it. Cells that
 * cannot be fired at score zero. Exposed for tests, tooling and visualisation.
 *
 * @param knowledge public view of the enemy board.
 * @param scores    receives BS_CELL_COUNT values in row-major order.
 * @return the highest score written.
 */
uint32_t bs_ai_density(const bs_knowledge *knowledge, uint32_t scores[BS_CELL_COUNT]);

#ifdef __cplusplus
}
#endif

#endif /* BATTLESHIP_AI_H */
