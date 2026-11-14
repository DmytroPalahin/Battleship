/* SPDX-License-Identifier: MIT */
#include "controller.h"

#include "audio.h"
#include "texts.h"
#include "view.h"

/* Pause between two shots of the computer, so that the player can follow. */
#define TURN_DELAY_MS 550
/* Same, when the computer plays both sides for a test. */
#define AUTOPLAY_DELAY_MS 12
/* Time left to GTK to settle the layout before the window is captured. */
#define CAPTURE_DELAY_MS 600

struct bs_controller {
    GtkApplication *application;
    bs_game *model;  /* borrowed: owned by the application */
    bs_view *view;   /* owned */
    bs_audio *audio; /* owned; NULL when sound is off */

    bs_controller_options options;
    char *screenshot_path; /* owned copy of options.screenshot_path */

    bs_rng seeds;    /* draws the seed of each new game */
    bool first_game; /* the first game may use the seed of the command line */
    bool in_game;    /* a game is displayed: the model is initialised */
    guint timer;     /* pending timeout, or 0 */
    int autoplay_shots;
    int exit_status;
};

static void schedule_tick(bs_controller *controller);

/* ---------------------------------------------------------- model -> view */

/* Look of a cell of the player's own board: ships are visible. */
static bs_cell_look own_look(const bs_board *board, bs_coord cell, bool show_deductions)
{
    switch (bs_board_cell(board, cell)) {
    case BS_CELL_MISS:
        return BS_LOOK_MISS;
    case BS_CELL_HIT:
        return BS_LOOK_HIT;
    case BS_CELL_SUNK:
        return BS_LOOK_SUNK;
    case BS_CELL_EXCLUDED:
        /* The flags show what the computer has worked out. At the easy level
         * it works nothing out, so there is nothing to show. */
        return show_deductions ? BS_LOOK_EXCLUDED : BS_LOOK_WATER;
    case BS_CELL_UNKNOWN:
        break;
    }
    return bs_board_has_ship(board, cell) ? BS_LOOK_SHIP : BS_LOOK_WATER;
}

/* Look of a cell of the enemy board: ships stay hidden until the game ends. */
static bs_cell_look enemy_look(const bs_board *board, bs_coord cell, bool reveal)
{
    switch (bs_board_cell(board, cell)) {
    case BS_CELL_MISS:
        return BS_LOOK_MISS;
    case BS_CELL_HIT:
        return BS_LOOK_HIT;
    case BS_CELL_SUNK:
        return BS_LOOK_SUNK;
    case BS_CELL_EXCLUDED:
        return BS_LOOK_EXCLUDED;
    case BS_CELL_UNKNOWN:
        break;
    }
    return reveal && bs_board_has_ship(board, cell) ? BS_LOOK_SHIP : BS_LOOK_WATER;
}

/*
 * Brings the view in line with the model. Called after every change: the
 * view holds no game state of its own, so it can never drift out of sync.
 */
static void synchronise(bs_controller *controller)
{
    const bs_game *game = controller->model;
    const bs_phase phase = bs_game_phase(game);
    const bool over = phase == BS_PHASE_OVER;
    const bool player_may_fire = phase == BS_PHASE_HUMAN_TURN;
    const bool show_deductions = bs_game_difficulty(game) != BS_DIFFICULTY_EASY;
    const bs_board *own = bs_game_board(game, BS_PLAYER_HUMAN);
    const bs_board *enemy = bs_game_board(game, BS_PLAYER_COMPUTER);

    for (int index = 0; index < BS_CELL_COUNT; index++) {
        const bs_coord cell = bs_coord_from_index(index);
        bs_view_set_cell(controller->view, BS_PLAYER_HUMAN, cell,
                         own_look(own, cell, show_deductions), false);
        /* Flagged cells are locked: firing there would be a wasted shot. */
        const bool playable = player_may_fire && bs_board_cell(enemy, cell) == BS_CELL_UNKNOWN;
        bs_view_set_cell(controller->view, BS_PLAYER_COMPUTER, cell, enemy_look(enemy, cell, over),
                         playable);
    }

    for (int shooter = 0; shooter < BS_PLAYER_COUNT; shooter++) {
        const bs_stats *stats = bs_game_stats(game, (bs_player)shooter);
        bs_view_set_score(controller->view, (bs_player)shooter, stats->shots, stats->hits,
                          stats->ships_sunk);
    }
    bs_view_set_level(controller->view, bs_game_difficulty(game));
    bs_view_set_reshuffle_enabled(controller->view, bs_game_can_reshuffle(game));

    if (over) {
        const bool victory = bs_game_winner(game) == BS_PLAYER_HUMAN;
        bs_view_set_status(controller->view,
                           victory ? BS_TEXT_VICTORY_STATUS : BS_TEXT_DEFEAT_STATUS,
                           victory ? BS_TONE_VICTORY : BS_TONE_DEFEAT);
    } else if (phase == BS_PHASE_COMPUTER_TURN) {
        bs_view_set_status(controller->view, BS_TEXT_STATUS_COMPUTER_TURN, BS_TONE_NEUTRAL);
    } else {
        bs_view_set_status(controller->view,
                           bs_game_can_reshuffle(game) ? BS_TEXT_STATUS_READY
                                                       : BS_TEXT_STATUS_HUMAN_TURN,
                           BS_TONE_NEUTRAL);
    }
}

