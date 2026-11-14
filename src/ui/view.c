/* SPDX-License-Identifier: MIT */
#include "view.h"

#include "resources.h"
#include "texts.h"

/* Size of a board cell and width of the central illustration, in pixels. */
#define CELL_SIZE 44
#define VERSUS_WIDTH 280

/* Index of the two information windows in bs_view::info_windows. */
enum {
    INFO_RULES = 0,
    INFO_TRAILER,
    INFO_COUNT
};

/*
 * Data attached to one button so that its callback knows what it stands for.
 * Same idea as the "controller of a box" of the course: a back-pointer to the
 * parent structure plus the identity of the widget.
 */
typedef struct cell_context {
    bs_view *view;
    bs_coord coord;
} cell_context;

typedef struct level_context {
    bs_view *view;
    bs_difficulty level;
} level_context;

struct bs_view {
    bs_view_actions actions;
    gpointer user_data;

    GtkWidget *window; /* NULL once the window has been destroyed */
    GtkWidget *stack;
    GtkWidget *info_windows[INFO_COUNT]; /* NULL while closed */
    GtkWidget *game_over_dialog;         /* NULL unless the question is open */

    GtkWidget *cells[BS_PLAYER_COUNT][BS_CELL_COUNT];
    bs_cell_look looks[BS_PLAYER_COUNT][BS_CELL_COUNT];
    GtkWidget *level_chips[BS_DIFFICULTY_COUNT];
    GtkWidget *score_labels[BS_PLAYER_COUNT]; /* indexed by shooter */
    GtkWidget *status_label;
    GtkWidget *reshuffle_button;

    cell_context cell_contexts[BS_CELL_COUNT];
    level_context level_contexts[BS_DIFFICULTY_COUNT];
};

/* CSS class of each bs_cell_look, in enum order. */
static const char *const LOOK_CLASSES[] = {"water", "ship", "miss", "hit", "sunk", "excluded"};

static const char *const LEVEL_LABELS[BS_DIFFICULTY_COUNT] = {
    BS_TEXT_LEVEL_EASY, BS_TEXT_LEVEL_MEDIUM, BS_TEXT_LEVEL_HARD};

/* --------------------------------------------------------- small helpers */

static void add_class(GtkWidget *widget, const char *class_name)
{
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), class_name);
}

static void set_class(GtkWidget *widget, const char *class_name, bool enabled)
{
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    if (enabled) {
        gtk_style_context_add_class(context, class_name);
    } else {
        gtk_style_context_remove_class(context, class_name);
    }
}

static GtkWidget *label_new(const char *text, const char *class_name)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    add_class(label, class_name);
    return label;
}

static GtkWidget *button_new(const char *text, const char *class_name)
{
    GtkWidget *button = gtk_button_new_with_label(text);
    add_class(button, class_name);
    return button;
}

/* Installs the application style sheet, once for the whole process. */
static void load_style_sheet(void)
{
    static bool loaded = false;
    if (loaded) {
        return;
    }
    loaded = true;

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, BS_RESOURCE_PREFIX "/style.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

/* ------------------------------------------------------------- callbacks */
/*
 * Signals are connected with g_signal_connect_swapped(): the callback gets
 * the data pointer first and can ignore the widget, as recommended in the
 * course for callbacks that only need their context.
 */

static void on_level_clicked(level_context *context)
{
    const bs_view *view = context->view;
    if (view->actions.level_chosen != NULL) {
        view->actions.level_chosen(context->level, view->user_data);
    }
}

static void on_cell_clicked(cell_context *context)
{
    const bs_view *view = context->view;
    if (view->actions.cell_activated != NULL) {
        view->actions.cell_activated(context->coord, view->user_data);
    }
}

static void on_reshuffle_clicked(bs_view *view)
{
    if (view->actions.reshuffle_requested != NULL) {
        view->actions.reshuffle_requested(view->user_data);
    }
}

static void on_restart_clicked(bs_view *view)
{
    if (view->actions.restart_requested != NULL) {
        view->actions.restart_requested(view->user_data);
    }
}

static void on_quit_clicked(bs_view *view)
{
    if (view->actions.quit_requested != NULL) {
        view->actions.quit_requested(view->user_data);
    }
}

static void on_window_destroyed(bs_view *view)
{
    view->window = NULL;
    if (view->actions.closed != NULL) {
        view->actions.closed(view->user_data);
    }
}

static void on_game_over_response(GtkDialog *dialog, gint response, bs_view *view)
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
    /* Negative ids are GTK's own (dialog closed): the finished board stays. */
    if (response >= 0 && view->actions.game_over_answered != NULL) {
        view->actions.game_over_answered((bs_view_choice)response, view->user_data);
    }
}

