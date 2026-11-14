/* SPDX-License-Identifier: MIT */
#include "battleship/board.h"
#include "bs_test.h"

static bs_coord at(int row, int col)
{
    return bs_coord_make(row, col);
}

/* Number of cells of the board in the given state. */
static int count_state(const bs_board *board, bs_cell_state state)
{
    int count = 0;
    for (int i = 0; i < BS_CELL_COUNT; i++) {
        count += board->state[i] == state;
    }
    return count;
}

/* --------------------------------------------------------------- lifecycle */

TEST(new_board_is_empty)
{
    bs_board board;
    bs_board_init(&board);

    CHECK_EQ(0, board.ship_count);
    CHECK_EQ(0, board.ships_afloat);
    CHECK_EQ(0, board.decks_afloat);
    CHECK_EQ(BS_CELL_COUNT, count_state(&board, BS_CELL_UNKNOWN));
    for (int i = 0; i < BS_CELL_COUNT; i++) {
        CHECK(!bs_board_has_ship(&board, bs_coord_from_index(i)));
    }
    CHECK(!bs_board_is_defeated(&board)); /* no fleet is not a lost fleet */
}

TEST(null_and_invalid_input_is_harmless)
{
    bs_board board;
    bs_board_init(&board);
    bs_board_init(NULL);

    CHECK(!bs_board_has_ship(NULL, at(0, 0)));
    CHECK(bs_board_ship_at(&board, at(-1, 0)) == NULL);
    CHECK_EQ(BS_CELL_UNKNOWN, bs_board_cell(&board, at(10, 10)));
    CHECK(!bs_board_can_fire(&board, at(0, 10)));
    CHECK(!bs_board_can_fire(NULL, at(0, 0)));
    CHECK(!bs_board_is_defeated(NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_fire(NULL, at(0, 0), NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_place_fleet_random(&board, NULL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_place_fleet_random(NULL, NULL));
    bs_board_get_knowledge(&board, NULL);
    bs_board_get_knowledge(NULL, NULL);
}

/* --------------------------------------------------------------- placement */

TEST(places_ships_in_both_orientations)
{
    bs_board board;
    bs_board_init(&board);

    CHECK_EQ(BS_OK, bs_board_place_ship(&board, at(0, 0), 4, BS_HORIZONTAL));
    CHECK_EQ(BS_OK, bs_board_place_ship(&board, at(2, 9), 3, BS_VERTICAL));

    CHECK_EQ(2, board.ship_count);
    CHECK_EQ(2, board.ships_afloat);
    CHECK_EQ(7, board.decks_afloat);
    for (int col = 0; col < 4; col++) {
        CHECK(bs_board_has_ship(&board, at(0, col)));
    }
    CHECK(!bs_board_has_ship(&board, at(0, 4)));
    for (int row = 2; row < 5; row++) {
        CHECK(bs_board_has_ship(&board, at(row, 9)));
    }

    const bs_ship *ship = bs_board_ship_at(&board, at(3, 9));
    REQUIRE(ship != NULL);
    CHECK_EQ(3, ship->length);
    CHECK_EQ(BS_VERTICAL, ship->orientation);
    CHECK_EQ(2, ship->origin.row);
    CHECK_EQ(4, bs_ship_deck(ship, 2).row);
}

TEST(rejects_ships_outside_the_board)
{
    bs_board board;
    bs_board_init(&board);

    CHECK_EQ(BS_ERR_OUT_OF_BOUNDS, bs_board_can_place(&board, at(0, 7), 4, BS_HORIZONTAL));
    CHECK_EQ(BS_ERR_OUT_OF_BOUNDS, bs_board_can_place(&board, at(8, 0), 3, BS_VERTICAL));
    CHECK_EQ(BS_ERR_OUT_OF_BOUNDS, bs_board_can_place(&board, at(-1, 0), 1, BS_HORIZONTAL));
    CHECK_EQ(BS_ERR_OUT_OF_BOUNDS, bs_board_can_place(&board, at(0, 10), 1, BS_HORIZONTAL));
    /* The last legal origins. */
    CHECK_EQ(BS_OK, bs_board_can_place(&board, at(0, 6), 4, BS_HORIZONTAL));
    CHECK_EQ(BS_OK, bs_board_can_place(&board, at(7, 0), 3, BS_VERTICAL));
    CHECK_EQ(BS_OK, bs_board_can_place(&board, at(9, 9), 1, BS_VERTICAL));
}

TEST(rejects_invalid_ship_parameters)
{
    bs_board board;
    bs_board_init(&board);

    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_can_place(&board, at(0, 0), 0, BS_HORIZONTAL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_can_place(&board, at(0, 0), 5, BS_HORIZONTAL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_can_place(&board, at(0, 0), 2, (bs_orientation)7));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_can_place(NULL, at(0, 0), 2, BS_HORIZONTAL));
    CHECK_EQ(BS_ERR_INVALID_ARGUMENT, bs_board_place_ship(NULL, at(0, 0), 2, BS_HORIZONTAL));
    CHECK_EQ(0, board.ship_count);
}

TEST(ships_may_not_overlap_or_touch)
{
    bs_board board;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(4, 4), 2, BS_HORIZONTAL) == BS_OK); /* (4,4) (4,5) */

    CHECK_EQ(BS_ERR_COLLISION, bs_board_can_place(&board, at(4, 5), 1, BS_HORIZONTAL)); /* on top */
    CHECK_EQ(BS_ERR_COLLISION, bs_board_can_place(&board, at(3, 4), 3, BS_VERTICAL)); /* crossing */
    CHECK_EQ(BS_ERR_COLLISION, bs_board_can_place(&board, at(4, 6), 1, BS_HORIZONTAL)); /* side */
    CHECK_EQ(BS_ERR_COLLISION, bs_board_can_place(&board, at(5, 4), 1, BS_HORIZONTAL)); /* below */
    CHECK_EQ(BS_ERR_COLLISION, bs_board_can_place(&board, at(3, 3), 1, BS_HORIZONTAL)); /* corner */
    CHECK_EQ(BS_ERR_COLLISION, bs_board_can_place(&board, at(5, 6), 1, BS_HORIZONTAL)); /* corner */

    /* One free cell in between is enough. */
    CHECK_EQ(BS_OK, bs_board_can_place(&board, at(4, 7), 1, BS_HORIZONTAL));
    CHECK_EQ(BS_OK, bs_board_can_place(&board, at(6, 4), 2, BS_HORIZONTAL));
    CHECK_EQ(BS_OK, bs_board_can_place(&board, at(2, 3), 1, BS_HORIZONTAL));

    /* A rejected placement leaves the board untouched. */
    CHECK_EQ(BS_ERR_COLLISION, bs_board_place_ship(&board, at(4, 6), 1, BS_HORIZONTAL));
    CHECK_EQ(1, board.ship_count);
    CHECK_EQ(2, board.decks_afloat);
    CHECK(!bs_board_has_ship(&board, at(4, 6)));
}

TEST(fleet_cannot_exceed_ten_ships)
{
    bs_board board;
    bs_board_init(&board);
    for (int i = 0; i < BS_FLEET_SIZE; i++) {
        REQUIRE(bs_board_place_ship(&board, at((i / 5) * 2, (i % 5) * 2), 1, BS_HORIZONTAL) ==
                BS_OK);
    }
    CHECK_EQ(BS_ERR_FLEET_FULL, bs_board_place_ship(&board, at(9, 9), 1, BS_HORIZONTAL));
    CHECK_EQ(BS_FLEET_SIZE, board.ship_count);
}

/* ------------------------------------------------------------------- shots */

TEST(miss_marks_the_cell)
{
    bs_board board;
    bs_shot_report report;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(0, 0), 2, BS_HORIZONTAL) == BS_OK);

    CHECK(bs_board_can_fire(&board, at(5, 5)));
    CHECK_EQ(BS_OK, bs_board_fire(&board, at(5, 5), &report));
    CHECK_EQ(BS_SHOT_MISS, report.result);
    CHECK_EQ(BS_NO_SHIP, report.ship_index);
    CHECK(!report.fleet_destroyed);
    CHECK_EQ(BS_CELL_MISS, bs_board_cell(&board, at(5, 5)));
    CHECK_EQ(2, board.decks_afloat);
    CHECK_EQ(0, count_state(&board, BS_CELL_EXCLUDED));
}

