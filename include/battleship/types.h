/* SPDX-License-Identifier: MIT */
/**
 * @file types.h
 * @brief Vocabulary types shared by every module of the game engine.
 *
 * The engine ("the model" in MVC terms) is plain C11 with no dependency other
 * than the C standard library. Nothing in here allocates memory.
 */
#ifndef BATTLESHIP_TYPES_H
#define BATTLESHIP_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Side length of the (square) board. */
#define BS_BOARD_SIZE 10
/** Total number of cells on a board. */
#define BS_CELL_COUNT (BS_BOARD_SIZE * BS_BOARD_SIZE)
/** Longest ship of the classic fleet. */
#define BS_MAX_SHIP_LENGTH 4
/** Number of ships in a fleet: 1x4, 2x3, 3x2, 4x1. */
#define BS_FLEET_SIZE 10
/** Number of deck cells in a full fleet: 4 + 2*3 + 3*2 + 4*1. */
#define BS_FLEET_DECKS 20

/** Outcome of an engine call. Zero is success, everything else is an error. */
typedef enum bs_status {
    BS_OK = 0,
    BS_ERR_INVALID_ARGUMENT, /**< NULL pointer or value outside its domain. */
    BS_ERR_OUT_OF_BOUNDS,    /**< Coordinate or ship outside the board. */
    BS_ERR_COLLISION,        /**< Ship would overlap or touch another ship. */
    BS_ERR_FLEET_FULL,       /**< Every ship of the fleet is already placed. */
    BS_ERR_ALREADY_SHOT,     /**< Target cell has already been fired at. */
    BS_ERR_WRONG_PHASE,      /**< Action is not allowed in the current phase. */
    BS_ERR_PLACEMENT_FAILED  /**< Random placement exhausted its attempts. */
} bs_status;

/** Human-readable, static description of a status code. Never returns NULL. */
const char *bs_status_str(bs_status status);

/** A cell position. Row 0 is the top row, column 0 the leftmost column. */
typedef struct bs_coord {
    int row;
    int col;
} bs_coord;

/** Direction in which a ship extends from its origin. */
typedef enum bs_orientation {
    BS_HORIZONTAL = 0, /**< Towards increasing columns. */
    BS_VERTICAL = 1    /**< Towards increasing rows. */
} bs_orientation;

/** The two sides of a game. Also used as an array index. */
typedef enum bs_player {
    BS_PLAYER_HUMAN = 0,
    BS_PLAYER_COMPUTER = 1
} bs_player;

/** Number of sides; handy for array sizes. */
#define BS_PLAYER_COUNT 2

/** Builds a coordinate from a row and a column. */
static inline bs_coord bs_coord_make(int row, int col)
{
    bs_coord c = {row, col};
    return c;
}

/** True when the coordinate lies on the board. */
static inline bool bs_coord_is_valid(bs_coord c)
{
    return c.row >= 0 && c.row < BS_BOARD_SIZE && c.col >= 0 && c.col < BS_BOARD_SIZE;
}

/** Row-major index of a valid coordinate, in [0, BS_CELL_COUNT). */
static inline int bs_coord_index(bs_coord c)
{
    return c.row * BS_BOARD_SIZE + c.col;
}

/** Inverse of bs_coord_index(). */
static inline bs_coord bs_coord_from_index(int index)
{
    return bs_coord_make(index / BS_BOARD_SIZE, index % BS_BOARD_SIZE);
}

/** The opponent of a side. */
static inline bs_player bs_player_other(bs_player p)
{
    return p == BS_PLAYER_HUMAN ? BS_PLAYER_COMPUTER : BS_PLAYER_HUMAN;
}

#ifdef __cplusplus
}
#endif

#endif /* BATTLESHIP_TYPES_H */