/* F11 toggles full screen, the way the original version always ran. */
static gboolean on_key_pressed(GtkWidget *window, GdkEventKey *event, gpointer user_data)
{
    (void)user_data;
    if (event->keyval != GDK_KEY_F11) {
        return GDK_EVENT_PROPAGATE;
    }
    GdkWindow *gdk_window = gtk_widget_get_window(window);
    if (gdk_window != NULL &&
        (gdk_window_get_state(gdk_window) & GDK_WINDOW_STATE_FULLSCREEN) != 0) {
        gtk_window_unfullscreen(GTK_WINDOW(window));
    } else {
        gtk_window_fullscreen(GTK_WINDOW(window));
    }
    return GDK_EVENT_STOP;
}

/* ----------------------------------------------------- information windows */

/* Opens the rules or the trailer; a second click raises the open window. */
static void show_info_window(bs_view *view, int which)
{
    static const char *const rules[] = BS_TEXT_RULES_LINES;
    static const char *const trailer[] = BS_TEXT_TRAILER_LINES;
    const char *const *lines = which == INFO_RULES ? rules : trailer;
    const gsize count = which == INFO_RULES ? G_N_ELEMENTS(rules) : G_N_ELEMENTS(trailer);
    const char *title = which == INFO_RULES ? BS_TEXT_RULES_TITLE : BS_TEXT_TRAILER_TITLE;

    if (view->window == NULL) {
        return;
    }
    if (view->info_windows[which] != NULL) {
        gtk_window_present(GTK_WINDOW(view->info_windows[which]));
        return;
    }

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(view->window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_default_size(GTK_WINDOW(window), 980, 620);
    add_class(window, which == INFO_RULES ? "info-rules" : "info-trailer");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    add_class(box, "info-box");
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_box_pack_start(GTK_BOX(box), label_new(title, "info-title"), FALSE, FALSE, 10);

    for (gsize i = 0; i < count; i++) {
        GtkWidget *label = label_new(lines[i], "info-text");
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 96);
        gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    }

    /* Forget the window when it closes, so that it can be opened again. */
    view->info_windows[which] = window;
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroyed),
                     (gpointer)&view->info_windows[which]);
    gtk_widget_show_all(window);
}

static void on_rules_clicked(bs_view *view)
{
    show_info_window(view, INFO_RULES);
}

static void on_trailer_clicked(bs_view *view)
{
    show_info_window(view, INFO_TRAILER);
}

/* Row with the two buttons that open the information windows. */
static GtkWidget *build_info_bar(bs_view *view)
{
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    GtkWidget *rules = button_new(BS_TEXT_RULES_BUTTON, "info-button");
    GtkWidget *trailer = button_new(BS_TEXT_TRAILER_BUTTON, "info-button");

    g_signal_connect_swapped(rules, "clicked", G_CALLBACK(on_rules_clicked), view);
    g_signal_connect_swapped(trailer, "clicked", G_CALLBACK(on_trailer_clicked), view);
    gtk_box_set_homogeneous(GTK_BOX(bar), TRUE);
    gtk_box_pack_start(GTK_BOX(bar), rules, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(bar), trailer, TRUE, TRUE, 0);
    return bar;
}

/* ---------------------------------------------------------------- menu page */

static GtkWidget *build_menu_page(bs_view *view)
{
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    add_class(page, "page-menu");

    gtk_box_pack_start(GTK_BOX(page), build_info_bar(view), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(page), label_new(BS_TEXT_MENU_TITLE, "menu-title"), TRUE, TRUE, 0);

    GtkWidget *levels = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 40);
    gtk_box_set_homogeneous(GTK_BOX(levels), TRUE);
    gtk_widget_set_halign(levels, GTK_ALIGN_CENTER);
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        GtkWidget *button = button_new(LEVEL_LABELS[level], "level-button");
        view->level_contexts[level].view = view;
        view->level_contexts[level].level = (bs_difficulty)level;
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_level_clicked),
                                 &view->level_contexts[level]);
        gtk_box_pack_start(GTK_BOX(levels), button, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(page), levels, TRUE, FALSE, 0);
    return page;
}

