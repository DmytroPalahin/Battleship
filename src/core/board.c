/* SPDX-License-Identifier: MIT */
#include "battleship/board.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/*
 * Upper bound on full-fleet placement attempts. A sequential random placement
 * of the classic fleet dead-ends in well under 1 % of the tries, so this is
 * never reached; it only turns "cannot happen" into a reported error instead
 * of an endless loop.
 */
#define PLACEMENT_MAX_ATTEMPTS 1000

/* Every (origin, orientation) pair for one ship: 2 orientations per cell. */
#define PLACEMENT_MAX_CANDIDATES (2 * BS_CELL_COUNT)

const uint8_t BS_FLEET_LENGTHS[BS_FLEET_SIZE] = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};

typedef struct placement {
    bs_coord origin;
    bs_orientation orientation;
} placement;

/* ------------------------------------------------------------------ helpers */

static bool orientation_is_valid(bs_orientation orientation)
{
    return orientation == BS_HORIZONTAL || orientation == BS_VERTICAL;
}

/* Coordinate of the last deck of a ship described by its three parameters. */
static bs_coord last_deck(bs_coord origin, int length, bs_orientation orientation)
{
    return orientation == BS_HORIZONTAL ? bs_coord_make(origin.row, origin.col + length - 1)
                                        : bs_coord_make(origin.row + length - 1, origin.col);
}

/* Marks a cell as provably empty, unless something is already known about it. */
static void exclude(bs_board *board, int row, int col)
{
    const bs_coord cell = bs_coord_make(row, col);
    if (!bs_coord_is_valid(cell)) {
        return;
    }
    uint8_t *state = &board->state[bs_coord_index(cell)];
    if (*state == BS_CELL_UNKNOWN) {
        *state = BS_CELL_EXCLUDED;
    }
}

/* Ships never touch, so the four diagonal neighbours of a deck are water. */
static void exclude_diagonals(bs_board *board, bs_coord hit)
{
    exclude(board, hit.row - 1, hit.col - 1);
    exclude(board, hit.row - 1, hit.col + 1);
    exclude(board, hit.row + 1, hit.col - 1);
    exclude(board, hit.row + 1, hit.col + 1);
}

/* Flags the decks of a sunk ship and rules out its whole surroundings. */
static void mark_sunk(bs_board *board, const bs_ship *ship)
{
    const bs_coord last = last_deck(ship->origin, ship->length, ship->orientation);

    for (int i = 0; i < ship->length; i++) {
        board->state[bs_coord_index(bs_ship_deck(ship, i))] = BS_CELL_SUNK;
    }
    for (int row = ship->origin.row - 1; row <= last.row + 1; row++) {
        for (int col = ship->origin.col - 1; col <= last.col + 1; col++) {
            exclude(board, row, col);
        }
    }
}

/* Lists every legal position for a ship of the given length. */
static int collect_placements(const bs_board *board, int length,
                              placement out[PLACEMENT_MAX_CANDIDATES])
{
    /* A single-deck ship looks the same in both orientations: count it once. */
    const int orientations = length == 1 ? 1 : 2;
    int count = 0;

    for (int o = 0; o < orientations; o++) {
        const bs_orientation orientation = o == 0 ? BS_HORIZONTAL : BS_VERTICAL;
        for (int index = 0; index < BS_CELL_COUNT; index++) {
            const bs_coord origin = bs_coord_from_index(index);
            if (bs_board_can_place(board, origin, length, orientation) == BS_OK) {
                out[count].origin = origin;
                out[count].orientation = orientation;
                count++;
            }
        }
    }
    return count;
}

/* One attempt at placing the whole fleet; fails if a ship has no room left. */
static bool try_place_fleet(bs_board *board, bs_rng *rng)
{
    placement candidates[PLACEMENT_MAX_CANDIDATES];

    for (int i = 0; i < BS_FLEET_SIZE; i++) {
        const int length = BS_FLEET_LENGTHS[i];
        const int count = collect_placements(board, length, candidates);
        if (count == 0) {
            return false;
        }
        const placement *chosen = &candidates[bs_rng_below(rng, (uint32_t)count)];
        const bs_status status =
            bs_board_place_ship(board, chosen->origin, length, chosen->orientation);
        assert(status == BS_OK);
        (void)status;
    }
    return true;
}

/* ------------------------------------------------------------------- public */

void bs_board_init(bs_board *board)
{
    if (board == NULL) {
        return;
    }
    memset(board, 0, sizeof *board);
    for (int i = 0; i < BS_CELL_COUNT; i++) {
        board->ship_at[i] = BS_NO_SHIP;
    }
}

bool bs_board_has_ship(const bs_board *board, bs_coord cell)
{
    return bs_board_ship_at(board, cell) != NULL;
}

const bs_ship *bs_board_ship_at(const bs_board *board, bs_coord cell)
{
    if (board == NULL || !bs_coord_is_valid(cell)) {
        return NULL;
    }
    const int ship_index = board->ship_at[bs_coord_index(cell)];
    return ship_index == BS_NO_SHIP ? NULL : &board->ships[ship_index];
}

bs_cell_state bs_board_cell(const bs_board *board, bs_coord cell)
{
    if (board == NULL || !bs_coord_is_valid(cell)) {
        return BS_CELL_UNKNOWN;
    }
    return (bs_cell_state)board->state[bs_coord_index(cell)];
}