/* ------------------------------------------------------------------ timers */

static void cancel_timer(bs_controller *controller)
{
    if (controller->timer != 0) {
        g_source_remove(controller->timer);
        controller->timer = 0;
    }
}

/*
 * Test hook: writes the screenshot if one was asked for, then quits. If the
 * end-of-game question is open it is answered "quit", so that the test also
 * leaves through the same door as a player would.
 */
static gboolean on_capture_and_quit(gpointer user_data)
{
    bs_controller *controller = user_data;
    controller->timer = 0;

    if (controller->screenshot_path != NULL) {
        GError *error = NULL;
        if (!bs_view_save_png(controller->view, controller->screenshot_path, &error)) {
            g_printerr("battleship: %s\n", error->message);
            g_clear_error(&error);
            controller->exit_status = 1;
        }
    }
    /* An unattended run is a test: a sound that failed to play fails it. */
    if (controller->options.autoplay && bs_audio_error_count(controller->audio) > 0) {
        g_printerr("battleship: a sound effect could not be played\n");
        controller->exit_status = 1;
    }
    if (!bs_view_answer_game_over(controller->view, BS_CHOICE_QUIT)) {
        g_application_quit(G_APPLICATION(controller->application));
    }
    return G_SOURCE_REMOVE;
}

static void capture_and_quit_soon(bs_controller *controller)
{
    cancel_timer(controller);
    controller->timer = g_timeout_add(CAPTURE_DELAY_MS, on_capture_and_quit, controller);
}

/* ------------------------------------------------------------------- shots */

/* Everything that follows a shot, whoever fired it. */
static void after_shot(bs_controller *controller, const bs_turn_report *report)
{
    const bool autoplay = controller->options.autoplay;

    bs_audio_play(controller->audio, report->result == BS_SHOT_MISS ? BS_SOUND_MISS : BS_SOUND_HIT);
    synchronise(controller);

    if (report->next_phase == BS_PHASE_OVER) {
        bs_view_ask_game_over(controller->view,
                              bs_game_winner(controller->model) == BS_PLAYER_HUMAN);
        if (autoplay) {
            capture_and_quit_soon(controller);
        }
        return;
    }

    if (autoplay) {
        controller->autoplay_shots++;
        if (controller->options.max_shots > 0 &&
            controller->autoplay_shots >= controller->options.max_shots) {
            capture_and_quit_soon(controller);
            return;
        }
    }
    if (autoplay || report->next_phase == BS_PHASE_COMPUTER_TURN) {
        schedule_tick(controller);
    }
}

/*
 * One step of the computer. It fires a single shot per tick, which leaves the
 * event loop free to redraw between shots: the player sees every move, and
 * the interface never freezes.
 *
 * In autoplay the tick also plays for the human, by clicking the cell an AI
 * would choose; the click then follows the normal path of a user action.
 */
static gboolean on_tick(gpointer user_data)
{
    bs_controller *controller = user_data;
    const bs_phase phase = bs_game_phase(controller->model);
    bs_status status = BS_OK;
    controller->timer = 0;

    if (phase == BS_PHASE_COMPUTER_TURN) {
        bs_turn_report report;
        status = bs_game_computer_fire(controller->model, &report);
        if (status == BS_OK) {
            after_shot(controller, &report);
        }
    } else if (phase == BS_PHASE_HUMAN_TURN && controller->options.autoplay) {
        bs_coord target;
        status = bs_game_suggest(controller->model, BS_DIFFICULTY_HARD, &target);
        if (status == BS_OK) {
            bs_view_click_cell(controller->view, target);
        }
    }

    if (status != BS_OK) {
        g_warning("the computer could not play: %s", bs_status_str(status));
    }
    return G_SOURCE_REMOVE;
}

static void schedule_tick(bs_controller *controller)
{
    cancel_timer(controller);
    controller->timer = g_timeout_add(
        controller->options.autoplay ? AUTOPLAY_DELAY_MS : TURN_DELAY_MS, on_tick, controller);
}

/* ------------------------------------------------------------ game control */

