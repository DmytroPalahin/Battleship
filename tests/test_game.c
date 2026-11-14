/* SPDX-License-Identifier: MIT */
#include "battleship/game.h"
#include "bs_test.h"

/* First cell of the computer's board that does / does not hold a ship and can
 * still be fired at. The tests play with open cards to script a scenario. */
static bool find_target(const bs_game *game, bool want_ship, bs_coord *out)
{
    const bs_board *enemy = bs_game_board(game, BS_PLAYER_COMPUTER);
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        const bs_coord cell = bs_coord_from_index(index);
        if (bs_board_can_fire(enemy, cell) && bs_board_has_ship(enemy, cell) == want_ship) {
            *out = cell;
            return true;
        }
    }
    return false;
}

/* Lets the computer play until the turn comes back to the human or the game
 * ends. Returns the number of shots it fired. */
static int run_computer_turn(bs_game *game)
{
    int shots = 0;
    while (bs_game_phase(game) == BS_PHASE_COMPUTER_TURN) {
        if (bs_game_computer_fire(game, NULL) != BS_OK) {
            return -1;
        }
        shots++;
    }
    return shots;
}

static bool same_layout(const bs_board *a, const bs_board *b)
{
    return memcmp(a->ship_at, b->ship_at, sizeof a->ship_at) == 0;
}

/* ---------------------------------------------------------------- lifecycle */

TEST(new_game_is_ready_for_the_human)
{
    bs_game game;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_MEDIUM, 1) == BS_OK);

    CHECK_EQ(BS_PHASE_HUMAN_TURN, bs_game_phase(&game));
    CHECK(!bs_game_is_over(&game));
    CHECK_EQ(BS_DIFFICULTY_MEDIUM, bs_game_difficulty(&game));
    CHECK(bs_game_can_reshuffle(&game));

    for (int player = 0; player < BS_PLAYER_COUNT; player++) {
        const bs_board *board = bs_game_board(&game, (bs_player)player);
        const bs_stats *stats = bs_game_stats(&game, (bs_player)player);
        REQUIRE(board != NULL && stats != NULL);
        CHECK_EQ(BS_FLEET_SIZE, board->ship_count);
        CHECK_EQ(BS_FLEET_DECKS, board->decks_afloat);
        CHECK_EQ(0, stats->shots);
        CHECK_EQ(0, stats->hits);
        CHECK_EQ(0, stats->ships_sunk);
    }
    /* The two fleets are drawn independently. */
    CHECK(!same_layout(bs_game_board(&game, BS_PLAYER_HUMAN),
                       bs_game_board(&game, BS_PLAYER_COMPUTER)));
}

