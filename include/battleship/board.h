/* SPDX-License-Identifier: MIT */
/**
 * @file board.h
 * @brief One side's ocean: where the ships are and what has been fired at.
 *
 * A board owns two kinds of information:
 *  - the secret truth (ship positions), read through bs_board_ship_at();
 *  - the public shot history, read through bs_board_cell() and
 *    bs_board_get_knowledge().
 *
 * Classic "no touching" rules apply: two ships may never occupy adjacent
 * cells, not even diagonally. The board uses that rule to derive cells that
 * cannot hold a ship (BS_CELL_EXCLUDED) as soon as the shots prove it.
 *
 * Invariants maintained by every function of this module:
 *  - ships never overlap, never touch and never leave the board;
 *  - decks_afloat equals the number of deck cells not yet hit;
 *  - a cell is BS_CELL_SUNK iff it belongs to a ship whose every deck was hit.
 */
#ifndef BATTLESHIP_BOARD_H
#define BATTLESHIP_BOARD_H

#include "battleship/rng.h"
#include "battleship/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Value of bs_board::ship_at for a cell without a ship. */
#define BS_NO_SHIP (-1)

/** Lengths of the ships of the classic fleet, longest first. */
extern const uint8_t BS_FLEET_LENGTHS[BS_FLEET_SIZE];

/** What an attacker can see of a cell. */
typedef enum bs_cell_state {
    BS_CELL_UNKNOWN = 0, /**< Never fired at, nothing known. */
    BS_CELL_MISS,        /**< Fired at: water. */
    BS_CELL_HIT,         /**< Fired at: deck of a ship that is still afloat. */
    BS_CELL_SUNK,        /**< Fired at: deck of a sunk ship. */
    BS_CELL_EXCLUDED     /**< Not fired at, but provably water. */
} bs_cell_state;

/** A placed ship. */
typedef struct bs_ship {
    bs_coord origin;            /**< Top-left deck. */
    bs_orientation orientation; /**< Direction of the remaining decks. */
    uint8_t length;             /**< Number of decks, 1..BS_MAX_SHIP_LENGTH. */
    uint8_t hits;               /**< Number of decks hit so far. */
} bs_ship;

/** One side's board. Zero-allocation; copy with plain assignment. */
typedef struct bs_board {
    bs_ship ships[BS_FLEET_SIZE];
    uint8_t ship_count;            /**< Ships placed so far. */
    uint8_t ships_afloat;          /**< Placed ships not yet sunk. */
    uint8_t decks_afloat;          /**< Deck cells not yet hit. */
    int8_t ship_at[BS_CELL_COUNT]; /**< Ship index per cell, or BS_NO_SHIP. */
    uint8_t state[BS_CELL_COUNT];  /**< bs_cell_state per cell. */
} bs_board;

/** Result of a shot. */
typedef enum bs_shot_result {
    BS_SHOT_MISS = 0,
    BS_SHOT_HIT,
    BS_SHOT_SUNK
} bs_shot_result;

/** Everything a caller may want to know about a shot that was just fired. */
typedef struct bs_shot_report {
    bs_shot_result result;
    int ship_index;       /**< Index of the ship hit, or BS_NO_SHIP. */
    bool fleet_destroyed; /**< True when this shot sank the last ship. */
} bs_shot_report;

/**
 * Public information about a board: exactly what the attacker is entitled to
 * know. The AI only ever receives this structure, so it cannot cheat.
 */
typedef struct bs_knowledge {
    uint8_t state[BS_CELL_COUNT];                     /**< bs_cell_state per cell. */
    uint8_t afloat_by_length[BS_MAX_SHIP_LENGTH + 1]; /**< Ships left, by length. */
} bs_knowledge;

/** Empties the board: no ships, no shots. */
void bs_board_init(bs_board *board);

/** True when a ship is afloat or sunk on that cell. False for invalid input. */
bool bs_board_has_ship(const bs_board *board, bs_coord cell);

/** Ship occupying the cell, or NULL. */
const bs_ship *bs_board_ship_at(const bs_board *board, bs_coord cell);

/** Attacker-visible state of a cell. BS_CELL_UNKNOWN for invalid input. */
bs_cell_state bs_board_cell(const bs_board *board, bs_coord cell);

/** Coordinate of the i-th deck of a ship, i in [0, ship->length). */
bs_coord bs_ship_deck(const bs_ship *ship, int i);

/** True once every deck of the ship has been hit. */
bool bs_ship_is_sunk(const bs_ship *ship);

/**
 * Checks whether a ship fits at the given position.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT, BS_ERR_OUT_OF_BOUNDS or
 *         BS_ERR_COLLISION (overlap or contact with another ship).
 */
bs_status bs_board_can_place(const bs_board *board, bs_coord origin, int length,
                             bs_orientation orientation);

/**
 * Places a ship. The board is left untouched on error.
 * @return as bs_board_can_place(), plus BS_ERR_FLEET_FULL.
 */
bs_status bs_board_place_ship(bs_board *board, bs_coord origin, int length,
                              bs_orientation orientation);

/**
 * Clears the board and places the classic fleet at random.
 * Every legal layout reachable by sequential placement can be produced.
 * @return BS_OK, or BS_ERR_PLACEMENT_FAILED if no layout was found (the board
 *         is then empty). With the classic fleet this does not happen in
 *         practice; the bound only guarantees termination.
 */
bs_status bs_board_place_fleet_random(bs_board *board, bs_rng *rng);

/** True when the cell has not been fired at yet (it may be excluded). */
bool bs_board_can_fire(const bs_board *board, bs_coord target);

/**
 * Fires at a cell and updates the shot history and the derived exclusions.
 * @param report optional; filled on success.
 * @return BS_OK, BS_ERR_INVALID_ARGUMENT, BS_ERR_OUT_OF_BOUNDS or
 *         BS_ERR_ALREADY_SHOT.
 */
bs_status bs_board_fire(bs_board *board, bs_coord target, bs_shot_report *report);

/** True when the board holds at least one ship and all of them are sunk. */
bool bs_board_is_defeated(const bs_board *board);

/** Copies the attacker-visible part of the board into @p out. */
void bs_board_get_knowledge(const bs_board *board, bs_knowledge *out);

#ifdef __cplusplus
}
#endif

#endif /* BATTLESHIP_BOARD_H */