TEST(hit_rules_out_the_diagonals)
{
    bs_board board;
    bs_shot_report report;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(4, 4), 3, BS_HORIZONTAL) == BS_OK);

    CHECK_EQ(BS_OK, bs_board_fire(&board, at(4, 5), &report));
    CHECK_EQ(BS_SHOT_HIT, report.result);
    CHECK_EQ(0, report.ship_index);
    CHECK_EQ(BS_CELL_HIT, bs_board_cell(&board, at(4, 5)));
    CHECK_EQ(2, board.decks_afloat);
    CHECK_EQ(1, board.ships_afloat);

    CHECK_EQ(BS_CELL_EXCLUDED, bs_board_cell(&board, at(3, 4)));
    CHECK_EQ(BS_CELL_EXCLUDED, bs_board_cell(&board, at(3, 6)));
    CHECK_EQ(BS_CELL_EXCLUDED, bs_board_cell(&board, at(5, 4)));
    CHECK_EQ(BS_CELL_EXCLUDED, bs_board_cell(&board, at(5, 6)));
    CHECK_EQ(4, count_state(&board, BS_CELL_EXCLUDED));
    /* The orthogonal neighbours stay open: the rest of the ship is there. */
    CHECK_EQ(BS_CELL_UNKNOWN, bs_board_cell(&board, at(4, 4)));
    CHECK_EQ(BS_CELL_UNKNOWN, bs_board_cell(&board, at(3, 5)));
}