TEST(rejects_invalid_arguments)
{
    bs_game game;
    bs_coord target;

    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_init(NULL, BS_DIFFICULTY_EASY, 1));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_init(&game, (bs_difficulty)3, 1));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_init(&game, (bs_difficulty)-1, 1));
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_EASY, 1) == BS_OK);

    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_human_fire(NULL, bs_coord_make(0, 0), NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_computer_fire(NULL, NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_reshuffle(NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_suggest(NULL, BS_DIFFICULTY_EASY, &target));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_suggest(&game, BS_DIFFICULTY_EASY, NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_game_suggest(&game, (bs_difficulty)5, &target));

    CHECK(bs_game_board(NULL, BS_PLAYER_HUMAN) == NULL);
    CHECK(bs_game_board(&game, (bs_player)2) == NULL);
    CHECK(bs_game_stats(&game, (bs_player)-1) == NULL);
    CHECK(!bs_game_can_reshuffle(NULL));
    CHECK(bs_game_is_over(NULL));
    CHECK_EQ(BS_PLAYER_HUMAN, bs_game_winner(NULL));
    CHECK_EQ(BS_DIFFICULTY_EASY, bs_game_difficulty(NULL));
}

TEST(same_seed_starts_the_same_match)
{
    bs_game a;
    bs_game b;
    bs_game c;
    REQUIRE(bs_game_init(&a, BS_DIFFICULTY_HARD, 77) == BS_OK);
    REQUIRE(bs_game_init(&b, BS_DIFFICULTY_HARD, 77) == BS_OK);
    REQUIRE(bs_game_init(&c, BS_DIFFICULTY_HARD, 78) == BS_OK);

    for (int player = 0; player < BS_PLAYER_COUNT; player++) {
        CHECK(same_layout(&a.boards[player], &b.boards[player]));
    }
    CHECK(!same_layout(&a.boards[BS_PLAYER_HUMAN], &c.boards[BS_PLAYER_HUMAN]));
}

/* ---------------------------------------------------------------- reshuffle */

TEST(fleet_can_be_redrawn_until_the_first_shot)
{
    bs_game game;
    bs_board before_human;
    bs_board before_computer;
    bs_coord water;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_EASY, 5) == BS_OK);
    before_human = *bs_game_board(&game, BS_PLAYER_HUMAN);
    before_computer = *bs_game_board(&game, BS_PLAYER_COMPUTER);

    CHECK_EQ(BS_OK, bs_game_reshuffle(&game));
    CHECK(!same_layout(&before_human, bs_game_board(&game, BS_PLAYER_HUMAN)));
    CHECK(same_layout(&before_computer, bs_game_board(&game, BS_PLAYER_COMPUTER)));
    CHECK_EQ(BS_FLEET_SIZE, bs_game_board(&game, BS_PLAYER_HUMAN)->ship_count);
    CHECK_EQ(BS_OK, bs_game_reshuffle(&game)); /* as often as wanted */

    REQUIRE(find_target(&game, false, &water));
    REQUIRE(bs_game_human_fire(&game, water, NULL) == BS_OK);
    before_human = *bs_game_board(&game, BS_PLAYER_HUMAN);
    CHECK(!bs_game_can_reshuffle(&game));
    CHECK_EQ(BS_ERR_WRONG_PHASE, bs_game_reshuffle(&game));
    CHECK(same_layout(&before_human, bs_game_board(&game, BS_PLAYER_HUMAN)));
}

/* -------------------------------------------------------------------- turns */

TEST(a_miss_passes_the_turn)
{
    bs_game game;
    bs_turn_report report;
    bs_coord water;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_EASY, 9) == BS_OK);
    REQUIRE(find_target(&game, false, &water));

    REQUIRE(bs_game_human_fire(&game, water, &report) == BS_OK);
    CHECK_EQ(BS_PLAYER_HUMAN, report.shooter);
    CHECK_EQ(water.row, report.target.row);
    CHECK_EQ(water.col, report.target.col);
    CHECK_EQ(BS_SHOT_MISS, report.result);
    CHECK_EQ(BS_PHASE_COMPUTER_TURN, report.next_phase);
    CHECK_EQ(BS_PHASE_COMPUTER_TURN, bs_game_phase(&game));

    /* Out of turn: refused, and nothing is recorded. */
    REQUIRE(find_target(&game, false, &water));
    CHECK_EQ(BS_ERR_WRONG_PHASE, bs_game_human_fire(&game, water, NULL));
    CHECK_EQ(1, bs_game_stats(&game, BS_PLAYER_HUMAN)->shots);
}

TEST(a_hit_grants_another_shot)
{
    bs_game game;
    bs_turn_report report;
    bs_coord deck;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_EASY, 9) == BS_OK);
    REQUIRE(find_target(&game, true, &deck));

    REQUIRE(bs_game_human_fire(&game, deck, &report) == BS_OK);
    CHECK(report.result == BS_SHOT_HIT || report.result == BS_SHOT_SUNK);
    CHECK_EQ(BS_PHASE_HUMAN_TURN, report.next_phase);
    CHECK_EQ(BS_ERR_WRONG_PHASE, bs_game_computer_fire(&game, NULL));

    const bs_stats *stats = bs_game_stats(&game, BS_PLAYER_HUMAN);
    CHECK_EQ(1, stats->shots);
    CHECK_EQ(1, stats->hits);
}