/* ---------------------------------------------------------------- game page */

/*
 * A 10x10 grid of buttons framed by its coordinates (A-J, 1-10).
 * Only the enemy board is interactive.
 */
static GtkWidget *build_board(bs_view *view, bs_player owner)
{
    const bool interactive = owner == BS_PLAYER_COMPUTER;
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    add_class(grid, "board");

    for (int i = 0; i < BS_BOARD_SIZE; i++) {
        const char column[] = {(char)('A' + i), '\0'};
        char *row = g_strdup_printf("%d", i + 1);
        gtk_grid_attach(GTK_GRID(grid), label_new(column, "board-header"), i + 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), label_new(row, "board-header"), 0, i + 1, 1, 1);
        g_free(row);
    }

    for (int index = 0; index < BS_CELL_COUNT; index++) {
        const bs_coord coord = bs_coord_from_index(index);
        GtkWidget *button = gtk_button_new();
        gtk_widget_set_size_request(button, CELL_SIZE, CELL_SIZE);
        add_class(button, "cell");
        add_class(button, interactive ? "enemy" : "own");
        add_class(button, LOOK_CLASSES[BS_LOOK_WATER]);

        if (interactive) {
            char *name = g_strdup_printf("%c%d", 'A' + coord.col, coord.row + 1);
            atk_object_set_name(gtk_widget_get_accessible(button), name);
            g_free(name);

            view->cell_contexts[index].view = view;
            view->cell_contexts[index].coord = coord;
            g_signal_connect_swapped(button, "clicked", G_CALLBACK(on_cell_clicked),
                                     &view->cell_contexts[index]);
        } else {
            gtk_widget_set_sensitive(button, FALSE);
        }

        gtk_grid_attach(GTK_GRID(grid), button, coord.col + 1, coord.row + 1, 1, 1);
        view->cells[owner][index] = button;
        view->looks[owner][index] = BS_LOOK_WATER;
    }
    return grid;
}

/* A board with its title above and the score of its attacker below. */
static GtkWidget *build_fleet_column(bs_view *view, bs_player owner)
{
    const bool own = owner == BS_PLAYER_HUMAN;
    const bs_player attacker = bs_player_other(owner);
    GtkWidget *column = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    view->score_labels[attacker] = label_new("", "score");
    gtk_box_pack_start(GTK_BOX(column),
                       label_new(own ? BS_TEXT_FLEET_PLAYER : BS_TEXT_FLEET_ENEMY,
                                 own ? "fleet-title-own" : "fleet-title-enemy"),
                       FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(column), build_board(view, owner), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(column), view->score_labels[attacker], FALSE, FALSE, 0);
    return column;
}

static GtkWidget *build_versus_image(void)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_resource_at_scale(
        BS_RESOURCE_PREFIX "/images/versus.png", VERSUS_WIDTH, -1, TRUE, &error);
    if (pixbuf == NULL) {
        g_warning("cannot load the illustration: %s", error->message);
        g_clear_error(&error);
        return gtk_label_new(NULL);
    }
    GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    add_class(image, "versus");
    return image;
}

/* Bottom-left: which level is being played. */
static GtkWidget *build_level_box(bs_view *view)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start(GTK_BOX(box), label_new(BS_TEXT_LEVEL_HEADING, "heading"), FALSE, FALSE, 0);
    for (int level = 0; level < BS_DIFFICULTY_COUNT; level++) {
        view->level_chips[level] = label_new(LEVEL_LABELS[level], "level-chip");
        gtk_box_pack_start(GTK_BOX(box), view->level_chips[level], FALSE, FALSE, 0);
    }
    return box;
}

/* Bottom-right: restart, replace the fleet, quit. */
static GtkWidget *build_control_box(bs_view *view)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *restart = button_new(BS_TEXT_RESTART_BUTTON, "control-button");
    GtkWidget *quit = button_new(BS_TEXT_QUIT_BUTTON, "control-button");
    view->reshuffle_button = button_new(BS_TEXT_RESHUFFLE_BUTTON, "control-button");

    g_signal_connect_swapped(restart, "clicked", G_CALLBACK(on_restart_clicked), view);
    g_signal_connect_swapped(view->reshuffle_button, "clicked", G_CALLBACK(on_reshuffle_clicked),
                             view);
    g_signal_connect_swapped(quit, "clicked", G_CALLBACK(on_quit_clicked), view);

    gtk_box_pack_start(GTK_BOX(box), label_new(BS_TEXT_CONTROL_HEADING, "heading"), FALSE, FALSE,
                       0);
    gtk_box_pack_start(GTK_BOX(box), restart, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), view->reshuffle_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), quit, FALSE, FALSE, 0);
    return box;
}