TEST(sinking_rules_out_the_surroundings)
{
    bs_board board;
    bs_shot_report report;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(4, 4), 2, BS_VERTICAL) == BS_OK); /* (4,4) (5,4) */
    REQUIRE(bs_board_place_ship(&board, at(0, 0), 1, BS_HORIZONTAL) == BS_OK);

    REQUIRE(bs_board_fire(&board, at(4, 4), &report) == BS_OK);
    CHECK_EQ(BS_SHOT_HIT, report.result);
    REQUIRE(bs_board_fire(&board, at(5, 4), &report) == BS_OK);
    CHECK_EQ(BS_SHOT_SUNK, report.result);
    CHECK(!report.fleet_destroyed);

    CHECK_EQ(BS_CELL_SUNK, bs_board_cell(&board, at(4, 4)));
    CHECK_EQ(BS_CELL_SUNK, bs_board_cell(&board, at(5, 4)));
    CHECK_EQ(1, board.ships_afloat);
    CHECK_EQ(1, board.decks_afloat);
    CHECK(bs_ship_is_sunk(bs_board_ship_at(&board, at(4, 4))));

    /* A 2x1 ship in the open is surrounded by 4x3 - 2 = 10 cells. */
    CHECK_EQ(10, count_state(&board, BS_CELL_EXCLUDED));
    for (int row = 3; row <= 6; row++) {
        for (int col = 3; col <= 5; col++) {
            if (col != 4 || row == 3 || row == 6) {
                CHECK_EQ(BS_CELL_EXCLUDED, bs_board_cell(&board, at(row, col)));
            }
        }
    }
}

