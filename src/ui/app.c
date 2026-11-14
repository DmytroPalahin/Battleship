/* SPDX-License-Identifier: MIT */
#include "app.h"

#include "controller.h"
#include "resources.h"

#include "battleship/battleship.h"

#include <gtk/gtk.h>
#include <string.h>

/* GLib renamed the "no flags" value in 2.74 and deprecated the old name. */
#if GLIB_CHECK_VERSION(2, 74, 0)
#define APP_FLAGS (G_APPLICATION_DEFAULT_FLAGS | G_APPLICATION_NON_UNIQUE)
#else
#define APP_FLAGS (G_APPLICATION_FLAGS_NONE | G_APPLICATION_NON_UNIQUE)
#endif

/*
 * Everything that lives as long as the process. The model is declared here,
 * outside of the controller: it could be saved, replayed or handed to another
 * interface without the graphical one knowing.
 */
typedef struct app_state {
    bs_game model;
    bs_controller *controller;
    bs_controller_options options;
    char *screenshot_path;
    int exit_status;
} app_state;

static const GOptionEntry OPTIONS[] = {
    {"version", 'V', 0, G_OPTION_ARG_NONE, NULL, "Show the version and exit", NULL},
    {"level", 'l', 0, G_OPTION_ARG_STRING, NULL,
     "Start at a level without showing the menu: easy, medium or hard", "LEVEL"},
    {"seed", 's', 0, G_OPTION_ARG_INT64, NULL, "Seed of the first game, to replay it", "N"},
    {"mute", 'm', 0, G_OPTION_ARG_NONE, NULL, "Disable the sound effects", NULL},
    {"autoplay", 0, 0, G_OPTION_ARG_NONE, NULL,
     "Testing: let the computer play both sides, then quit", NULL},
    {"max-shots", 0, 0, G_OPTION_ARG_INT, NULL, "Testing: with --autoplay, stop after N shots",
     "N"},
    {"screenshot", 0, 0, G_OPTION_ARG_FILENAME, NULL,
     "Testing: save the window as a PNG file, then quit", "FILE"},
    {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL},
};

/*
 * Validates the command line before any window exists.
 * Returns -1 to carry on, or the exit status to stop with.
 */
static gint on_handle_local_options(GApplication *application, GVariantDict *dict, app_state *state)
{
    const char *level = NULL;
    const char *screenshot = NULL;
    gint64 seed = 0;
    gint32 max_shots = 0;
    (void)application;

    if (g_variant_dict_contains(dict, "version")) {
        g_print("battleship %s\n", bs_version());
        return 0;
    }

    if (g_variant_dict_lookup(dict, "level", "&s", &level)) {
        if (!bs_difficulty_parse(level, &state->options.level)) {
            g_printerr("battleship: unknown level '%s' (expected easy, medium or hard)\n", level);
            return 2;
        }
        state->options.has_level = true;
    }
    if (g_variant_dict_lookup(dict, "seed", "x", &seed)) {
        state->options.seed = (uint64_t)seed;
        state->options.has_seed = true;
    }
    if (g_variant_dict_lookup(dict, "max-shots", "i", &max_shots)) {
        if (max_shots < 0) {
            g_printerr("battleship: --max-shots must not be negative\n");
            return 2;
        }
        state->options.max_shots = max_shots;
    }
    if (g_variant_dict_lookup(dict, "screenshot", "^&ay", &screenshot)) {
        state->screenshot_path = g_strdup(screenshot);
        state->options.screenshot_path = state->screenshot_path;
    }
    state->options.mute = g_variant_dict_contains(dict, "mute");
    state->options.autoplay = g_variant_dict_contains(dict, "autoplay");
    return -1;
}

static void on_activate(GtkApplication *application, app_state *state)
{
    if (state->controller != NULL) {
        bs_controller_present(state->controller);
        return;
    }
    state->controller = bs_controller_new(application, &state->model, &state->options);
}

static void on_shutdown(GApplication *application, app_state *state)
{
    (void)application;
    state->exit_status = bs_controller_exit_status(state->controller);
    bs_controller_free(state->controller);
    state->controller = NULL;
}

int bs_app_run(int argc, char **argv)
{
    app_state state;
    memset(&state, 0, sizeof state);

    GtkApplication *application = gtk_application_new(BS_APPLICATION_ID, APP_FLAGS);
    g_application_add_main_option_entries(G_APPLICATION(application), OPTIONS);
    g_application_set_option_context_summary(
        G_APPLICATION(application),
        "Battleship (Bataille Navale): sink the pirate fleet before it sinks yours.");

    g_signal_connect(application, "handle-local-options", G_CALLBACK(on_handle_local_options),
                     &state);
    g_signal_connect(application, "activate", G_CALLBACK(on_activate), &state);
    g_signal_connect(application, "shutdown", G_CALLBACK(on_shutdown), &state);

    const int status = g_application_run(G_APPLICATION(application), argc, argv);

    g_object_unref(application);
    g_free(state.screenshot_path);
    return status != 0 ? status : state.exit_status;
}