TEST(computer_plays_until_it_misses)
{
    bs_game game;
    bs_coord water;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_HARD, 4) == BS_OK);
    REQUIRE(find_target(&game, false, &water));
    REQUIRE(bs_game_human_fire(&game, water, NULL) == BS_OK);

    const int shots = run_computer_turn(&game);
    const bs_stats *stats = bs_game_stats(&game, BS_PLAYER_COMPUTER);
    REQUIRE(shots >= 1);
    CHECK_EQ(shots, stats->shots);
    if (!bs_game_is_over(&game)) {
        CHECK_EQ(BS_PHASE_HUMAN_TURN, bs_game_phase(&game));
        CHECK_EQ(shots - 1, stats->hits); /* every shot but the last one hit */
    }
}

TEST(refused_shots_leave_the_game_untouched)
{
    bs_game game;
    bs_coord deck;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_EASY, 2) == BS_OK);
    REQUIRE(find_target(&game, true, &deck));
    REQUIRE(bs_game_human_fire(&game, deck, NULL) == BS_OK);

    CHECK_EQ(BS_ERR_ALREADY_SHOT, bs_game_human_fire(&game, deck, NULL));
    CHECK_EQ(BS_ERR_OUT_OF_BOUNDS, bs_game_human_fire(&game, bs_coord_make(10, 0), NULL));
    CHECK_EQ(1, bs_game_stats(&game, BS_PLAYER_HUMAN)->shots);
    CHECK_EQ(BS_PHASE_HUMAN_TURN, bs_game_phase(&game));
}

/* --------------------------------------------------------------- end of game */

TEST(human_wins_by_sinking_every_ship)
{
    bs_game game;
    bs_turn_report report;
    bs_coord deck;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_HARD, 123) == BS_OK);

    while (find_target(&game, true, &deck)) {
        CHECK(!bs_game_is_over(&game));
        REQUIRE(bs_game_human_fire(&game, deck, &report) == BS_OK);
    }

    CHECK(bs_game_is_over(&game));
    CHECK_EQ(BS_PHASE_OVER, report.next_phase);
    CHECK_EQ(BS_SHOT_SUNK, report.result);
    CHECK_EQ(BS_PLAYER_HUMAN, bs_game_winner(&game));
    CHECK(bs_board_is_defeated(bs_game_board(&game, BS_PLAYER_COMPUTER)));

    const bs_stats *stats = bs_game_stats(&game, BS_PLAYER_HUMAN);
    CHECK_EQ(BS_FLEET_DECKS, stats->shots); /* a perfect game */
    CHECK_EQ(BS_FLEET_DECKS, stats->hits);
    CHECK_EQ(BS_FLEET_SIZE, stats->ships_sunk);
    CHECK_EQ(0, bs_game_stats(&game, BS_PLAYER_COMPUTER)->shots);

    /* Nothing moves once the game is over. */
    CHECK_EQ(BS_ERR_WRONG_PHASE, bs_game_human_fire(&game, bs_coord_make(0, 0), NULL));
    CHECK_EQ(BS_ERR_WRONG_PHASE, bs_game_computer_fire(&game, NULL));
    CHECK_EQ(BS_ERR_WRONG_PHASE, bs_game_reshuffle(&game));
}

