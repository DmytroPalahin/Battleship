/* SPDX-License-Identifier: MIT */
/**
 * @file battleship.h
 * @brief Umbrella header of the Battleship game engine.
 *
 * @mainpage Battleship engine
 *
 * The engine is the *model* of the application: a dependency-free C11 library
 * that knows the rules of Battleship and nothing about how the game is shown.
 *
 * | Header     | Responsibility                                          |
 * |------------|---------------------------------------------------------|
 * | types.h    | coordinates, status codes, shared constants             |
 * | rng.h      | seedable random numbers                                 |
 * | board.h    | ship placement, shots, derived knowledge                |
 * | ai.h       | computer opponent                                       |
 * | game.h     | turn order, score, end of game                          |
 *
 * Start with bs_game_init(), then alternate bs_game_human_fire() and
 * bs_game_computer_fire() according to bs_game_phase().
 */
#ifndef BATTLESHIP_BATTLESHIP_H
#define BATTLESHIP_BATTLESHIP_H

#include "battleship/ai.h"
#include "battleship/board.h"
#include "battleship/game.h"
#include "battleship/rng.h"
#include "battleship/types.h"
#include "battleship/version.h"

#endif /* BATTLESHIP_BATTLESHIP_H */