static void start_game(bs_controller *controller, bs_difficulty level)
{
    cancel_timer(controller);

    const bool use_given_seed = controller->first_game && controller->options.has_seed;
    const uint64_t seed =
        use_given_seed ? controller->options.seed : bs_rng_next(&controller->seeds);
    controller->first_game = false;

    const bs_status status = bs_game_init(controller->model, level, seed);
    if (status != BS_OK) {
        g_warning("cannot start a game: %s", bs_status_str(status));
        return;
    }
    controller->in_game = true;
    controller->autoplay_shots = 0;

    synchronise(controller);
    bs_view_show_page(controller->view, BS_VIEW_PAGE_GAME);
    if (controller->options.autoplay) {
        schedule_tick(controller);
    }
}

static void back_to_menu(bs_controller *controller)
{
    cancel_timer(controller);
    controller->in_game = false;
    bs_view_show_page(controller->view, BS_VIEW_PAGE_MENU);
}

/* ------------------------------------------------- actions (view -> model) */

static void on_level_chosen(bs_difficulty level, gpointer user_data)
{
    start_game(user_data, level);
}

static void on_cell_activated(bs_coord cell, gpointer user_data)
{
    bs_controller *controller = user_data;
    bs_turn_report report;

    if (!controller->in_game) {
        return;
    }
    /* The model is the judge: a shot out of turn or at a used cell is refused. */
    if (bs_game_human_fire(controller->model, cell, &report) == BS_OK) {
        after_shot(controller, &report);
    }
}

static void on_reshuffle_requested(gpointer user_data)
{
    bs_controller *controller = user_data;
    if (controller->in_game && bs_game_reshuffle(controller->model) == BS_OK) {
        synchronise(controller);
    }
}

static void on_restart_requested(gpointer user_data)
{
    back_to_menu(user_data);
}

static void on_quit_requested(gpointer user_data)
{
    bs_controller *controller = user_data;
    g_application_quit(G_APPLICATION(controller->application));
}

static void on_game_over_answered(bs_view_choice choice, gpointer user_data)
{
    bs_controller *controller = user_data;
    switch (choice) {
    case BS_CHOICE_REPLAY:
        start_game(controller, bs_game_difficulty(controller->model));
        break;
    case BS_CHOICE_MENU:
        back_to_menu(controller);
        break;
    case BS_CHOICE_QUIT:
        g_application_quit(G_APPLICATION(controller->application));
        break;
    }
}

/* The window was closed by the user: stop touching the view. */
static void on_closed(gpointer user_data)
{
    bs_controller *controller = user_data;
    cancel_timer(controller);
    controller->in_game = false;
}

/* ------------------------------------------------------------------ public */

bs_controller *bs_controller_new(GtkApplication *application, bs_game *model,
                                 const bs_controller_options *options)
{
    static const bs_view_actions actions = {
        .level_chosen = on_level_chosen,
        .cell_activated = on_cell_activated,
        .reshuffle_requested = on_reshuffle_requested,
        .restart_requested = on_restart_requested,
        .quit_requested = on_quit_requested,
        .game_over_answered = on_game_over_answered,
        .closed = on_closed,
    };

    bs_controller *controller = g_new0(bs_controller, 1);
    controller->application = application;
    controller->model = model;
    controller->first_game = true;
    if (options != NULL) {
        controller->options = *options;
        controller->screenshot_path = g_strdup(options->screenshot_path);
        controller->options.screenshot_path = controller->screenshot_path;
    }

    /* Without a seed, start from the clock mixed with GLib's own entropy. */
    const uint64_t entropy = (uint64_t)g_get_real_time() ^ ((uint64_t)g_random_int() << 32);
    bs_rng_seed(&controller->seeds,
                controller->options.has_seed ? controller->options.seed : entropy);

    controller->audio = controller->options.mute ? NULL : bs_audio_new();
    controller->view = bs_view_new(application, &actions, controller);

    if (controller->options.has_level) {
        start_game(controller, controller->options.level);
    } else if (controller->options.autoplay) {
        start_game(controller, BS_DIFFICULTY_MEDIUM);
    }
    /* A screenshot without autoplay captures the first screen and quits. */
    if (controller->screenshot_path != NULL && !controller->options.autoplay) {
        capture_and_quit_soon(controller);
    }
    return controller;
}

void bs_controller_free(bs_controller *controller)
{
    if (controller == NULL) {
        return;
    }
    cancel_timer(controller);
    bs_view_free(controller->view);
    bs_audio_free(controller->audio);
    g_free(controller->screenshot_path);
    g_free(controller);
}

void bs_controller_present(bs_controller *controller)
{
    if (controller != NULL) {
        bs_view_present(controller->view);
    }
}

int bs_controller_exit_status(const bs_controller *controller)
{
    return controller == NULL ? 0 : controller->exit_status;
}
