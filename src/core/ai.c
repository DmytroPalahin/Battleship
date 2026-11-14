/* SPDX-License-Identifier: MIT */
#include "battleship/ai.h"

#include <stddef.h>
#include <string.h>

static const char *const DIFFICULTY_NAMES[BS_DIFFICULTY_COUNT] = {"easy", "medium", "hard"};

/* ------------------------------------------------------------------ helpers */

static bs_cell_state state_at(const bs_knowledge *knowledge, int row, int col)
{
    return (bs_cell_state)knowledge->state[row * BS_BOARD_SIZE + col];
}

static bool is_on_board(int row, int col)
{
    return row >= 0 && row < BS_BOARD_SIZE && col >= 0 && col < BS_BOARD_SIZE;
}

static bool is_state(const bs_knowledge *knowledge, int row, int col, bs_cell_state state)
{
    return is_on_board(row, col) && state_at(knowledge, row, col) == state;
}

/* Picks one of the listed cell indices uniformly at random. */
static bs_status pick(const int *cells, int count, bs_rng *rng, bs_coord *target)
{
    if (count <= 0) {
        return BS_ERR_ALREADY_SHOT;
    }
    *target = bs_coord_from_index(cells[bs_rng_below(rng, (uint32_t)count)]);
    return BS_OK;
}

/* Lists the cells in one of the given states; returns how many were found. */
static int collect(const bs_knowledge *knowledge, bool unknown, bool excluded,
                   int cells[BS_CELL_COUNT])
{
    int count = 0;
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        const uint8_t state = knowledge->state[index];
        if ((unknown && state == BS_CELL_UNKNOWN) || (excluded && state == BS_CELL_EXCLUDED)) {
            cells[count++] = index;
        }
    }
    return count;
}

/* --------------------------------------------------------------------- easy */

/* Fires anywhere it has not fired yet, ignoring everything it could deduce. */
static bs_status choose_easy(const bs_knowledge *knowledge, bs_rng *rng, bs_coord *target)
{
    int cells[BS_CELL_COUNT];
    return pick(cells, collect(knowledge, true, true, cells), rng, target);
}

/* ------------------------------------------------------------------- medium */

/*
 * Fires at random, but never at a cell the rules prove to be water: the
 * diagonals of a hit and the surroundings of a sunk ship. On this rule set
 * that deduction alone saves about thirty shots per game.
 */
static bs_status choose_medium(const bs_knowledge *knowledge, bs_rng *rng, bs_coord *target)
{
    int cells[BS_CELL_COUNT];
    const int count = collect(knowledge, true, false, cells);
    if (count > 0) {
        return pick(cells, count, rng, target);
    }
    return choose_easy(knowledge, rng, target);
}

/* --------------------------------------------------------------------- hard */

/*
 * Tests one candidate position of a ship against the known facts.
 *
 * A position is consistent when:
 *  - each of its cells is either unexplored or a hit on a ship still afloat;
 *  - it is not made of hits only (that ship would already be sunk);
 *  - no hit lies next to it without being part of it (ships never touch).
 *
 * @return the number of hits the position explains, or -1 if inconsistent.
 */
static int placement_hits(const bs_knowledge *knowledge, int row, int col, int length, int d_row,
                          int d_col)
{
    const int last_row = row + d_row * (length - 1);
    const int last_col = col + d_col * (length - 1);
    int hits = 0;

    for (int i = 0; i < length; i++) {
        const bs_cell_state state = state_at(knowledge, row + d_row * i, col + d_col * i);
        if (state == BS_CELL_HIT) {
            hits++;
        } else if (state != BS_CELL_UNKNOWN) {
            return -1;
        }
    }
    if (hits == length) {
        return -1;
    }

    for (int r = row - 1; r <= last_row + 1; r++) {
        for (int c = col - 1; c <= last_col + 1; c++) {
            const bool inside = r >= row && r <= last_row && c >= col && c <= last_col;
            if (!inside && is_state(knowledge, r, c, BS_CELL_HIT)) {
                return -1;
            }
        }
    }
    return hits;
}