bs_coord bs_ship_deck(const bs_ship *ship, int i)
{
    return ship->orientation == BS_HORIZONTAL
               ? bs_coord_make(ship->origin.row, ship->origin.col + i)
               : bs_coord_make(ship->origin.row + i, ship->origin.col);
}

bool bs_ship_is_sunk(const bs_ship *ship)
{
    return ship != NULL && ship->hits >= ship->length;
}

bs_status bs_board_can_place(const bs_board *board, bs_coord origin, int length,
                             bs_orientation orientation)
{
    if (board == NULL || length < 1 || length > BS_MAX_SHIP_LENGTH ||
        !orientation_is_valid(orientation)) {
        return BS_ERR_INVALID_ARGUMENT;
    }

    const bs_coord last = last_deck(origin, length, orientation);
    if (!bs_coord_is_valid(origin) || !bs_coord_is_valid(last)) {
        return BS_ERR_OUT_OF_BOUNDS;
    }

    /* Any ship inside the footprint grown by one cell overlaps or touches. */
    for (int row = origin.row - 1; row <= last.row + 1; row++) {
        for (int col = origin.col - 1; col <= last.col + 1; col++) {
            const bs_coord cell = bs_coord_make(row, col);
            if (bs_coord_is_valid(cell) && board->ship_at[bs_coord_index(cell)] != BS_NO_SHIP) {
                return BS_ERR_COLLISION;
            }
        }
    }
    return BS_OK;
}

bs_status bs_board_place_ship(bs_board *board, bs_coord origin, int length,
                              bs_orientation orientation)
{
    const bs_status status = bs_board_can_place(board, origin, length, orientation);
    if (status != BS_OK) {
        return status;
    }
    if (board->ship_count >= BS_FLEET_SIZE) {
        return BS_ERR_FLEET_FULL;
    }

    const uint8_t ship_index = board->ship_count;
    bs_ship *ship = &board->ships[ship_index];
    ship->origin = origin;
    ship->orientation = orientation;
    ship->length = (uint8_t)length;
    ship->hits = 0;

    for (int i = 0; i < length; i++) {
        board->ship_at[bs_coord_index(bs_ship_deck(ship, i))] = (int8_t)ship_index;
    }
    board->ship_count++;
    board->ships_afloat++;
    board->decks_afloat = (uint8_t)(board->decks_afloat + length);
    return BS_OK;
}

bs_status bs_board_place_fleet_random(bs_board *board, bs_rng *rng)
{
    if (board == NULL || rng == NULL) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    for (int attempt = 0; attempt < PLACEMENT_MAX_ATTEMPTS; attempt++) {
        bs_board_init(board);
        if (try_place_fleet(board, rng)) {
            return BS_OK;
        }
    }
    bs_board_init(board);
    return BS_ERR_PLACEMENT_FAILED;
}

bool bs_board_can_fire(const bs_board *board, bs_coord target)
{
    if (board == NULL || !bs_coord_is_valid(target)) {
        return false;
    }
    const uint8_t state = board->state[bs_coord_index(target)];
    return state == BS_CELL_UNKNOWN || state == BS_CELL_EXCLUDED;
}

bs_status bs_board_fire(bs_board *board, bs_coord target, bs_shot_report *report)
{
    if (board == NULL) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    if (!bs_coord_is_valid(target)) {
        return BS_ERR_OUT_OF_BOUNDS;
    }
    if (!bs_board_can_fire(board, target)) {
        return BS_ERR_ALREADY_SHOT;
    }

    const int index = bs_coord_index(target);
    const int ship_index = board->ship_at[index];
    bs_shot_report outcome = {BS_SHOT_MISS, BS_NO_SHIP, false};

    if (ship_index == BS_NO_SHIP) {
        board->state[index] = BS_CELL_MISS;
    } else {
        /* An excluded cell is water by construction: it cannot be reached here. */
        assert(board->state[index] == BS_CELL_UNKNOWN);

        bs_ship *ship = &board->ships[ship_index];
        ship->hits++;
        board->decks_afloat--;
        outcome.ship_index = ship_index;

        if (bs_ship_is_sunk(ship)) {
            board->ships_afloat--;
            mark_sunk(board, ship);
            outcome.result = BS_SHOT_SUNK;
            outcome.fleet_destroyed = board->ships_afloat == 0;
        } else {
            board->state[index] = BS_CELL_HIT;
            exclude_diagonals(board, target);
            outcome.result = BS_SHOT_HIT;
        }
    }

    if (report != NULL) {
        *report = outcome;
    }
    return BS_OK;
}

bool bs_board_is_defeated(const bs_board *board)
{
    return board != NULL && board->ship_count > 0 && board->ships_afloat == 0;
}

void bs_board_get_knowledge(const bs_board *board, bs_knowledge *out)
{
    if (board == NULL || out == NULL) {
        return;
    }
    memset(out, 0, sizeof *out);
    memcpy(out->state, board->state, sizeof out->state);
    for (int i = 0; i < board->ship_count; i++) {
        const bs_ship *ship = &board->ships[i];
        if (!bs_ship_is_sunk(ship)) {
            out->afloat_by_length[ship->length]++;
        }
    }
}