TEST(computer_wins_against_a_player_who_only_misses)
{
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        bs_game game;
        bs_coord water;
        REQUIRE(bs_game_init(&game, (bs_difficulty)level, 2024) == BS_OK);

        while (!bs_game_is_over(&game)) {
            REQUIRE(find_target(&game, false, &water));
            REQUIRE(bs_game_human_fire(&game, water, NULL) == BS_OK);
            REQUIRE(run_computer_turn(&game) >= 1);
        }

        CHECK_EQ(BS_PLAYER_COMPUTER, bs_game_winner(&game));
        CHECK(bs_board_is_defeated(bs_game_board(&game, BS_PLAYER_HUMAN)));
        CHECK(!bs_board_is_defeated(bs_game_board(&game, BS_PLAYER_COMPUTER)));
        CHECK_EQ(BS_FLEET_SIZE, bs_game_stats(&game, BS_PLAYER_COMPUTER)->ships_sunk);
        CHECK_EQ(0, bs_game_stats(&game, BS_PLAYER_HUMAN)->hits);
    }
}

/* ---------------------------------------------------------------- suggestion */

TEST(suggestions_are_legal_moves)
{
    bs_game game;
    REQUIRE(bs_game_init(&game, BS_DIFFICULTY_MEDIUM, 6) == BS_OK);

    for (int i = 0; i < 30 && bs_game_phase(&game) == BS_PHASE_HUMAN_TURN; i++) {
        bs_coord target;
        REQUIRE(bs_game_suggest(&game, BS_DIFFICULTY_HARD, &target) == BS_OK);
        CHECK(bs_board_can_fire(bs_game_board(&game, BS_PLAYER_COMPUTER), target));
        REQUIRE(bs_game_human_fire(&game, target, NULL) == BS_OK);
        REQUIRE(run_computer_turn(&game) >= 0);
    }
}

/* Plays a whole match with both sides on autopilot. */
static void autoplay(bs_game *game, bs_difficulty human_level)
{
    while (!bs_game_is_over(game)) {
        if (bs_game_phase(game) == BS_PHASE_HUMAN_TURN) {
            bs_coord target;
            if (bs_game_suggest(game, human_level, &target) != BS_OK ||
                bs_game_human_fire(game, target, NULL) != BS_OK) {
                return;
            }
        } else if (bs_game_computer_fire(game, NULL) != BS_OK) {
            return;
        }
    }
}

TEST(a_seed_replays_the_whole_match)
{
    bs_game a;
    bs_game b;
    REQUIRE(bs_game_init(&a, BS_DIFFICULTY_HARD, 4242) == BS_OK);
    REQUIRE(bs_game_init(&b, BS_DIFFICULTY_HARD, 4242) == BS_OK);
    autoplay(&a, BS_DIFFICULTY_MEDIUM);
    autoplay(&b, BS_DIFFICULTY_MEDIUM);

    REQUIRE(bs_game_is_over(&a) && bs_game_is_over(&b));
    CHECK_EQ(bs_game_winner(&a), bs_game_winner(&b));
    for (int player = 0; player < BS_PLAYER_COUNT; player++) {
        CHECK(memcmp(a.boards[player].state, b.boards[player].state,
                     sizeof a.boards[player].state) == 0);
        CHECK_EQ(a.stats[player].shots, b.stats[player].shots);
        CHECK_EQ(a.stats[player].hits, b.stats[player].hits);
    }
}

static const bs_test_case TESTS[] = {
    BS_CASE(new_game_is_ready_for_the_human),
    BS_CASE(rejects_invalid_arguments),
    BS_CASE(same_seed_starts_the_same_match),
    BS_CASE(fleet_can_be_redrawn_until_the_first_shot),
    BS_CASE(a_miss_passes_the_turn),
    BS_CASE(a_hit_grants_another_shot),
    BS_CASE(computer_plays_until_it_misses),
    BS_CASE(refused_shots_leave_the_game_untouched),
    BS_CASE(human_wins_by_sinking_every_ship),
    BS_CASE(computer_wins_against_a_player_who_only_misses),
    BS_CASE(suggestions_are_legal_moves),
    BS_CASE(a_seed_replays_the_whole_match),
};

BS_TEST_MAIN(TESTS)