static GtkWidget *build_game_page(bs_view *view)
{
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    add_class(page, "page-game");

    GtkWidget *boards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 24);
    gtk_box_pack_start(GTK_BOX(boards), build_fleet_column(view, BS_PLAYER_HUMAN), TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(boards), build_versus_image(), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(boards), build_fleet_column(view, BS_PLAYER_COMPUTER), TRUE, FALSE,
                       0);

    GtkWidget *footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 24);
    view->status_label = label_new("", "status");
    gtk_label_set_line_wrap(GTK_LABEL(view->status_label), TRUE);
    gtk_box_pack_start(GTK_BOX(footer), build_level_box(view), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(footer), view->status_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(footer), build_control_box(view), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(page), build_info_bar(view), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(page), boards, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(page), footer, FALSE, FALSE, 0);
    return page;
}

/* ------------------------------------------------------------------ public */

bs_view *bs_view_new(GtkApplication *application, const bs_view_actions *actions,
                     gpointer user_data)
{
    load_style_sheet();

    bs_view *view = g_new0(bs_view, 1);
    if (actions != NULL) {
        view->actions = *actions;
    }
    view->user_data = user_data;

    view->window = gtk_application_window_new(application);
    gtk_window_set_title(GTK_WINDOW(view->window), BS_TEXT_WINDOW_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(view->window), 1320, 840);
    gtk_window_set_position(GTK_WINDOW(view->window), GTK_WIN_POS_CENTER);
    g_signal_connect_swapped(view->window, "destroy", G_CALLBACK(on_window_destroyed), view);
    g_signal_connect(view->window, "key-press-event", G_CALLBACK(on_key_pressed), NULL);

    view->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(view->stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_add_named(GTK_STACK(view->stack), build_menu_page(view), "menu");
    gtk_stack_add_named(GTK_STACK(view->stack), build_game_page(view), "game");
    gtk_container_add(GTK_CONTAINER(view->window), view->stack);

    gtk_widget_show_all(view->window);
    return view;
}

void bs_view_free(bs_view *view)
{
    if (view == NULL) {
        return;
    }
    for (int i = 0; i < INFO_COUNT; i++) {
        if (view->info_windows[i] != NULL) {
            gtk_widget_destroy(view->info_windows[i]);
        }
    }
    if (view->game_over_dialog != NULL) {
        gtk_widget_destroy(view->game_over_dialog);
    }
    if (view->window != NULL) {
        /* The owner is tearing the view down: no need to tell it. */
        view->actions.closed = NULL;
        gtk_widget_destroy(view->window);
    }
    g_free(view);
}

void bs_view_present(bs_view *view)
{
    if (view != NULL && view->window != NULL) {
        gtk_window_present(GTK_WINDOW(view->window));
    }
}

void bs_view_show_page(bs_view *view, bs_view_page page)
{
    if (view == NULL || view->window == NULL) {
        return;
    }
    gtk_stack_set_visible_child_name(GTK_STACK(view->stack),
                                     page == BS_VIEW_PAGE_MENU ? "menu" : "game");
}

void bs_view_set_cell(bs_view *view, bs_player owner, bs_coord cell, bs_cell_look look,
                      bool sensitive)
{
    if (view == NULL || view->window == NULL || !bs_coord_is_valid(cell) ||
        (owner != BS_PLAYER_HUMAN && owner != BS_PLAYER_COMPUTER) ||
        (gsize)look >= G_N_ELEMENTS(LOOK_CLASSES)) {
        return;
    }

    const int index = bs_coord_index(cell);
    GtkWidget *button = view->cells[owner][index];
    const bs_cell_look previous = view->looks[owner][index];

    if (previous != look) {
        GtkStyleContext *context = gtk_widget_get_style_context(button);
        gtk_style_context_remove_class(context, LOOK_CLASSES[previous]);
        gtk_style_context_add_class(context, LOOK_CLASSES[look]);
        view->looks[owner][index] = look;
    }
    gtk_widget_set_sensitive(button, sensitive);
}

void bs_view_set_level(bs_view *view, bs_difficulty level)
{
    if (view == NULL || view->window == NULL) {
        return;
    }
    for (int i = 0; i < BS_DIFFICULTY_COUNT; i++) {
        set_class(view->level_chips[i], "active", i == (int)level);
    }
}

void bs_view_set_status(bs_view *view, const char *text, bs_view_tone tone)
{
    if (view == NULL || view->window == NULL) {
        return;
    }
    gtk_label_set_text(GTK_LABEL(view->status_label), text != NULL ? text : "");
    set_class(view->status_label, "victory", tone == BS_TONE_VICTORY);
    set_class(view->status_label, "defeat", tone == BS_TONE_DEFEAT);
}

void bs_view_set_score(bs_view *view, bs_player shooter, unsigned shots, unsigned hits,
                       unsigned ships_sunk)
{
    if (view == NULL || view->window == NULL ||
        (shooter != BS_PLAYER_HUMAN && shooter != BS_PLAYER_COMPUTER)) {
        return;
    }
    char *text = g_strdup_printf(BS_TEXT_SCORE_FORMAT, shots, hits, ships_sunk, BS_FLEET_SIZE);
    gtk_label_set_text(GTK_LABEL(view->score_labels[shooter]), text);
    g_free(text);
}

void bs_view_set_reshuffle_enabled(bs_view *view, bool enabled)
{
    if (view == NULL || view->window == NULL) {
        return;
    }
    gtk_widget_set_sensitive(view->reshuffle_button, enabled);
}

void bs_view_ask_game_over(bs_view *view, bool victory)
{
    if (view == NULL || view->window == NULL || view->game_over_dialog != NULL) {
        return;
    }
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(view->window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, "%s",
        victory ? BS_TEXT_VICTORY_TITLE : BS_TEXT_DEFEAT_TITLE);
    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog), "%s", victory ? BS_TEXT_VICTORY_DIALOG : BS_TEXT_DEFEAT_DIALOG);
    gtk_dialog_add_buttons(GTK_DIALOG(dialog), BS_TEXT_LEAVE_BUTTON, BS_CHOICE_QUIT,
                           BS_TEXT_MENU_BUTTON, BS_CHOICE_MENU, BS_TEXT_REPLAY_BUTTON,
                           BS_CHOICE_REPLAY, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), BS_CHOICE_REPLAY);
    g_signal_connect(dialog, "response", G_CALLBACK(on_game_over_response), view);
    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_destroyed),
                     (gpointer)&view->game_over_dialog);
    view->game_over_dialog = dialog;
    gtk_widget_show_all(dialog);
}