TEST(sinking_at_the_edge_stays_on_the_board)
{
    bs_board board;
    bs_shot_report report;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(0, 0), 1, BS_HORIZONTAL) == BS_OK);
    REQUIRE(bs_board_place_ship(&board, at(9, 8), 2, BS_HORIZONTAL) == BS_OK);

    REQUIRE(bs_board_fire(&board, at(0, 0), &report) == BS_OK);
    CHECK_EQ(BS_SHOT_SUNK, report.result);
    CHECK_EQ(3, count_state(&board, BS_CELL_EXCLUDED)); /* a corner has 3 neighbours */

    REQUIRE(bs_board_fire(&board, at(9, 9), NULL) == BS_OK); /* report is optional */
    REQUIRE(bs_board_fire(&board, at(9, 8), &report) == BS_OK);
    CHECK_EQ(BS_SHOT_SUNK, report.result);
    CHECK(report.fleet_destroyed);
    CHECK(bs_board_is_defeated(&board));
    CHECK_EQ(0, board.decks_afloat);
}

TEST(cannot_fire_twice_at_the_same_cell)
{
    bs_board board;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(2, 2), 2, BS_HORIZONTAL) == BS_OK);
    REQUIRE(bs_board_fire(&board, at(0, 0), NULL) == BS_OK); /* miss */
    REQUIRE(bs_board_fire(&board, at(2, 2), NULL) == BS_OK); /* hit */

    CHECK_EQ(BS_ERR_ALREADY_SHOT, bs_board_fire(&board, at(0, 0), NULL));
    CHECK_EQ(BS_ERR_ALREADY_SHOT, bs_board_fire(&board, at(2, 2), NULL));
    CHECK(!bs_board_can_fire(&board, at(0, 0)));
    CHECK_EQ(1, board.decks_afloat); /* the refused shots changed nothing */

    REQUIRE(bs_board_fire(&board, at(2, 3), NULL) == BS_OK); /* sunk */
    CHECK_EQ(BS_ERR_ALREADY_SHOT, bs_board_fire(&board, at(2, 3), NULL));
    CHECK_EQ(BS_ERR_OUT_OF_BOUNDS, bs_board_fire(&board, at(0, -1), NULL));
}

/* Firing at a cell known to be water is a legal, if wasteful, move. */
TEST(excluded_cells_can_still_be_fired_at)
{
    bs_board board;
    bs_shot_report report;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(5, 5), 1, BS_HORIZONTAL) == BS_OK);
    REQUIRE(bs_board_place_ship(&board, at(0, 0), 1, BS_HORIZONTAL) == BS_OK);
    REQUIRE(bs_board_fire(&board, at(5, 5), NULL) == BS_OK);
    REQUIRE(bs_board_cell(&board, at(4, 4)) == BS_CELL_EXCLUDED);

    CHECK(bs_board_can_fire(&board, at(4, 4)));
    CHECK_EQ(BS_OK, bs_board_fire(&board, at(4, 4), &report));
    CHECK_EQ(BS_SHOT_MISS, report.result);
    CHECK_EQ(BS_CELL_MISS, bs_board_cell(&board, at(4, 4)));
}

TEST(knowledge_reports_what_the_attacker_sees)
{
    bs_board board;
    bs_knowledge knowledge;
    bs_board_init(&board);
    REQUIRE(bs_board_place_ship(&board, at(0, 0), 3, BS_HORIZONTAL) == BS_OK);
    REQUIRE(bs_board_place_ship(&board, at(5, 5), 1, BS_HORIZONTAL) == BS_OK);
    REQUIRE(bs_board_place_ship(&board, at(9, 0), 1, BS_HORIZONTAL) == BS_OK);

    bs_board_get_knowledge(&board, &knowledge);
    CHECK_EQ(1, knowledge.afloat_by_length[3]);
    CHECK_EQ(2, knowledge.afloat_by_length[1]);
    CHECK_EQ(0, knowledge.afloat_by_length[2]);
    CHECK_EQ(0, knowledge.afloat_by_length[4]);

    REQUIRE(bs_board_fire(&board, at(5, 5), NULL) == BS_OK);
    REQUIRE(bs_board_fire(&board, at(0, 1), NULL) == BS_OK);
    bs_board_get_knowledge(&board, &knowledge);
    CHECK_EQ(1, knowledge.afloat_by_length[3]); /* wounded, not sunk */
    CHECK_EQ(1, knowledge.afloat_by_length[1]);
    CHECK_EQ(BS_CELL_SUNK, knowledge.state[bs_coord_index(at(5, 5))]);
    CHECK_EQ(BS_CELL_HIT, knowledge.state[bs_coord_index(at(0, 1))]);
    CHECK_EQ(BS_CELL_UNKNOWN, knowledge.state[bs_coord_index(at(0, 0))]); /* no leak */
}

