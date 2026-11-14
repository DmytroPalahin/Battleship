/* SPDX-License-Identifier: MIT */
/**
 * @file view.h
 * @brief The view: every GTK widget of the application, and nothing else.
 *
 * The view knows how to draw and how to listen. It does not know the rules:
 * it never includes the game engine's turn logic and never decides anything.
 *
 *  - Output: the controller describes what to show with the setters below
 *    (a look for each cell, a status line, a score...).
 *  - Input: user gestures are translated into the semantic callbacks of
 *    bs_view_actions ("the player chose a level", "the player fired at C4").
 *
 * As a consequence no GtkWidget ever crosses this interface, and the
 * controller could drive a completely different toolkit unchanged.
 */
#ifndef BATTLESHIP_UI_VIEW_H
#define BATTLESHIP_UI_VIEW_H

#include "battleship/ai.h"
#include "battleship/types.h"

#include <gtk/gtk.h>
#include <stdbool.h>

/** The two screens of the main window. */
typedef enum bs_view_page {
    BS_VIEW_PAGE_MENU = 0, /**< Choice of the difficulty level. */
    BS_VIEW_PAGE_GAME      /**< The two boards. */
} bs_view_page;

/** What a board cell looks like. */
typedef enum bs_cell_look {
    BS_LOOK_WATER = 0, /**< Nothing to show. */
    BS_LOOK_SHIP,      /**< An intact deck. */
    BS_LOOK_MISS,      /**< A shot in the water. */
    BS_LOOK_HIT,       /**< A burning deck. */
    BS_LOOK_SUNK,      /**< A deck of a sunk ship. */
    BS_LOOK_EXCLUDED   /**< A flag: no ship can be here. */
} bs_cell_look;

/** Mood of the status line. */
typedef enum bs_view_tone {
    BS_TONE_NEUTRAL = 0,
    BS_TONE_VICTORY,
    BS_TONE_DEFEAT
} bs_view_tone;

/** What the player answered when a game ended. */
typedef enum bs_view_choice {
    BS_CHOICE_REPLAY = 0, /**< Another game at the same level. */
    BS_CHOICE_MENU,       /**< Back to the choice of the level. */
    BS_CHOICE_QUIT        /**< Leave the application. */
} bs_view_choice;

/**
 * Callbacks through which the view reports what the user did. All of them
 * receive the user_data pointer given to bs_view_new(). Entries may be NULL.
 */
typedef struct bs_view_actions {
    /** A level was chosen on the menu page. */
    void (*level_chosen)(bs_difficulty level, gpointer user_data);
    /** A cell of the enemy board was activated. */
    void (*cell_activated)(bs_coord cell, gpointer user_data);
    /** "Replace the fleet" was pressed. */
    void (*reshuffle_requested)(gpointer user_data);
    /** "Restart" was pressed. */
    void (*restart_requested)(gpointer user_data);
    /** "Quit" was pressed. */
    void (*quit_requested)(gpointer user_data);
    /** The end-of-game question was answered. */
    void (*game_over_answered)(bs_view_choice choice, gpointer user_data);
    /** The main window is gone; the view must not be used any more. */
    void (*closed)(gpointer user_data);
} bs_view_actions;

/** The view. Opaque: use the functions below. */
typedef struct bs_view bs_view;

/**
 * Builds the main window and shows the menu page.
 * @param actions copied; may be NULL for a view nobody listens to.
 * @return a view to release with bs_view_free().
 */
bs_view *bs_view_new(GtkApplication *application, const bs_view_actions *actions,
                     gpointer user_data);

/** Destroys the window (if still open) and releases the view. NULL is fine. */
void bs_view_free(bs_view *view);

/** Brings the main window to the front. */
void bs_view_present(bs_view *view);

/** Switches between the menu and the game. */
void bs_view_show_page(bs_view *view, bs_view_page page);

/**
 * Updates one cell.
 * @param owner     whose board the cell belongs to.
 * @param sensitive whether the cell reacts to the user.
 */
void bs_view_set_cell(bs_view *view, bs_player owner, bs_coord cell, bs_cell_look look,
                      bool sensitive);

/** Highlights the level being played. */
void bs_view_set_level(bs_view *view, bs_difficulty level);

/** Sets the message of the status area. */
void bs_view_set_status(bs_view *view, const char *text, bs_view_tone tone);

/** Sets the score line under the board attacked by @p shooter. */
void bs_view_set_score(bs_view *view, bs_player shooter, unsigned shots, unsigned hits,
                       unsigned ships_sunk);

/** Enables or disables the "replace the fleet" button. */
void bs_view_set_reshuffle_enabled(bs_view *view, bool enabled);

/**
 * Asks the player what to do after a game. Does not block: the answer comes
 * through bs_view_actions::game_over_answered.
 */
void bs_view_ask_game_over(bs_view *view, bool victory);

/* ---------------------------------------------------------- test support */
/*
 * The three functions below let a program stand in for the user. The
 * automated end-to-end test plays a whole game through them, so that a shot
 * travels the same path as a real click: widget -> view -> controller -> model.
 */

/** Clicks a cell of the enemy board, exactly as the mouse would. */
void bs_view_click_cell(bs_view *view, bs_coord cell);

/**
 * Answers the end-of-game question, exactly as a click on its button would.
 * @return false when no question is being asked.
 */
bool bs_view_answer_game_over(bs_view *view, bs_view_choice choice);

/**
 * Renders the main window into a PNG file. Also used to produce the
 * screenshots of the documentation.
 * @return true on success; on failure @p error is set.
 */
bool bs_view_save_png(bs_view *view, const char *path, GError **error);

#endif /* BATTLESHIP_UI_VIEW_H */