void bs_view_click_cell(bs_view *view, bs_coord cell)
{
    if (view == NULL || view->window == NULL || !bs_coord_is_valid(cell)) {
        return;
    }
    GtkWidget *button = view->cells[BS_PLAYER_COMPUTER][bs_coord_index(cell)];
    if (gtk_widget_is_sensitive(button)) {
        gtk_button_clicked(GTK_BUTTON(button));
    }
}

bool bs_view_answer_game_over(bs_view *view, bs_view_choice choice)
{
    if (view == NULL || view->game_over_dialog == NULL) {
        return false;
    }
    gtk_dialog_response(GTK_DIALOG(view->game_over_dialog), (gint)choice);
    return true;
}

bool bs_view_save_png(bs_view *view, const char *path, GError **error)
{
    if (view == NULL || view->window == NULL || path == NULL) {
        g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, "no window to capture");
        return false;
    }

    const int width = gtk_widget_get_allocated_width(view->window);
    const int height = gtk_widget_get_allocated_height(view->window);
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cairo_t *cairo = cairo_create(surface);

    /* Ask GTK to paint the window into our surface instead of the screen. */
    gtk_widget_draw(view->window, cairo);
    const cairo_status_t status = cairo_surface_write_to_png(surface, path);

    cairo_destroy(cairo);
    cairo_surface_destroy(surface);

    if (status != CAIRO_STATUS_SUCCESS) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "cannot write '%s': %s", path,
                    cairo_status_to_string(status));
        return false;
    }
    return true;
}
