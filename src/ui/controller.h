/* SPDX-License-Identifier: MIT */
/**
 * @file controller.h
 * @brief The controller: the conductor between the model and the view.
 *
 * Following the organisation taught in the course:
 *  - the model exists on its own, before and after the interface, so the
 *    controller only receives a pointer to it;
 *  - the view has no reason to exist without a controller, so the controller
 *    builds it and destroys it.
 *
 * The controller reacts to what the user does (reported by the view), asks
 * the model to perform the move, then *synchronises* the view with the new
 * state of the model. It contains no rule of the game and no widget.
 */
#ifndef BATTLESHIP_UI_CONTROLLER_H
#define BATTLESHIP_UI_CONTROLLER_H

#include "battleship/game.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdint.h>

/** Start-up choices, normally taken from the command line. */
typedef struct bs_controller_options {
    bool has_level;      /**< Skip the menu and start at @ref level. */
    bs_difficulty level; /**< Level to start at when has_level is set. */
    bool has_seed;       /**< Use @ref seed for the first game. */
    uint64_t seed;       /**< Seed of the first game when has_seed is set. */
    bool mute;           /**< No sound effects. */

    /* Test and documentation hooks. */
    bool autoplay;               /**< The computer plays both sides, then quits. */
    int max_shots;               /**< With autoplay: stop after that many shots (0: no limit). */
    const char *screenshot_path; /**< PNG to write before quitting, or NULL. */
} bs_controller_options;

/** The controller. Opaque. */
typedef struct bs_controller bs_controller;

/**
 * Builds the view and connects it to the model.
 * @param model   a game owned by the caller; it must outlive the controller.
 * @param options copied; NULL means defaults.
 */
bs_controller *bs_controller_new(GtkApplication *application, bs_game *model,
                                 const bs_controller_options *options);

/** Destroys the view and releases the controller. NULL is fine. */
void bs_controller_free(bs_controller *controller);

/** Brings the window to the front. */
void bs_controller_present(bs_controller *controller);

/** Zero unless a test hook failed (for instance writing the screenshot). */
int bs_controller_exit_status(const bs_controller *controller);

#endif /* BATTLESHIP_UI_CONTROLLER_H */