uint32_t bs_ai_density(const bs_knowledge *knowledge, uint32_t scores[BS_CELL_COUNT])
{
    if (knowledge == NULL || scores == NULL) {
        return 0;
    }
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        scores[index] = 0;
    }

    /* While a ship is wounded, only positions that explain a hit matter. */
    bool targeting = false;
    for (int index = 0; index < BS_CELL_COUNT && !targeting; index++) {
        targeting = knowledge->state[index] == BS_CELL_HIT;
    }

    for (int length = 1; length <= BS_MAX_SHIP_LENGTH; length++) {
        const uint32_t afloat = knowledge->afloat_by_length[length];
        if (afloat == 0) {
            continue;
        }
        const int orientations = length == 1 ? 1 : 2;

        for (int o = 0; o < orientations; o++) {
            const int d_row = o == 1 ? 1 : 0;
            const int d_col = o == 0 ? 1 : 0;
            const int rows = BS_BOARD_SIZE - d_row * (length - 1);
            const int cols = BS_BOARD_SIZE - d_col * (length - 1);

            for (int row = 0; row < rows; row++) {
                for (int col = 0; col < cols; col++) {
                    const int hits = placement_hits(knowledge, row, col, length, d_row, d_col);
                    if (hits < 0 || (targeting && hits == 0)) {
                        continue;
                    }
                    const uint32_t weight = targeting ? afloat * (uint32_t)hits : afloat;
                    for (int i = 0; i < length; i++) {
                        const int r = row + d_row * i;
                        const int c = col + d_col * i;
                        if (state_at(knowledge, r, c) == BS_CELL_UNKNOWN) {
                            scores[r * BS_BOARD_SIZE + c] += weight;
                        }
                    }
                }
            }
        }
    }

    uint32_t best = 0;
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        if (scores[index] > best) {
            best = scores[index];
        }
    }
    return best;
}

static bs_status choose_hard(const bs_knowledge *knowledge, bs_rng *rng, bs_coord *target)
{
    uint32_t scores[BS_CELL_COUNT];
    const uint32_t best = bs_ai_density(knowledge, scores);
    if (best == 0) {
        return choose_medium(knowledge, rng, target);
    }

    int cells[BS_CELL_COUNT];
    int count = 0;
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        if (scores[index] == best) {
            cells[count++] = index;
        }
    }
    return pick(cells, count, rng, target);
}

/* ------------------------------------------------------------------- public */

const char *bs_difficulty_name(bs_difficulty difficulty)
{
    if ((int)difficulty < 0 || (int)difficulty >= BS_DIFFICULTY_COUNT) {
        return "unknown";
    }
    return DIFFICULTY_NAMES[difficulty];
}

bool bs_difficulty_parse(const char *name, bs_difficulty *out)
{
    if (name == NULL || out == NULL) {
        return false;
    }
    for (int i = 0; i < BS_DIFFICULTY_COUNT; i++) {
        if (strcmp(name, DIFFICULTY_NAMES[i]) == 0) {
            *out = (bs_difficulty)i;
            return true;
        }
    }
    return false;
}

bs_status bs_ai_choose_target(bs_difficulty difficulty, const bs_knowledge *knowledge, bs_rng *rng,
                              bs_coord *target)
{
    if (knowledge == NULL || rng == NULL || target == NULL) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    switch (difficulty) {
    case BS_DIFFICULTY_EASY:
        return choose_easy(knowledge, rng, target);
    case BS_DIFFICULTY_MEDIUM:
        return choose_medium(knowledge, rng, target);
    case BS_DIFFICULTY_HARD:
        return choose_hard(knowledge, rng, target);
    }
    return BS_ERR_INVALID_ARGUMENT;
}
