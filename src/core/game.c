/* SPDX-License-Identifier: MIT */
#include "battleship/game.h"

#include <stddef.h>
#include <string.h>

static bool difficulty_is_valid(bs_difficulty difficulty)
{
    return (int)difficulty >= 0 && (int)difficulty < BS_DIFFICULTY_COUNT;
}

static bool player_is_valid(bs_player player)
{
    return player == BS_PLAYER_HUMAN || player == BS_PLAYER_COMPUTER;
}

/* Applies a shot and moves the state machine. The game is unchanged on error. */
static bs_status fire(bs_game *game, bs_player shooter, bs_coord target, bs_turn_report *report)
{
    bs_shot_report shot;
    const bs_status status = bs_board_fire(&game->boards[bs_player_other(shooter)], target, &shot);
    if (status != BS_OK) {
        return status;
    }

    bs_stats *stats = &game->stats[shooter];
    stats->shots++;
    if (shot.result != BS_SHOT_MISS) {
        stats->hits++;
    }
    if (shot.result == BS_SHOT_SUNK) {
        stats->ships_sunk++;
    }

    if (shot.fleet_destroyed) {
        game->phase = BS_PHASE_OVER;
        game->winner = shooter;
    } else if (shot.result == BS_SHOT_MISS) {
        game->phase = shooter == BS_PLAYER_HUMAN ? BS_PHASE_COMPUTER_TURN : BS_PHASE_HUMAN_TURN;
    }
    /* A hit that does not end the game leaves the turn with the shooter. */

    if (report != NULL) {
        report->shooter = shooter;
        report->target = target;
        report->result = shot.result;
        report->next_phase = game->phase;
    }
    return BS_OK;
}

bs_status bs_game_init(bs_game *game, bs_difficulty difficulty, uint64_t seed)
{
    if (game == NULL || !difficulty_is_valid(difficulty)) {
        return BS_ERR_INVALID_ARGUMENT;
    }

    memset(game, 0, sizeof *game);
    game->difficulty = difficulty;
    game->phase = BS_PHASE_HUMAN_TURN;
    game->winner = BS_PLAYER_HUMAN;
    bs_rng_seed(&game->rng, seed);

    for (int player = 0; player < BS_PLAYER_COUNT; player++) {
        const bs_status status = bs_board_place_fleet_random(&game->boards[player], &game->rng);
        if (status != BS_OK) {
            return status;
        }
    }
    return BS_OK;
}

bs_phase bs_game_phase(const bs_game *game)
{
    return game == NULL ? BS_PHASE_OVER : game->phase;
}

bool bs_game_is_over(const bs_game *game)
{
    return bs_game_phase(game) == BS_PHASE_OVER;
}

bs_player bs_game_winner(const bs_game *game)
{
    return game == NULL ? BS_PLAYER_HUMAN : game->winner;
}

bs_difficulty bs_game_difficulty(const bs_game *game)
{
    return game == NULL ? BS_DIFFICULTY_EASY : game->difficulty;
}

const bs_board *bs_game_board(const bs_game *game, bs_player owner)
{
    if (game == NULL || !player_is_valid(owner)) {
        return NULL;
    }
    return &game->boards[owner];
}

const bs_stats *bs_game_stats(const bs_game *game, bs_player shooter)
{
    if (game == NULL || !player_is_valid(shooter)) {
        return NULL;
    }
    return &game->stats[shooter];
}

bool bs_game_can_reshuffle(const bs_game *game)
{
    return game != NULL && game->phase == BS_PHASE_HUMAN_TURN &&
           game->stats[BS_PLAYER_HUMAN].shots == 0 && game->stats[BS_PLAYER_COMPUTER].shots == 0;
}

bs_status bs_game_reshuffle(bs_game *game)
{
    if (game == NULL) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    if (!bs_game_can_reshuffle(game)) {
        return BS_ERR_WRONG_PHASE;
    }

    /* Draw into a scratch board so that a failure cannot leave an empty fleet. */
    bs_board fresh;
    const bs_status status = bs_board_place_fleet_random(&fresh, &game->rng);
    if (status == BS_OK) {
        game->boards[BS_PLAYER_HUMAN] = fresh;
    }
    return status;
}

bs_status bs_game_human_fire(bs_game *game, bs_coord target, bs_turn_report *report)
{
    if (game == NULL) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    if (game->phase != BS_PHASE_HUMAN_TURN) {
        return BS_ERR_WRONG_PHASE;
    }
    return fire(game, BS_PLAYER_HUMAN, target, report);
}

bs_status bs_game_computer_fire(bs_game *game, bs_turn_report *report)
{
    if (game == NULL) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    if (game->phase != BS_PHASE_COMPUTER_TURN) {
        return BS_ERR_WRONG_PHASE;
    }

    bs_knowledge knowledge;
    bs_coord target;
    bs_board_get_knowledge(&game->boards[BS_PLAYER_HUMAN], &knowledge);
    const bs_status status = bs_ai_choose_target(game->difficulty, &knowledge, &game->rng, &target);
    if (status != BS_OK) {
        return status;
    }
    return fire(game, BS_PLAYER_COMPUTER, target, report);
}

bs_status bs_game_suggest(bs_game *game, bs_difficulty level, bs_coord *target)
{
    if (game == NULL || target == NULL || !difficulty_is_valid(level)) {
        return BS_ERR_INVALID_ARGUMENT;
    }
    bs_knowledge knowledge;
    bs_board_get_knowledge(&game->boards[BS_PLAYER_COMPUTER], &knowledge);
    return bs_ai_choose_target(level, &knowledge, &game->rng, target);
}