/* ------------------------------------------------------------ random fleet */

/* Checks a complete fleet against the rules, without using the engine's own
 * placement test. */
static bool fleet_is_legal(const bs_board *board)
{
    int by_length[BS_MAX_SHIP_LENGTH + 1] = {0};
    int decks = 0;

    if (board->ship_count != BS_FLEET_SIZE || board->ships_afloat != BS_FLEET_SIZE ||
        board->decks_afloat != BS_FLEET_DECKS) {
        return false;
    }
    for (int i = 0; i < BS_FLEET_SIZE; i++) {
        by_length[board->ships[i].length]++;
    }
    if (by_length[4] != 1 || by_length[3] != 2 || by_length[2] != 3 || by_length[1] != 4) {
        return false;
    }

    for (int index = 0; index < BS_CELL_COUNT; index++) {
        const int ship = board->ship_at[index];
        if (ship == BS_NO_SHIP) {
            continue;
        }
        decks++;
        /* No deck of another ship in the eight surrounding cells. */
        const bs_coord cell = bs_coord_from_index(index);
        for (int d_row = -1; d_row <= 1; d_row++) {
            for (int d_col = -1; d_col <= 1; d_col++) {
                const bs_coord other = at(cell.row + d_row, cell.col + d_col);
                if (!bs_coord_is_valid(other)) {
                    continue;
                }
                const int neighbour = board->ship_at[bs_coord_index(other)];
                if (neighbour != BS_NO_SHIP && neighbour != ship) {
                    return false;
                }
            }
        }
    }
    return decks == BS_FLEET_DECKS;
}

TEST(random_fleets_always_follow_the_rules)
{
    bs_board board;
    bs_rng rng;
    for (uint64_t seed = 0; seed < 3000; seed++) {
        bs_rng_seed(&rng, seed);
        REQUIRE(bs_board_place_fleet_random(&board, &rng) == BS_OK);
        REQUIRE(fleet_is_legal(&board));
        CHECK_EQ(BS_CELL_COUNT, count_state(&board, BS_CELL_UNKNOWN));
    }
}

TEST(random_fleet_is_reproducible)
{
    bs_board first;
    bs_board second;
    bs_board third;
    bs_rng rng;

    bs_rng_seed(&rng, 99);
    REQUIRE(bs_board_place_fleet_random(&first, &rng) == BS_OK);
    bs_rng_seed(&rng, 99);
    REQUIRE(bs_board_place_fleet_random(&second, &rng) == BS_OK);
    bs_rng_seed(&rng, 100);
    REQUIRE(bs_board_place_fleet_random(&third, &rng) == BS_OK);

    CHECK(memcmp(first.ship_at, second.ship_at, sizeof first.ship_at) == 0);
    CHECK(memcmp(first.ship_at, third.ship_at, sizeof first.ship_at) != 0);
}

/* Over many draws every cell must receive a ship now and then: a placement
 * routine that never used, say, the last column would pass the rule check. */
