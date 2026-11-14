/* SPDX-License-Identifier: MIT */
/*
 * Property test: thousands of complete, randomly seeded matches are played by
 * the engine against itself, and the invariants documented in board.h and
 * game.h are re-verified from scratch after every single shot.
 *
 * Run under AddressSanitizer and UBSan in CI, this doubles as a fuzzing pass
 * over the whole engine.
 */
#include "battleship/game.h"
#include "bs_test.h"

/* Recomputes everything the board caches and compares it with the cache. */
static bool board_is_consistent(const bs_board *board)
{
    int decks_afloat = 0;
    int ships_afloat = 0;

    for (int i = 0; i < board->ship_count; i++) {
        const bs_ship *ship = &board->ships[i];
        int hits = 0;
        for (int deck = 0; deck < ship->length; deck++) {
            const bs_coord cell = bs_ship_deck(ship, deck);
            if (!bs_coord_is_valid(cell) || board->ship_at[bs_coord_index(cell)] != i) {
                return false;
            }
            const bs_cell_state state = bs_board_cell(board, cell);
            const bool sunk = bs_ship_is_sunk(ship);
            /* A deck is unknown or hit while its ship floats, sunk afterwards. */
            if (sunk ? state != BS_CELL_SUNK : (state != BS_CELL_UNKNOWN && state != BS_CELL_HIT)) {
                return false;
            }
            hits += state == BS_CELL_HIT || state == BS_CELL_SUNK;
        }
        if (hits != ship->hits) {
            return false;
        }
        decks_afloat += ship->length - hits;
        ships_afloat += !bs_ship_is_sunk(ship);
    }

    for (int index = 0; index < BS_CELL_COUNT; index++) {
        const bool has_ship = board->ship_at[index] != BS_NO_SHIP;
        const uint8_t state = board->state[index];
        /* Water is never reported as a hit, a ship never as water. */
        if (!has_ship && (state == BS_CELL_HIT || state == BS_CELL_SUNK)) {
            return false;
        }
        if (has_ship && (state == BS_CELL_MISS || state == BS_CELL_EXCLUDED)) {
            return false;
        }
    }
    return decks_afloat == board->decks_afloat && ships_afloat == board->ships_afloat;
}

static bool game_is_consistent(const bs_game *game)
{
    for (int shooter = 0; shooter < BS_PLAYER_COUNT; shooter++) {
        const bs_board *target = &game->boards[bs_player_other((bs_player)shooter)];
        const bs_stats *stats = &game->stats[shooter];
        int fired = 0;

        if (!board_is_consistent(target)) {
            return false;
        }
        for (int index = 0; index < BS_CELL_COUNT; index++) {
            const uint8_t state = target->state[index];
            fired += state == BS_CELL_MISS || state == BS_CELL_HIT || state == BS_CELL_SUNK;
        }
        /* The score is exactly what can be read off the board. */
        if ((int)stats->shots != fired ||
            (int)stats->hits != BS_FLEET_DECKS - target->decks_afloat ||
            (int)stats->ships_sunk != BS_FLEET_SIZE - target->ships_afloat) {
            return false;
        }
    }

    const bool human_lost = bs_board_is_defeated(&game->boards[BS_PLAYER_HUMAN]);
    const bool computer_lost = bs_board_is_defeated(&game->boards[BS_PLAYER_COMPUTER]);
    if (human_lost && computer_lost) {
        return false;
    }
    if (bs_game_is_over(game)) {
        return bs_game_winner(game) == (human_lost ? BS_PLAYER_COMPUTER : BS_PLAYER_HUMAN) &&
               (human_lost || computer_lost);
    }
    return !human_lost && !computer_lost;
}

/* Plays one match to the end; false as soon as an invariant breaks. */
static bool play_checked_match(bs_difficulty computer, bs_difficulty human, uint64_t seed)
{
    /* Each side fires at most once per cell: 200 shots is a hard ceiling. */
    enum {
        MAX_SHOTS = 2 * BS_CELL_COUNT
    };
    bs_game game;

    if (bs_game_init(&game, computer, seed) != BS_OK || !game_is_consistent(&game)) {
        return false;
    }
    for (int shot = 0; shot < MAX_SHOTS && !bs_game_is_over(&game); shot++) {
        const bs_phase phase = bs_game_phase(&game);
        bs_turn_report report;
        bs_status status;

        if (phase == BS_PHASE_HUMAN_TURN) {
            bs_coord target;
            if (bs_game_suggest(&game, human, &target) != BS_OK) {
                return false;
            }
            status = bs_game_human_fire(&game, target, &report);
        } else {
            status = bs_game_computer_fire(&game, &report);
        }
        if (status != BS_OK || report.next_phase != bs_game_phase(&game)) {
            return false;
        }
        /* The turn only changes hands on a miss. */
        const bool turn_kept = report.next_phase == phase;
        if (!bs_game_is_over(&game) && turn_kept != (report.result != BS_SHOT_MISS)) {
            return false;
        }
        if (!game_is_consistent(&game)) {
            return false;
        }
    }
    return bs_game_is_over(&game);
}

TEST(invariants_hold_through_thousands_of_matches)
{
    uint64_t seed = 0;
    for (int computer = 0; computer < BS_DIFFICULTY_COUNT; computer++) {
        for (int human = 0; human < BS_DIFFICULTY_COUNT; human++) {
            for (int match = 0; match < 250; match++, seed++) {
                if (!play_checked_match((bs_difficulty)computer, (bs_difficulty)human, seed)) {
                    fprintf(stderr, "# failing match: computer=%s human=%s seed=%llu\n",
                            bs_difficulty_name((bs_difficulty)computer),
                            bs_difficulty_name((bs_difficulty)human), (unsigned long long)seed);
                    REQUIRE(false);
                }
            }
        }
    }
}

/* The stronger side should win most matches, even though the human opens. */
TEST(a_stronger_computer_wins_more_often)
{
    enum {
        MATCHES = 400
    };
    int computer_wins[BS_DIFFICULTY_COUNT] = {0};

    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        for (uint64_t seed = 0; seed < MATCHES; seed++) {
            bs_game game;
            REQUIRE(bs_game_init(&game, (bs_difficulty)level, seed) == BS_OK);
            while (!bs_game_is_over(&game)) {
                if (bs_game_phase(&game) == BS_PHASE_HUMAN_TURN) {
                    bs_coord target;
                    REQUIRE(bs_game_suggest(&game, BS_DIFFICULTY_MEDIUM, &target) == BS_OK);
                    REQUIRE(bs_game_human_fire(&game, target, NULL) == BS_OK);
                } else {
                    REQUIRE(bs_game_computer_fire(&game, NULL) == BS_OK);
                }
            }
            computer_wins[level] += bs_game_winner(&game) == BS_PLAYER_COMPUTER;
        }
    }

    printf("# computer wins out of %d against a medium-level player: easy %d, medium %d, "
           "hard %d\n",
           MATCHES, computer_wins[BS_DIFFICULTY_EASY], computer_wins[BS_DIFFICULTY_MEDIUM],
           computer_wins[BS_DIFFICULTY_HARD]);
    CHECK(computer_wins[BS_DIFFICULTY_EASY] < computer_wins[BS_DIFFICULTY_MEDIUM]);
    CHECK(computer_wins[BS_DIFFICULTY_MEDIUM] < computer_wins[BS_DIFFICULTY_HARD]);
}

static const bs_test_case TESTS[] = {
    BS_CASE(invariants_hold_through_thousands_of_matches),
    BS_CASE(a_stronger_computer_wins_more_often),
};

BS_TEST_MAIN(TESTS)