TEST(random_fleets_use_the_whole_board)
{
    int occupancy[BS_CELL_COUNT] = {0};
    bs_board board;
    bs_rng rng;
    bs_rng_seed(&rng, 5);

    for (int i = 0; i < 2000; i++) {
        REQUIRE(bs_board_place_fleet_random(&board, &rng) == BS_OK);
        for (int index = 0; index < BS_CELL_COUNT; index++) {
            occupancy[index] += board.ship_at[index] != BS_NO_SHIP;
        }
    }
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        CHECK(occupancy[index] > 0);
    }
}

TEST(sinking_a_whole_random_fleet_defeats_the_board)
{
    bs_board board;
    bs_rng rng;
    bs_shot_report report = {BS_SHOT_MISS, BS_NO_SHIP, false};
    bs_rng_seed(&rng, 31337);
    REQUIRE(bs_board_place_fleet_random(&board, &rng) == BS_OK);

    int sunk = 0;
    for (int index = 0; index < BS_CELL_COUNT; index++) {
        if (board.ship_at[index] == BS_NO_SHIP) {
            continue;
        }
        CHECK(!bs_board_is_defeated(&board));
        REQUIRE(bs_board_fire(&board, bs_coord_from_index(index), &report) == BS_OK);
        CHECK(report.result != BS_SHOT_MISS);
        sunk += report.result == BS_SHOT_SUNK;
    }
    CHECK_EQ(BS_FLEET_SIZE, sunk);
    CHECK(report.fleet_destroyed);
    CHECK(bs_board_is_defeated(&board));
    CHECK_EQ(BS_FLEET_DECKS, count_state(&board, BS_CELL_SUNK));
    CHECK_EQ(0, count_state(&board, BS_CELL_HIT));
    CHECK_EQ(0, count_state(&board, BS_CELL_MISS));
}

TEST(status_codes_have_distinct_descriptions)
{
    static const bs_status all[] = {BS_OK,
                                    BS_ERR_INVALID_ARGUMENT,
                                    BS_ERR_OUT_OF_BOUNDS,
                                    BS_ERR_COLLISION,
                                    BS_ERR_FLEET_FULL,
                                    BS_ERR_ALREADY_SHOT,
                                    BS_ERR_WRONG_PHASE,
                                    BS_ERR_PLACEMENT_FAILED};
    const size_t count = sizeof all / sizeof all[0];

    for (size_t i = 0; i < count; i++) {
        REQUIRE(bs_status_str(all[i]) != NULL);
        CHECK(strlen(bs_status_str(all[i])) > 0);
        for (size_t j = i + 1; j < count; j++) {
            CHECK(strcmp(bs_status_str(all[i]), bs_status_str(all[j])) != 0);
        }
    }
    CHECK(strcmp(bs_status_str((bs_status)999), "unknown status") == 0);
}

static const bs_test_case TESTS[] = {
    BS_CASE(new_board_is_empty),
    BS_CASE(null_and_invalid_input_is_harmless),
    BS_CASE(places_ships_in_both_orientations),
    BS_CASE(rejects_ships_outside_the_board),
    BS_CASE(rejects_invalid_ship_parameters),
    BS_CASE(ships_may_not_overlap_or_touch),
    BS_CASE(fleet_cannot_exceed_ten_ships),
    BS_CASE(miss_marks_the_cell),
    BS_CASE(hit_rules_out_the_diagonals),
    BS_CASE(sinking_rules_out_the_surroundings),
    BS_CASE(sinking_at_the_edge_stays_on_the_board),
    BS_CASE(cannot_fire_twice_at_the_same_cell),
    BS_CASE(excluded_cells_can_still_be_fired_at),
    BS_CASE(knowledge_reports_what_the_attacker_sees),
    BS_CASE(random_fleets_always_follow_the_rules),
    BS_CASE(random_fleet_is_reproducible),
    BS_CASE(random_fleets_use_the_whole_board),
    BS_CASE(sinking_a_whole_random_fleet_defeats_the_board),
    BS_CASE(status_codes_have_distinct_descriptions),
};

BS_TEST_MAIN(TESTS)
