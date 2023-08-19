/****************************            "BATTLESHIP"  PALAHIN Dmytro et MAKAM Ivanna            ****************************/
#include <gtk/gtk.h>
#include <stdlib.h>				
#include <stdio.h>				
#include <string.h>				
#include <time.h>


/*******************************        Constant definitions        *******************************/ 
#define GRID_SIZE 10
#define NUM_DECKS 20
#define NUM_SHIPS 10
#define NUM_ATTEMPTS 1000000
/**************************************************************************************************/



/*******************************        Data structures        *******************************/
// Structure for storing the state of each cell on the playing field
typedef struct {
    GtkWidget *button;  // Button to display the cell
    gboolean is_ship;   // Flag indicating whether there is a ship in this box
    gboolean is_hit;    // Flag indicating whether a shot has been touched the ship
    gboolean is_shot;    // Flag indicating whether a shot has been done
} Cell;

typedef struct {
    int x;
    int y;
    int kx;
    int ky;
    int hits;
} TempShips;

static TempShips around_hit = {0, 0, 0, 0, 0} ;

static int coords4[4][2] = {0};



// Structure for storing game status
// Player1 = We
// Player2 = Computer
typedef struct {
    GtkWindow *window;       // Main window
    GtkWidget *grid_player1; // Grid for the first player's field of play
    GtkWidget *grid_player2; // Grid for the second player's field of play
    Cell player1_grid[GRID_SIZE][GRID_SIZE]; // An array of cells of the first player's playing field
    Cell player2_grid[GRID_SIZE][GRID_SIZE]; // An array of cells of the second player's playing field
    int player1_ships_left; // Number of ships remaining with the first player
    int player2_ships_left; // Number of ships remaining with the second player
    gboolean is_player1_turn; // Flag indicating whose turn it is, if "is_player1_turn" = 1, so it is our turn to play
   GtkWidget *button_easy;
   GtkWidget *button_average;
   GtkWidget *button_difficult;
   GtkWidget *button_replace_flot;
   GtkWidget *button_restart;
   GtkWidget *legend_attention;
} GameData;



typedef struct {
    int x;
    int y;
    int size;
    int horizontal;
    int size_leftover;
} Ship;

// The first and the second -> coordinates, at the beginning, they can be all "0"
// The third -> the size of deck of ship
// The fourth -> boolean like "0" or "1", at the beginning, they can be all "1", after function will randomly place ships
static Ship ships1[] = {
    {0, 0, 4, 1, 4},
    {0, 0, 3, 1, 3},
    {0, 0, 3, 1, 3},
    {0, 0, 2, 1, 2},
    {0, 0, 2, 1, 2},
    {0, 0, 2, 1, 2},
    {0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1}
};

static Ship ships2[] = {
    {0, 0, 4, 1, 4},
    {0, 0, 3, 1, 3},
    {0, 0, 3, 1, 3},
    {0, 0, 2, 1, 2},
    {0, 0, 2, 1, 2},
    {0, 0, 2, 1, 2},
    {0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1},
    {0, 0, 1, 1, 1}
};

static int grid1[GRID_SIZE][GRID_SIZE] = {0};
static int grid_ships1[GRID_SIZE][GRID_SIZE] = {0};
static int grid_ships2[GRID_SIZE][GRID_SIZE] = {0};
/******************************************************************************************************/





/************************************************************          DEFINITION FUNCTIONS          ************************************************************/


/**********************************************          The part with the missile          **********************************************/
void button_selected(GtkWidget *button) {
    GtkWidget *image = gtk_image_new_from_file("military_rocket_44.png");
    gtk_button_set_label(GTK_BUTTON(button), NULL);
    gtk_button_set_image(GTK_BUTTON(button), image);
}

void button_unselected(GtkWidget *button) {
    gtk_button_set_label(GTK_BUTTON(button), "");
    gtk_button_set_image(GTK_BUTTON(button), NULL);
}

gboolean button_key_press(GtkWidget *button, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down ||
        event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_Right) {
        button_unselected(button);
    }
    return FALSE;
}

gboolean button_key_release(GtkWidget *button, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down ||
        event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_Right) {
        button_selected(button);
    }
    return FALSE;
}
/**************************************************************************************************************************************/

void create_game_interface_start(GameData *data);

/**************************************************          FUNCTIONS FOR DIFFICULT LEVEL         **************************************************/

// Function for handling a click event on a playing field cell
void on_cell_clicked_difficult(GtkWidget *widget, gpointer user_data) {

    GameData *data = (GameData *)user_data;
    gtk_widget_set_sensitive(data->button_replace_flot, FALSE);

    // If "is_player1_turn" = 0  => not our turn, computer must play
    if (!data->is_player1_turn) {
        // Computer move, ignore player presses
        return;
    }
    while (data->is_player1_turn) { 

        Cell *cell = (Cell *)g_object_get_data(G_OBJECT(widget), "cell_data");
        // GtkWidget *widget = GTK_WIDGET(widget);
        int row = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell->button), "row"));
        int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell->button), "col"));
        data->player2_grid[row][col].is_shot = TRUE;


        if (cell->is_hit) {
            // The cell has already been shot up
            return;
        }

        if (cell->is_ship) {
            // The ship is hit
            cell->is_hit = TRUE;
            gtk_button_set_label(GTK_BUTTON(cell->button), NULL);
            GtkWidget *image2 = gtk_image_new_from_file("boom_44.png");
            gtk_button_set_image(GTK_BUTTON(cell->button), image2);
            // gtk_button_set_label(GTK_BUTTON(cell->button), "🔥");
            data->player2_ships_left--;
            data->is_player1_turn = TRUE;

            // Set Flags for Player1

            int index = grid_ships2[col][row];
            ships2[index - 1].size_leftover = ships2[index - 1].size_leftover - 1;
            // printf("index = %d\n", index);
            // printf("size_leftover = %d\n", ships2[index - 1].size_leftover);



            if ((row != 0) && (col != 0) && (data->player2_grid[row - 1][col - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row - 1][col - 1].button), "🚩");
                data->player2_grid[row - 1][col - 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row - 1][col - 1].button), FALSE);
            }

            if ((row != GRID_SIZE - 1) && (col != GRID_SIZE - 1) && (data->player2_grid[row + 1][col + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row + 1][col + 1].button), "🚩");
                data->player2_grid[row + 1][col + 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row + 1][col + 1].button), FALSE);
            }

            if ((row != 0) && (col != GRID_SIZE - 1) && (data->player2_grid[row - 1][col + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row - 1][col + 1].button), "🚩");
                data->player2_grid[row - 1][col + 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row - 1][col + 1].button), FALSE);
            }

            if ((row != GRID_SIZE - 1) && (col != 0) && (data->player2_grid[row + 1][col - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row + 1][col - 1].button), "🚩");
                data->player2_grid[row + 1][col - 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row + 1][col - 1].button), FALSE);
            }


            if (ships2[index - 1].size_leftover == 0) {
                int head_x = ships2[index - 1].y;
                int head_y = ships2[index - 1].x;

                int i, j;
                if (ships2[index - 1].horizontal == 1) {
                    j = head_y - 1;
                    for (i = head_x - 1; i <= head_x + ships2[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    j = head_y + 1;
                    for (i = head_x - 1; i <= head_x + ships2[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    if (head_x - 1 >= 0 && head_x - 1 < GRID_SIZE && data->player2_grid[head_x - 1][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x - 1][head_y].button), "🚩");
                        data->player2_grid[head_x - 1][head_y].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x - 1][head_y].button), FALSE);
                    }
                    if (head_x + ships2[index - 1].size >= 0 && head_x + ships2[index - 1].size < GRID_SIZE && data->player2_grid[head_x + ships1[index - 1].size][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x + ships1[index - 1].size][head_y].button), "🚩");
                        data->player2_grid[head_x + ships2[index - 1].size][head_y].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x + ships1[index - 1].size][head_y].button), FALSE);
                    }

                    // Set image
                    j = head_y;
                    for (i = head_x; i <= head_x + ships2[index - 1].size - 1; i++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_pirate2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player2_grid[i][j].button), image3);
                    }             
                } 
                else {
                    i = head_x - 1;
                    for (j = head_y - 1; j <= head_y + ships2[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    i = head_x + 1;
                    for (j = head_y - 1; j <= head_y + ships2[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    if (head_y - 1 >= 0 && head_y - 1 < GRID_SIZE && data->player2_grid[head_x][head_y - 1].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x][head_y - 1].button), "🚩");
                        data->player2_grid[head_x][head_y - 1].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x][head_y - 1].button), FALSE);
                    }
                    if (head_y + ships2[index - 1].size >= 0 && head_y + ships2[index - 1].size < GRID_SIZE && data->player2_grid[head_x][head_y + ships2[index - 1].size].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x][head_y + ships2[index - 1].size].button), "🚩");
                        data->player2_grid[head_x][head_y + ships2[index - 1].size].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x][head_y + ships2[index - 1].size].button), FALSE);
                    } 

                    // Set image
                    i = head_x;
                    for (j = head_y; j <= head_y + ships2[index - 1].size - 1; j++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_pirate2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player2_grid[i][j].button), image3);
                    }              
                } 

            }

        } 
        else {
            // Missing
            // GtkWidget *image2 = gtk_image_new_from_file("blunder_47.png");
            // gtk_button_set_image(GTK_BUTTON(cell->button), image2);
            gtk_button_set_label(GTK_BUTTON(cell->button), "🕳️");
            data->is_player1_turn = FALSE;
        }

        // If computer has 0 ships, so we win
        if (data->player2_ships_left == 0) {
            // The game is over, the player won
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Bravo ✌️🎉👍! Mission accomplie ✅, Vous avez protégé la science 👩‍🔬 contre des pirates ! Vous êtes un bon 🎖️ Commandant 🎖️ de la Sup Galilée Marine Nationale 🇫🇷. Continuez comme ça, Capitaine 🤝💪!");
            for (int i = 0; i < GRID_SIZE; i++) {
            	for (int j = 0; j < GRID_SIZE; j++) {
            		gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
            	}
            }
            
            gtk_label_set_text(GTK_LABEL(data->legend_attention), "🏅🏅   Cher Capitaine, gagner, c'est génial. \nLes guerres se gagnent par de petites victoires  ✌️✌️! \nMais nous savons tous les deux, \nque trop de jeux peuvent être dangereux pour la santé, \nnous Vous conseillons donc de faire une pause  🏁🏁. \nÀ bientôt !  🤗👋🤗");
			// Obtaining a label style context
			GtkStyleContext *style_context3 = gtk_widget_get_style_context(data->legend_attention);
			// Use CSS to change color
			const char *css3 = "label { color: #FBFF00; font-size: 23px; font-family: Calibri; }";
			GtkCssProvider *css_provider3 = gtk_css_provider_new();
			gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
			gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			g_object_unref(css_provider3);
			
			gtk_widget_set_sensitive(data->button_restart, FALSE);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_main_quit();
            return;
            
        }
        gtk_widget_set_sensitive(GTK_WIDGET(cell->button), FALSE);
    } 
    

/*********************************************         Computer stroke         *********************************************/

    while (!data->is_player1_turn) {

        // The computer takes the shot
        Cell *computer_cell;
        int flag_test = 0;
        int x, y;

        if (around_hit.hits == 0) { 
            while (flag_test == 0) {
                x = rand() % GRID_SIZE;
                y = rand() % GRID_SIZE;
                if (data->player1_grid[x][y].is_shot == FALSE) { 
                    computer_cell = &data->player1_grid[x][y];
                    flag_test++;
                    data->player1_grid[x][y].is_shot = TRUE; 
                }
            }
        }
        else {
            // computer_cell = &data->player1_grid[around_hit.x][around_hit.y];
            x = around_hit.x;
            y = around_hit.y;
            computer_cell = &data->player1_grid[x][y];
        }
        

        if (computer_cell->is_ship) {

            // Setting "clever" computer choice
            if (around_hit.hits != 0) {
                x = around_hit.x;
                y = around_hit.y;
                computer_cell = &data->player1_grid[x][y];
            }
            around_hit.hits = around_hit.hits + 1;


            // The ship is hit
            computer_cell->is_hit = TRUE;
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), NULL);
            GtkWidget *image = gtk_image_new_from_file("boom_44.png");
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), image);
            // gtk_button_set_label(GTK_BUTTON(computer_cell->button), "🔥");
            data->player1_ships_left = data->player1_ships_left - 1;
            data->is_player1_turn = FALSE;
            data->player1_grid[x][y].is_shot = TRUE;


            // Set Flags for Player1
            int index = grid_ships1[y][x];
            ships1[index - 1].size_leftover = ships1[index - 1].size_leftover - 1;


            // Mark not to use diagonal squares
            if ((x != 0) && (y != 0) && (data->player1_grid[x - 1][y - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player1_grid[x - 1][y - 1].button), "🚩");
                data->player1_grid[x - 1][y - 1].is_shot = TRUE;
            }

            if ((x != GRID_SIZE - 1) && (y != GRID_SIZE - 1) && (data->player1_grid[x + 1][y + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player1_grid[x + 1][y + 1].button), "🚩");
                data->player1_grid[x + 1][y + 1].is_shot = TRUE;
            }

            if ((x != 0) && (y != GRID_SIZE - 1) && (data->player1_grid[x - 1][y + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player1_grid[x - 1][y + 1].button), "🚩");
                data->player1_grid[x - 1][y + 1].is_shot = TRUE;
            }

            if ((x != GRID_SIZE - 1) && (y != 0) && (data->player1_grid[x + 1][y - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player1_grid[x + 1][y - 1].button), "🚩");
                data->player1_grid[x + 1][y - 1].is_shot = TRUE;
            }         

            // Forming an array of possible strokes
            coords4[0][0] = x - 1;
            coords4[0][1] = y;

            coords4[1][0] = x + 1;
            coords4[1][1] = y;
                        
            coords4[2][0] = x;
            coords4[2][1] = y - 1;

            coords4[3][0] = x;
            coords4[3][1] = y + 1;

            // Randomly select 1 out of 4
            int random = rand() % 4;

            if ((data->player1_grid[coords4[random][0]][coords4[random][1]].is_shot == FALSE) && 
                (coords4[random][0] != -1) && 
                (coords4[random][0] != GRID_SIZE) &&
                (coords4[random][1] != -1) && 
                (coords4[random][1] != GRID_SIZE)
                ) 
            { 
                    around_hit.x = coords4[random][0];
                    around_hit.y = coords4[random][1];
            }
            else {
                around_hit.hits = 0;
            }


            if (ships1[index - 1].size_leftover == 0) {
                int head_x = ships1[index - 1].y;
                int head_y = ships1[index - 1].x;

                int i, j;
                if (ships1[index - 1].horizontal == 1) {
                    j = head_y - 1;
                    for (i = head_x - 1; i <= head_x + ships1[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    j = head_y + 1;
                    for (i = head_x - 1; i <= head_x + ships1[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    if (head_x - 1 >= 0 && head_x - 1 < GRID_SIZE && data->player1_grid[head_x - 1][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x - 1][head_y].button), "🚩");
                        data->player1_grid[head_x - 1][head_y].is_shot = TRUE;
                    }
                    if (head_x + ships1[index - 1].size >= 0 && head_x + ships1[index - 1].size < GRID_SIZE && data->player1_grid[head_x + ships1[index - 1].size][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x + ships1[index - 1].size][head_y].button), "🚩");
                        data->player1_grid[head_x + ships1[index - 1].size][head_y].is_shot = TRUE;
                    } 

                    // Set image
                    j = head_y;
                    for (i = head_x; i <= head_x + ships1[index - 1].size - 1; i++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_galilee2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player1_grid[i][j].button), image3);
                    }               
                } 
                else {
                    i = head_x - 1;
                    for (j = head_y - 1; j <= head_y + ships1[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    i = head_x + 1;
                    for (j = head_y - 1; j <= head_y + ships1[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    if (head_y - 1 >= 0 && head_y - 1 < GRID_SIZE && data->player1_grid[head_x][head_y - 1].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x][head_y - 1].button), "🚩");
                        data->player1_grid[head_x][head_y - 1].is_shot = TRUE;
                    }
                    if (head_y + ships1[index - 1].size >= 0 && head_y + ships1[index - 1].size < GRID_SIZE && data->player1_grid[head_x][head_y + ships1[index - 1].size].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x][head_y + ships1[index - 1].size].button), "🚩");
                        data->player1_grid[head_x][head_y + ships1[index - 1].size].is_shot = TRUE;
                    }   

                    // Set image
                    i = head_x;
                    for (j = head_y; j <= head_y + ships1[index - 1].size - 1; j++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_galilee2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player1_grid[i][j].button), image3);
                    }               
                }
  
                around_hit.x = 0;
                around_hit.y = 0;
                around_hit.kx = 0;
                around_hit.ky = 0;
                around_hit.hits = 0;
            }

        } 
        else {
            // Missing
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), NULL);
            gtk_button_set_label(GTK_BUTTON(computer_cell->button), "🕳️");
            data->player1_grid[x][y].is_shot = TRUE;

            data->is_player1_turn = TRUE;

            around_hit.x = 0;
            around_hit.y = 0;
            around_hit.kx = 0;
            around_hit.ky = 0;
            around_hit.hits = 0;
        }

        if (data->player1_ships_left == 0) {
            // The game is over, computer won
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Malheureusement, la mission n'a pas été accomplie ❌. Mais ce n'est pas grave, la prochaine fois nous les battrons à coup sûr 💪!");
           for (int i = 0; i < GRID_SIZE; i++) {
            	for (int j = 0; j < GRID_SIZE; j++) {
            		gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
            	}
            }
            
            gtk_label_set_text(GTK_LABEL(data->legend_attention), "Cher Capitaine, perdre n'est pas la fin du monde 🏋️‍♂️🏋️‍♂️. \nVous avez perdu les batailles, mais pas la guerre 💪💪! \nNous Vous conseillons de mieux vous préparer 📙📙 \net de jouer la prochaine fois 🥊🥊. \nA bientôt !  🤗👋🤗");
			// Obtaining a label style context
			GtkStyleContext *style_context3 = gtk_widget_get_style_context(data->legend_attention);
			// Use CSS to change color
			const char *css3 = "label { color: #FBFF00; font-size: 23px; font-family: Calibri; }";
			GtkCssProvider *css_provider3 = gtk_css_provider_new();
			gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
			gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			g_object_unref(css_provider3);
            
            gtk_widget_set_sensitive(data->button_restart, FALSE);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_main_quit();
            return;
        }
    } 

}




// Function for creating playing field cells
GtkWidget* create_cell_difficult(GameData *data, gboolean is_player1, int x, int y) {
    GtkWidget *button = gtk_button_new();

    // GtkWidget *button = gtk_button_new_with_label("");
    Cell *cell;
    if (is_player1) {
        cell = &data->player1_grid[x][y];
    } 
    else {
        cell = &data->player2_grid[x][y];
    }
    gtk_widget_set_size_request(button, 58, 58);
    cell->button = button;
   
    cell->is_ship = FALSE;
    cell->is_hit = FALSE;
    cell->is_shot = FALSE;

/**********************************************          The part with the missile          **********************************************/
    g_signal_connect(G_OBJECT(button), "key_press_event", G_CALLBACK(button_key_press), NULL);
    g_signal_connect(G_OBJECT(button), "key_release_event", G_CALLBACK(button_key_release), NULL);
/**********************************************                                             **********************************************/


	g_signal_connect(button, "clicked", G_CALLBACK(on_cell_clicked_difficult), data);   
	g_object_set_data(G_OBJECT(button), "cell_data", cell);

	g_object_set_data(G_OBJECT(button), "row", GINT_TO_POINTER(x));
	g_object_set_data(G_OBJECT(button), "col", GINT_TO_POINTER(y));

    if (is_player1)
        gtk_widget_set_sensitive(GTK_WIDGET(cell->button), FALSE);
    

    return button;
}



// Create a playing field function
GtkWidget* create_grid_difficult(GameData *data, gboolean is_player1) {	
	
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            GtkWidget *cell = create_cell_difficult(data, is_player1, x, y);
            gtk_grid_attach(GTK_GRID(grid), cell, x, y, 1, 1);
        }
    }

    return grid;
}
/**************************************************          END FUNCTIONS FOR DIFFICULT LEVEL         **************************************************/






/**************************************************          FUNCTIONS FOR AVERAGE LEVEL         **************************************************/

// Function for handling a click event on a playing field cell
void on_cell_clicked_average(GtkWidget *widget, gpointer user_data) {

    GameData *data = (GameData *)user_data;
    gtk_widget_set_sensitive(data->button_replace_flot, FALSE);

    // If "is_player1_turn" = 0  => not our turn, computer must play
    if (!data->is_player1_turn) {
        // Computer move, ignore player presses
        return;
    }
    while (data->is_player1_turn) { 

        Cell *cell = (Cell *)g_object_get_data(G_OBJECT(widget), "cell_data");
        // GtkWidget *widget = GTK_WIDGET(widget);
        int row = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell->button), "row"));
        int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell->button), "col"));
        data->player2_grid[row][col].is_shot = TRUE;


        if (cell->is_hit) {
            // The cell has already been shot up
            return;
        }

        if (cell->is_ship) {
            // The ship is hit
            cell->is_hit = TRUE;
            gtk_button_set_label(GTK_BUTTON(cell->button), NULL);
            GtkWidget *image2 = gtk_image_new_from_file("boom_44.png");
            gtk_button_set_image(GTK_BUTTON(cell->button), image2);
            // gtk_button_set_label(GTK_BUTTON(cell->button), "🔥");
            data->player2_ships_left--;
            data->is_player1_turn = TRUE;

            // Set Flags for Player1

            int index = grid_ships2[col][row];
            ships2[index - 1].size_leftover = ships2[index - 1].size_leftover - 1;


            if ((row != 0) && (col != 0) && (data->player2_grid[row - 1][col - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row - 1][col - 1].button), "🚩");
                data->player2_grid[row - 1][col - 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row - 1][col - 1].button), FALSE);
            }

            if ((row != GRID_SIZE - 1) && (col != GRID_SIZE - 1) && (data->player2_grid[row + 1][col + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row + 1][col + 1].button), "🚩");
                data->player2_grid[row + 1][col + 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row + 1][col + 1].button), FALSE);
            }

            if ((row != 0) && (col != GRID_SIZE - 1) && (data->player2_grid[row - 1][col + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row - 1][col + 1].button), "🚩");
                data->player2_grid[row - 1][col + 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row - 1][col + 1].button), FALSE);
            }

            if ((row != GRID_SIZE - 1) && (col != 0) && (data->player2_grid[row + 1][col - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row + 1][col - 1].button), "🚩");
                data->player2_grid[row + 1][col - 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row + 1][col - 1].button), FALSE);
            }


            if (ships2[index - 1].size_leftover == 0) {
                int head_x = ships2[index - 1].y;
                int head_y = ships2[index - 1].x;

                int i, j;
                if (ships2[index - 1].horizontal == 1) {
                    j = head_y - 1;
                    for (i = head_x - 1; i <= head_x + ships2[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    j = head_y + 1;
                    for (i = head_x - 1; i <= head_x + ships2[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    if (head_x - 1 >= 0 && head_x - 1 < GRID_SIZE && data->player2_grid[head_x - 1][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x - 1][head_y].button), "🚩");
                        data->player2_grid[head_x - 1][head_y].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x - 1][head_y].button), FALSE);
                    }
                    if (head_x + ships2[index - 1].size >= 0 && head_x + ships2[index - 1].size < GRID_SIZE && data->player2_grid[head_x + ships1[index - 1].size][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x + ships1[index - 1].size][head_y].button), "🚩");
                        data->player2_grid[head_x + ships2[index - 1].size][head_y].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x + ships1[index - 1].size][head_y].button), FALSE);
                    }

                    // Set image
                    j = head_y;
                    for (i = head_x; i <= head_x + ships2[index - 1].size - 1; i++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_pirate2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player2_grid[i][j].button), image3);
                    }             
                } 
                else {
                    i = head_x - 1;
                    for (j = head_y - 1; j <= head_y + ships2[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    i = head_x + 1;
                    for (j = head_y - 1; j <= head_y + ships2[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    if (head_y - 1 >= 0 && head_y - 1 < GRID_SIZE && data->player2_grid[head_x][head_y - 1].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x][head_y - 1].button), "🚩");
                        data->player2_grid[head_x][head_y - 1].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x][head_y - 1].button), FALSE);
                    }
                    if (head_y + ships2[index - 1].size >= 0 && head_y + ships2[index - 1].size < GRID_SIZE && data->player2_grid[head_x][head_y + ships2[index - 1].size].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x][head_y + ships2[index - 1].size].button), "🚩");
                        data->player2_grid[head_x][head_y + ships2[index - 1].size].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x][head_y + ships2[index - 1].size].button), FALSE);
                    } 

                    // Set image
                    i = head_x;
                    for (j = head_y; j <= head_y + ships2[index - 1].size - 1; j++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_pirate2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player2_grid[i][j].button), image3);
                    }              
                } 

            }

        } 
        else {
            // Missing
            // GtkWidget *image2 = gtk_image_new_from_file("blunder_47.png");
            // gtk_button_set_image(GTK_BUTTON(cell->button), image2);
            gtk_button_set_label(GTK_BUTTON(cell->button), "🕳️");
            data->is_player1_turn = FALSE;
        }

        // If computer has 0 ships, so we win

        if (data->player2_ships_left == 0) {
            // The game is over, the player won
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Bravo ✌️🎉👍! Mission accomplie ✅, Vous avez protégé la science 👩‍🔬 contre des pirates ! Vous êtes un bon 🎖️ Commandant 🎖️ de la Sup Galilée Marine Nationale 🇫🇷. Continuez comme ça, Capitaine 🤝💪!");
            for (int i = 0; i < GRID_SIZE; i++) {
            	for (int j = 0; j < GRID_SIZE; j++) {
            		gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
            	}
            }
            
            gtk_label_set_text(GTK_LABEL(data->legend_attention), "🏅🏅   Cher Capitaine, gagner, c'est génial. \nLes guerres se gagnent par de petites victoires  ✌️✌️! \nMais nous savons tous les deux, \nque trop de jeux peuvent être dangereux pour la santé, \nnous Vous conseillons donc de faire une pause  🏁🏁. \nÀ bientôt !  🤗👋🤗");
			// Obtaining a label style context
			GtkStyleContext *style_context3 = gtk_widget_get_style_context(data->legend_attention);
			// Use CSS to change color
			const char *css3 = "label { color: #FBFF00; font-size: 23px; font-family: Calibri; }";
			GtkCssProvider *css_provider3 = gtk_css_provider_new();
			gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
			gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			g_object_unref(css_provider3);
			
			gtk_widget_set_sensitive(data->button_restart, FALSE);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_main_quit();
            return;
            
        }
        gtk_widget_set_sensitive(GTK_WIDGET(cell->button), FALSE);
    } 
    


/*********************************************         Computer stroke         *********************************************/

    while (!data->is_player1_turn) {

        // while (!data->player1_grid[rand() % GRID_SIZE][rand() % GRID_SIZE].is_ship) {
        // The computer selects a random cell until it finds a cell with a ship
        // }

        // The computer takes the shot
        Cell *computer_cell;
        int flag_test = 0;
        int x, y;
        while (flag_test == 0) {
            x = rand() % GRID_SIZE;
            y = rand() % GRID_SIZE;
            if (data->player1_grid[x][y].is_shot == FALSE) { 
                computer_cell = &data->player1_grid[x][y];
                flag_test++;
                data->player1_grid[x][y].is_shot = TRUE;
            } 
        }


        if (computer_cell->is_ship) {
            // The ship is hit
            computer_cell->is_hit = TRUE;
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), NULL);
            GtkWidget *image = gtk_image_new_from_file("boom_44.png");
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), image);
            // gtk_button_set_label(GTK_BUTTON(computer_cell->button), "🔥");
            data->player1_ships_left--;
            data->is_player1_turn = FALSE;

            // Set Flags for Player1
            int index = grid_ships1[y][x];
            ships1[index - 1].size_leftover = ships1[index - 1].size_leftover - 1;

            if (ships1[index - 1].size_leftover == 0) {
                int head_x = ships1[index - 1].y;
                int head_y = ships1[index - 1].x;

                int i, j;
                if (ships1[index - 1].horizontal == 1) {
                    j = head_y - 1;
                    for (i = head_x - 1; i <= head_x + ships1[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    j = head_y + 1;
                    for (i = head_x - 1; i <= head_x + ships1[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    if (head_x - 1 >= 0 && head_x - 1 < GRID_SIZE && data->player1_grid[head_x - 1][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x - 1][head_y].button), "🚩");
                        data->player1_grid[head_x - 1][head_y].is_shot = TRUE;
                    }
                    if (head_x + ships1[index - 1].size >= 0 && head_x + ships1[index - 1].size < GRID_SIZE && data->player1_grid[head_x + ships1[index - 1].size][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x + ships1[index - 1].size][head_y].button), "🚩");
                        data->player1_grid[head_x + ships1[index - 1].size][head_y].is_shot = TRUE;
                    }   
                    
                    // Set image
                    j = head_y;
                    for (i = head_x; i <= head_x + ships1[index - 1].size - 1; i++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_galilee2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player1_grid[i][j].button), image3);
                    }             
                } 
                else {
                    i = head_x - 1;
                    for (j = head_y - 1; j <= head_y + ships1[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    i = head_x + 1;
                    for (j = head_y - 1; j <= head_y + ships1[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player1_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player1_grid[i][j].button), "🚩");
                            data->player1_grid[i][j].is_shot = TRUE;
                        } 
                    }
                    if (head_y - 1 >= 0 && head_y - 1 < GRID_SIZE && data->player1_grid[head_x][head_y - 1].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x][head_y - 1].button), "🚩");
                        data->player1_grid[head_x][head_y - 1].is_shot = TRUE;
                    }
                    if (head_y + ships1[index - 1].size >= 0 && head_y + ships1[index - 1].size < GRID_SIZE && data->player1_grid[head_x][head_y + ships1[index - 1].size].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player1_grid[head_x][head_y + ships1[index - 1].size].button), "🚩");
                        data->player1_grid[head_x][head_y + ships1[index - 1].size].is_shot = TRUE;
                    }
                    
                    // Set image
                    i = head_x;
                    for (j = head_y; j <= head_y + ships1[index - 1].size - 1; j++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_galilee2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player1_grid[i][j].button), image3);
                    }                    
                } 
            } 
        } 
        else {
            // Missing
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), NULL);
            gtk_button_set_label(GTK_BUTTON(computer_cell->button), "🕳️");
            // computer_cell->is_shot = TRUE;
            data->is_player1_turn = TRUE;
        }

        if (data->player1_ships_left == 0) {
            // The game is over, computer won
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Malheureusement, la mission n'a pas été accomplie ❌. Mais ce n'est pas grave, la prochaine fois nous les battrons à coup sûr 💪!");
            for (int i = 0; i < GRID_SIZE; i++) {
            	for (int j = 0; j < GRID_SIZE; j++) {
            		gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
            	}
            }
            
            gtk_label_set_text(GTK_LABEL(data->legend_attention), "Cher Capitaine, perdre n'est pas la fin du monde 🏋️‍♂️🏋️‍♂️. \nVous avez perdu les batailles, mais pas la guerre 💪💪! \nNous Vous conseillons de mieux vous préparer 📙📙 \net de jouer la prochaine fois 🥊🥊. \nA bientôt !  🤗👋🤗");
			// Obtaining a label style context
			GtkStyleContext *style_context3 = gtk_widget_get_style_context(data->legend_attention);
			// Use CSS to change color
			const char *css3 = "label { color: #FBFF00; font-size: 23px; font-family: Calibri; }";
			GtkCssProvider *css_provider3 = gtk_css_provider_new();
			gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
			gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			g_object_unref(css_provider3);
            
            gtk_widget_set_sensitive(data->button_restart, FALSE);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_main_quit();
            return;
        }
    } 

    // data->is_player1_turn = TRUE;
}




// Function for creating playing field cells
GtkWidget* create_cell_average(GameData *data, gboolean is_player1, int x, int y) {
    GtkWidget *button = gtk_button_new();

    // GtkWidget *button = gtk_button_new_with_label("");
    Cell *cell;
    if (is_player1) {
        cell = &data->player1_grid[x][y];
    } 
    else {
        cell = &data->player2_grid[x][y];
    }
    gtk_widget_set_size_request(button, 58, 58);
    cell->button = button;
   
    cell->is_ship = FALSE;
    cell->is_hit = FALSE;
    cell->is_shot = FALSE;

/**********************************************          The part with the missile          **********************************************/
    g_signal_connect(G_OBJECT(button), "key_press_event", G_CALLBACK(button_key_press), NULL);
    g_signal_connect(G_OBJECT(button), "key_release_event", G_CALLBACK(button_key_release), NULL);
/**********************************************                                             **********************************************/


	g_signal_connect(button, "clicked", G_CALLBACK(on_cell_clicked_average), data);   
	g_object_set_data(G_OBJECT(button), "cell_data", cell);

	g_object_set_data(G_OBJECT(button), "row", GINT_TO_POINTER(x));
	g_object_set_data(G_OBJECT(button), "col", GINT_TO_POINTER(y));

    if (is_player1)
        gtk_widget_set_sensitive(GTK_WIDGET(cell->button), FALSE);
    

    return button;
}



// Create a playing field function
GtkWidget* create_grid_average(GameData *data, gboolean is_player1) {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            GtkWidget *cell = create_cell_average(data, is_player1, x, y);
            gtk_grid_attach(GTK_GRID(grid), cell, x, y, 1, 1);
        }
    }

    return grid;
}
/**************************************************          END FUNCTIONS FOR AVERAGE LEVEL         **************************************************/








/**************************************************          FUNCTIONS FOR EASY LEVEL         **************************************************/

// Function for handling a click event on a playing field cell
void on_cell_clicked_easy(GtkWidget *widget, gpointer user_data) {

    GameData *data = (GameData *)user_data;
    gtk_widget_set_sensitive(data->button_replace_flot, FALSE);

    // If "is_player1_turn" = 0  => not our turn, computer must play
    if (!data->is_player1_turn) {
        // Computer move, ignore player presses
        return;
    }
    while (data->is_player1_turn) { 

        Cell *cell = (Cell *)g_object_get_data(G_OBJECT(widget), "cell_data");
        // GtkWidget *widget = GTK_WIDGET(widget);
        int row = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell->button), "row"));
        int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell->button), "col"));
        data->player2_grid[row][col].is_shot = TRUE;


        if (cell->is_hit) {
            // The cell has already been shot up
            return;
        }

        if (cell->is_ship) {
            // The ship is hit
            cell->is_hit = TRUE;
            gtk_button_set_label(GTK_BUTTON(cell->button), NULL);
            GtkWidget *image2 = gtk_image_new_from_file("boom_44.png");
            gtk_button_set_image(GTK_BUTTON(cell->button), image2);
            // gtk_button_set_label(GTK_BUTTON(cell->button), "🔥");
            data->player2_ships_left--;
            data->is_player1_turn = TRUE;

            // Set Flags for Player1

            int index = grid_ships2[col][row];
            ships2[index - 1].size_leftover = ships2[index - 1].size_leftover - 1;


            if ((row != 0) && (col != 0) && (data->player2_grid[row - 1][col - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row - 1][col - 1].button), "🚩");
                data->player2_grid[row - 1][col - 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row - 1][col - 1].button), FALSE);
            }

            if ((row != GRID_SIZE - 1) && (col != GRID_SIZE - 1) && (data->player2_grid[row + 1][col + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row + 1][col + 1].button), "🚩");
                data->player2_grid[row + 1][col + 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row + 1][col + 1].button), FALSE);
            }

            if ((row != 0) && (col != GRID_SIZE - 1) && (data->player2_grid[row - 1][col + 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row - 1][col + 1].button), "🚩");
                data->player2_grid[row - 1][col + 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row - 1][col + 1].button), FALSE);
            }

            if ((row != GRID_SIZE - 1) && (col != 0) && (data->player2_grid[row + 1][col - 1].is_shot == FALSE)) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[row + 1][col - 1].button), "🚩");
                data->player2_grid[row + 1][col - 1].is_shot = TRUE;
                gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[row + 1][col - 1].button), FALSE);
            }


            if (ships2[index - 1].size_leftover == 0) {
                int head_x = ships2[index - 1].y;
                int head_y = ships2[index - 1].x;

                int i, j;
                if (ships2[index - 1].horizontal == 1) {
                    j = head_y - 1;
                    for (i = head_x - 1; i <= head_x + ships2[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    j = head_y + 1;
                    for (i = head_x - 1; i <= head_x + ships2[index - 1].size; i++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    if (head_x - 1 >= 0 && head_x - 1 < GRID_SIZE && data->player2_grid[head_x - 1][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x - 1][head_y].button), "🚩");
                        data->player2_grid[head_x - 1][head_y].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x - 1][head_y].button), FALSE);
                    }
                    if (head_x + ships2[index - 1].size >= 0 && head_x + ships2[index - 1].size < GRID_SIZE && data->player2_grid[head_x + ships1[index - 1].size][head_y].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x + ships1[index - 1].size][head_y].button), "🚩");
                        data->player2_grid[head_x + ships2[index - 1].size][head_y].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x + ships1[index - 1].size][head_y].button), FALSE);
                    }

                    // Set image
                    j = head_y;
                    for (i = head_x; i <= head_x + ships2[index - 1].size - 1; i++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_pirate2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player2_grid[i][j].button), image3);
                    }             
                } 
                else {
                    i = head_x - 1;
                    for (j = head_y - 1; j <= head_y + ships2[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    i = head_x + 1;
                    for (j = head_y - 1; j <= head_y + ships2[index - 1].size; j++) {
                        if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && data->player2_grid[i][j].is_shot == FALSE) {
                            gtk_button_set_label(GTK_BUTTON(data->player2_grid[i][j].button), "🚩");
                            data->player2_grid[i][j].is_shot = TRUE;
                            gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
                        } 
                    }
                    if (head_y - 1 >= 0 && head_y - 1 < GRID_SIZE && data->player2_grid[head_x][head_y - 1].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x][head_y - 1].button), "🚩");
                        data->player2_grid[head_x][head_y - 1].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x][head_y - 1].button), FALSE);
                    }
                    if (head_y + ships2[index - 1].size >= 0 && head_y + ships2[index - 1].size < GRID_SIZE && data->player2_grid[head_x][head_y + ships2[index - 1].size].is_shot == FALSE) {
                        gtk_button_set_label(GTK_BUTTON(data->player2_grid[head_x][head_y + ships2[index - 1].size].button), "🚩");
                        data->player2_grid[head_x][head_y + ships2[index - 1].size].is_shot = TRUE;
                        gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[head_x][head_y + ships2[index - 1].size].button), FALSE);
                    } 

                    // Set image
                    i = head_x;
                    for (j = head_y; j <= head_y + ships2[index - 1].size - 1; j++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_pirate2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player2_grid[i][j].button), image3);
                    }              
                } 

            }

        } 
        else {
            // Missing
            // GtkWidget *image2 = gtk_image_new_from_file("blunder_47.png");
            // gtk_button_set_image(GTK_BUTTON(cell->button), image2);
            gtk_button_set_label(GTK_BUTTON(cell->button), "🕳️");
            data->is_player1_turn = FALSE;
        }

        // If computer has 0 ships, so we win
        if (data->player2_ships_left == 0) {
            // The game is over, the player won
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Bravo ✌️🎉👍! Mission accomplie ✅, Vous avez protégé la science 👩‍🔬 des pirates ! Vous êtes un bon 🎖️ Commandant 🎖️ de la Sup Galilée Marine Nationale 🇫🇷. Continuez comme ça, Capitaine 🤝💪!");
            for (int i = 0; i < GRID_SIZE; i++) {
            	for (int j = 0; j < GRID_SIZE; j++) {
            		gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
            	}
            }
            
            gtk_label_set_text(GTK_LABEL(data->legend_attention), "🏅🏅   Cher Capitaine, gagner, c'est génial. \nLes guerres se gagnent par de petites victoires  ✌️✌️! \nMais nous savons tous les deux, \nque trop de jeux peuvent être dangereux pour la santé, \nnous Vous conseillons donc de faire une pause  🏁🏁. \nÀ bientôt !  🤗👋🤗");
			// Obtaining a label style context
			GtkStyleContext *style_context3 = gtk_widget_get_style_context(data->legend_attention);
			// Use CSS to change color
			const char *css3 = "label { color: #FBFF00; font-size: 23px; font-family: Calibri; }";
			GtkCssProvider *css_provider3 = gtk_css_provider_new();
			gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
			gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			g_object_unref(css_provider3);
            
            gtk_widget_set_sensitive(data->button_restart, FALSE);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_main_quit();
            return;
        }
        gtk_widget_set_sensitive(GTK_WIDGET(cell->button), FALSE);
    } 
    


/*********************************************         Computer stroke         *********************************************/

    // data->is_player1_turn = FALSE;

    while (!data->is_player1_turn) {

        /*while (!data->player1_grid[rand() % GRID_SIZE][rand() % GRID_SIZE].is_ship) {
            // The computer selects a random cell until it finds a cell with a ship
        }*/
        
        // The computer takes the shot
        Cell *computer_cell;
        int flag_test = 0;
        int x, y;
        while (flag_test == 0) {
            x = rand() % GRID_SIZE;
            y = rand() % GRID_SIZE;
            if (data->player1_grid[x][y].is_shot == FALSE) { 
                computer_cell = &data->player1_grid[x][y];
                flag_test++;
                data->player1_grid[x][y].is_shot = TRUE;
            } 
        } 


        if (computer_cell->is_ship) {
        	 
        	// The ship is hit
            computer_cell->is_hit = TRUE;
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), NULL);
            GtkWidget *image = gtk_image_new_from_file("boom_44.png");
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), image);
            // gtk_button_set_label(GTK_BUTTON(computer_cell->button), "🔥");
            data->player1_ships_left = data->player1_ships_left - 1;
            data->is_player1_turn = FALSE;
            data->player1_grid[x][y].is_shot = TRUE;


            // Set Flags for Player1
            int index = grid_ships1[y][x];
            ships1[index - 1].size_leftover = ships1[index - 1].size_leftover - 1;
            
            
            if (ships1[index - 1].size_leftover == 0) {
                int head_x = ships1[index - 1].y;
                int head_y = ships1[index - 1].x;

                int i, j;
                if (ships1[index - 1].horizontal == 1) {

                    // Set image
                    j = head_y;
                    for (i = head_x; i <= head_x + ships1[index - 1].size - 1; i++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_galilee2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player1_grid[i][j].button), image3);
                    }               
                } 
                else {
                    
                    // Set image
                    i = head_x;
                    for (j = head_y; j <= head_y + ships1[index - 1].size - 1; j++) {
                        GtkWidget *image3 = gtk_image_new_from_file("mil_ship_bung_galilee2_43.png");
                        gtk_button_set_image(GTK_BUTTON(data->player1_grid[i][j].button), image3);
                    }               
                }
            } 
            
        } 
        else {
            // Missing
            gtk_button_set_image(GTK_BUTTON(computer_cell->button), NULL);
            gtk_button_set_label(GTK_BUTTON(computer_cell->button), "🕳️");
            // computer_cell->is_shot = TRUE;
            data->is_player1_turn = TRUE;
        }

        if (data->player1_ships_left == 0) {
            // The game is over, computer won
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(data->window),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK,
                                                    "Malheureusement, la mission n'a pas été accomplie ❌. Mais ce n'est pas grave, la prochaine fois nous les battrons à coup sûr 💪!");
            for (int i = 0; i < GRID_SIZE; i++) {
            	for (int j = 0; j < GRID_SIZE; j++) {
            		gtk_widget_set_sensitive(GTK_WIDGET(data->player2_grid[i][j].button), FALSE);
            	}
            }
            
            gtk_label_set_text(GTK_LABEL(data->legend_attention), "Cher Capitaine, perdre n'est pas la fin du monde 🏋️‍♂️🏋️‍♂️. \nVous avez perdu les batailles, mais pas la guerre 💪💪! \nNous Vous conseillons de mieux vous préparer 📙📙 \net de jouer la prochaine fois 🥊🥊. \nA bientôt !  🤗👋🤗");
			// Obtaining a label style context
			GtkStyleContext *style_context3 = gtk_widget_get_style_context(data->legend_attention);
			// Use CSS to change color
			const char *css3 = "label { color: #FBFF00; font-size: 23px; font-family: Calibri; }";
			GtkCssProvider *css_provider3 = gtk_css_provider_new();
			gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
			gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
			g_object_unref(css_provider3);
            
            gtk_widget_set_sensitive(data->button_restart, FALSE);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            gtk_main_quit();
            return;
        }
    } 

    // data->is_player1_turn = TRUE;
}



// Function for creating playing field cells
GtkWidget* create_cell_easy(GameData *data, gboolean is_player1, int x, int y) {
    GtkWidget *button = gtk_button_new();

    // GtkWidget *button = gtk_button_new_with_label("");
    Cell *cell;
    if (is_player1) {
        cell = &data->player1_grid[x][y];
    } 
    else {
        cell = &data->player2_grid[x][y];
    }
    gtk_widget_set_size_request(button, 58, 58);
    cell->button = button;
   
    cell->is_ship = FALSE;
    cell->is_hit = FALSE;
    cell->is_shot = FALSE;

/**********************************************          The part with the missile          **********************************************/
    g_signal_connect(G_OBJECT(button), "key_press_event", G_CALLBACK(button_key_press), NULL);
    g_signal_connect(G_OBJECT(button), "key_release_event", G_CALLBACK(button_key_release), NULL);
/**********************************************                                             **********************************************/


	g_signal_connect(button, "clicked", G_CALLBACK(on_cell_clicked_easy), data);   
	g_object_set_data(G_OBJECT(button), "cell_data", cell);

	g_object_set_data(G_OBJECT(button), "row", GINT_TO_POINTER(x));
	g_object_set_data(G_OBJECT(button), "col", GINT_TO_POINTER(y));

    if (is_player1)
        gtk_widget_set_sensitive(GTK_WIDGET(cell->button), FALSE);
    

    return button;
}



// Create a playing field function
GtkWidget* create_grid_easy(GameData *data, gboolean is_player1) {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            GtkWidget *cell = create_cell_easy(data, is_player1, x, y);
            gtk_grid_attach(GTK_GRID(grid), cell, x, y, 1, 1);
        }
    }

    return grid;
}
/**************************************************          END FUNCTIONS FOR EASY LEVEL         **************************************************/




/**************************************************          FUNCTIONS FOR CONTROLE SHIPS          **************************************************/
static gboolean check_neighbors(int x, int y, int size, int horizontal) {
    int i, j;

    if (horizontal == 1) {
        for (i = x - 1; i <= x + 1; i++) {
            for (j = y - 1; j <= y + size; j++) {
                if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE) {
                    if (grid1[i][j] == 1)
                        return FALSE;
                }
            }
        }
    } 
    else {
        for (i = x - 1; i <= x + size; i++) {
            for (j = y - 1; j <= y + 1; j++) {
                if (i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE) {
                    if (grid1[i][j] == 1) 
                        return FALSE;
                }
            }   
        }
    }

    return TRUE;
}



static gboolean is_valid_ship(int ship_index) {
    int x = ships1[ship_index].x;
    int y = ships1[ship_index].y;
    int size = ships1[ship_index].size;
    int horizontal = ships1[ship_index].horizontal;

    if (horizontal) {
        if (x + size - 1 > GRID_SIZE) 
            return FALSE;
    } else {
        if (y + size - 1 > GRID_SIZE) 
            return FALSE;
    }

    return check_neighbors(x, y, size, horizontal);
}



static void place_ships1(GameData *data) {
    int i, j, k;
    int attempts = 0;

    for (int c1 = 0; c1 < NUM_SHIPS; c1++) {
        for (int c2 = 0; c2 < NUM_SHIPS; c2++) 
            data->player1_grid[c1][c2].is_ship = FALSE;
    }

    for (k = 0; k < NUM_SHIPS; k++) {

        do {
            attempts++;
            ships1[k].horizontal = rand() % 2;
            if (ships1[k].horizontal) {
                ships1[k].x = rand() % GRID_SIZE;
                ships1[k].y = rand() % (GRID_SIZE - ships1[k].size + 1);

            } 
            else {
                ships1[k].x = rand() % (GRID_SIZE - ships1[k].size + 1);
                ships1[k].y = rand() % GRID_SIZE;
            }

        } while (!is_valid_ship(k) && attempts < NUM_ATTEMPTS);

        if (attempts >= NUM_ATTEMPTS) {
            printf("%d\n",k);

            for (int i = 0; i < NUM_SHIPS; i++) {
                for (int j = 0; j < NUM_SHIPS; j++) {
                    printf("%d", grid1[i][j]);
                }
                printf("\n");
            }
        } 
        else {
            int x = ships1[k].x;
            int y = ships1[k].y;
            // int size = ships1[k].size;
            // int horizontal = ships1[k].horizontal;
            int lenth = ships1[k].size;

            if (ships1[k].horizontal) {
                for (int t = y; t <= y + lenth - 1; t++)
                    grid1[x][t]= 1;
            } 
            else {
                for (int s = x; s <= x + lenth - 1; s++)
                    grid1[s][y] = 1; 
            }
        }
    }

    for (i = 0; i < NUM_SHIPS; i++) {
        for (j = 0; j < NUM_SHIPS; j++) {
            if (grid1[i][j] == 1) {
                data->player1_grid[j][i].is_ship = TRUE;
            }
        }
    }
}



static gboolean is_valid_ship2(int ship_index) {
    int x = ships2[ship_index].x;
    int y = ships2[ship_index].y;
    int size = ships2[ship_index].size;
    int horizontal = ships2[ship_index].horizontal;

    if (horizontal) {
        if (y + size - 1> GRID_SIZE)
            return FALSE;
    } 
    else {
        if (x + size - 1 > GRID_SIZE)
            return FALSE;
    }

    return check_neighbors(x, y, size, horizontal);
}



static void place_ships2(GameData *data) {

    int k;
    int attempts = 0;

     for (int c1 = 0; c1 < NUM_SHIPS; c1++) {
        for (int c2 = 0; c2 < NUM_SHIPS; c2++) {
            grid1[c1][c2] = 0;
        }
    }

    for (int c1 = 0; c1 < NUM_SHIPS; c1++) {
        for (int c2 = 0; c2 < NUM_SHIPS; c2++) {
            data->player2_grid[c1][c2].is_ship = FALSE;
        }
    }

    for (k = 0; k < NUM_SHIPS; k++) {
        do {
            attempts++;
            ships2[k].horizontal = rand() % 2;
            if (ships2[k].horizontal) {
                ships2[k].x = rand() % GRID_SIZE;
                ships2[k].y = rand() % (GRID_SIZE - ships2[k].size + 1);

            } 
            else {
                ships2[k].x = rand() % (GRID_SIZE - ships2[k].size + 1);
                ships2[k].y = rand() % GRID_SIZE;
            }

        } while (!is_valid_ship2(k) && attempts < NUM_ATTEMPTS);

        if (attempts >= NUM_ATTEMPTS) {
            //
        } 
        else {
            int x = ships2[k].x;
            int y = ships2[k].y;
            // int size = ships2[k].size;
            // int horizontal = ships2[k].horizontal;
            int lenth = ships2[k].size;

            if (ships2[k].horizontal) {
                for (int t = y; t <= y + lenth - 1; t++)
                    grid1[x][t]= 1;
            }
            else {
                for (int s = x; s <= x + lenth - 1; s++)
                    grid1[s][y] = 1; 
            }
        }

    }

    for (int c1 = 0; c1 < NUM_SHIPS; c1++) {
        for (int c2 = 0; c2 < NUM_SHIPS; c2++) {
            if (grid1[c1][c2] == 1) 
                data->player2_grid[c2][c1].is_ship = TRUE;
        }
    }

}

static void flot() {
    
    for (int k = 0; k < NUM_SHIPS; k++) {
        int x = ships1[k].x;
        int y = ships1[k].y;
        int lenth = ships1[k].size;
        int horizontal = ships1[k].horizontal;

        if (horizontal == 1) {
            for (int t = y; t <= y + lenth - 1; t++)
                grid_ships1[x][t]= k + 1;
            } 
        else {
            for (int s = x; s <= x + lenth - 1; s++)
                grid_ships1[s][y] = k + 1;
        }
    } 

    for (int k = 0; k < NUM_SHIPS; k++) {
        int x = ships2[k].x;
        int y = ships2[k].y;
        int lenth = ships2[k].size;
        int horizontal = ships2[k].horizontal;

        if (horizontal == 1) {
            for (int t = y; t <= y + lenth - 1; t++)
                grid_ships2[x][t]= k + 1;
            } 
        else {
            for (int s = x; s <= x + lenth - 1; s++)
                grid_ships2[s][y] = k + 1; 
        }
    }     
} 



void visualize_ships(GameData *data) {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (data->player1_grid[x][y].is_ship) {
                GtkWidget *image = gtk_image_new_from_file("mil_ship_flag_galilee_43.png");
                gtk_button_set_image(GTK_BUTTON(data->player1_grid[x][y].button), image);
            }
            if (data->player2_grid[x][y].is_ship) {
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[x][y].button), "");
                // GtkWidget *image = gtk_image_new_from_file("mil_ship_43.png");
                // gtk_button_set_image(GTK_BUTTON(data->player2_grid[x][y].button), image);
            }
        }
    }
}
/**************************************************          END FUNCTIONS FOR CONTROLE SHIPS          **************************************************/



/**************************************************          FUNCTIONS FOR DESIGN          **************************************************/
void show_text_window_rules(GtkWidget *widget, gpointer data) {
    // Creating new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "👉  REGLES  👈");
    gtk_window_set_default_size(GTK_WINDOW(window), 1600, 600);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(main_box));

    GtkWidget *legend_box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(legend_box1));

    // Creating widget, like a text 
    GtkWidget *label1 = (GtkWidget*)gtk_label_new("\n\n📖  REGLES DU JEU  📖\n\n");
    GtkWidget *label2 = (GtkWidget*)gtk_label_new("➡️ Chaque joueur possède une flotte de 10 bateaux 🚢 :");
    GtkWidget *label3 = (GtkWidget*)gtk_label_new("🔻1 étage (une seule case) \t\t–  4 bateaux ;\n🔻2 étage (deux cases) \t\t\t–  3 bateaux ;\n🔻3 étage (trois cases) \t\t\t–  2 bateaux ;\n🔻4 étage (quatre cases) \t\t–  1 bateau;");
    GtkWidget *label4 = (GtkWidget*)gtk_label_new("➡️ Tous ces bateaux ⛵ doivent être placés sur une grille carrée de 🔟 cases sur 🔟.");
    GtkWidget *label5 = (GtkWidget*)gtk_label_new("➡️ Ils ne peuvent pas se toucher 🚫 ni par les coins, ni par les côtés 🚫.");
    GtkWidget *label6 = (GtkWidget*)gtk_label_new("➡️ L’objectif de ce jeu est de détruire 🔥 tous les bateaux de l’adversaire. Le premier joueur à y parvenir remporte la partie ✌️.");
    GtkWidget *label7 = (GtkWidget*)gtk_label_new("➡️ Lorsqu’un tir réussit touche un bateau ennemi, un signe de feu 💥 est placé sur la case correspondante du champ de bataille de l’adversaire, et un tir de suivi est effectué.");
    GtkWidget *label8 = (GtkWidget*)gtk_label_new("➡️ En cas d’échec, un point est marqué sur la case correspondante, et le tour passe à l’adversaire.");
    GtkWidget *label9 = (GtkWidget*)gtk_label_new("➡️ Le jeu se termine lorsque l’un des joueurs a perdu tous ses bateaux ❌🛳️❌.");

    // Obtaining a label style context
    GtkStyleContext *style_context1 = gtk_widget_get_style_context(label1);
    GtkStyleContext *style_context2 = gtk_widget_get_style_context(label2);
    GtkStyleContext *style_context3 = gtk_widget_get_style_context(label3);
    GtkStyleContext *style_context4 = gtk_widget_get_style_context(label4);
    GtkStyleContext *style_context5 = gtk_widget_get_style_context(label5);
    GtkStyleContext *style_context6 = gtk_widget_get_style_context(label6);
    GtkStyleContext *style_context7 = gtk_widget_get_style_context(label7);
    GtkStyleContext *style_context8 = gtk_widget_get_style_context(label8);
    GtkStyleContext *style_context9 = gtk_widget_get_style_context(label9);
    // Use CSS to change color
    // const char *css1 = "label { color: #000000; font-size: 18px; font-family: Arial; }";
    const char *css1 = "label { color: #000000; font-size: 17px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider1, css1, -1, NULL);
    gtk_style_context_add_provider(style_context1, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context2, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context4, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context5, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context6, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context7, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context8, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context9, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider1);


    gtk_box_pack_start(GTK_BOX(legend_box1), label1, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label2, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label3, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label4, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label5, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label6, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label7, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label8, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(legend_box1), label9, TRUE, TRUE, 5);


    gtk_widget_show_all(window);
    
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-image: url(\"background3.png\"); }", -1, NULL);
    // Obtaining a window style context and adding a CSS provider
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(window));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}


void show_text_window_trailer(GtkWidget *widget, gpointer data) {
    // Creating new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "👉  BANDE – ANNONCE  👈");
    gtk_window_set_default_size(GTK_WINDOW(window), 2000, 600);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(main_box));

    GtkWidget *legend_box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(legend_box1));

    GtkWidget *label1 = (GtkWidget*)gtk_label_new("\n\n📢🎥  BANDE – ANNONCE  🎥📢\n\n");
    GtkWidget *label2 = (GtkWidget*)gtk_label_new("Nous vous présentons 🚀⛵💥  BATAILLE NAVALE  💥⛵🚀 - le jeu bien-aimé et célèbre de notre enfance  👶, joué dans les cahiers  📝.");
    GtkWidget *label3 = (GtkWidget*)gtk_label_new("Il a été, est et restera le meilleur jeu de classe avec notre voisin de pupitre.");
    GtkWidget *label4 = (GtkWidget*)gtk_label_new("Vous avez maintenant la possibilité de jouer à la fois en mode classique et en mode avancé, dont nous ne pouvions que rêver dans notre enfance !");
    GtkWidget *label5 = (GtkWidget*)gtk_label_new("BATAILLE NAVALE est un jeu stratégique classique pour 2️⃣ joueurs, où vous, en tant que 👩‍✈️ Capitaine 👨‍✈️ de la 🇫🇷  SUP GALILEE MARINE NATIONALE  🇫🇷!");
    GtkWidget *label6 = (GtkWidget*)gtk_label_new("Vous devez découvrir la position des navires 🛳️ de votre adversaire, les ☠️ Pirates ☠️, avant qu'ils ne découvrent vos navires et ne coulent votre flotte.");
    GtkWidget *label7 = (GtkWidget*)gtk_label_new("Tous les 👩‍🎓 étudiants 👨‍🎓, ingénieurs et 🧑‍🏫 enseignants 👨‍🏫 de la SUP GALILEE se trouvent dans une situation très dangereuse ⚠️, une ⚡⚡ question de vie ⚡⚡ ou de mort, car ils sont attaqués par les Pirates.");
    GtkWidget *label8 = (GtkWidget*)gtk_label_new("Remplissez votre devoir en utilisant les navires de votre 🚀 Armada 🚀, prenez le commandement des Forces Navales et détruisez la flottille ennemie en utilisant votre intuition, votre stratégie et surtout vos compétences d'ingénierie.");
    GtkWidget *label9 = (GtkWidget*)gtk_label_new("Développez votre propre tactique qui vous aidera à remporter la victoire sur les trois niveaux de difficulté !");
    GtkWidget *label10 = (GtkWidget*)gtk_label_new("Affrontez les 🏴‍☠️ Pirates 🏴‍☠️ et prouvez que vous avez les talents d'un commandant de flotte réelle et que vous êtes un 💯 Véritable Ingénieur 💯 de la SUP GALILEE 😎✌️!");
    GtkWidget *label11 = (GtkWidget*)gtk_label_new("⚔️ Que la bonté l'emporte ⚔️ et VIVE SUP GALILEE 🦾🔝✌️!");


    // Obtaining a label style context
    GtkStyleContext *style_context1 = gtk_widget_get_style_context(label1);
    GtkStyleContext *style_context2 = gtk_widget_get_style_context(label2);
    GtkStyleContext *style_context3 = gtk_widget_get_style_context(label3);
    GtkStyleContext *style_context4 = gtk_widget_get_style_context(label4);
    GtkStyleContext *style_context5 = gtk_widget_get_style_context(label5);
    GtkStyleContext *style_context6 = gtk_widget_get_style_context(label6);
    GtkStyleContext *style_context7 = gtk_widget_get_style_context(label7);
    GtkStyleContext *style_context8 = gtk_widget_get_style_context(label8);
    GtkStyleContext *style_context9 = gtk_widget_get_style_context(label9);
    GtkStyleContext *style_context10 = gtk_widget_get_style_context(label10);
    GtkStyleContext *style_context11 = gtk_widget_get_style_context(label11);
    // Use CSS to change color
    // const char *css1 = "label { color: #000000; font-size: 18px; font-family: Arial; }";
    const char *css1 = "label { color: #000000; font-size: 17px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider1, css1, -1, NULL);
    gtk_style_context_add_provider(style_context1, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context2, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context4, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context5, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context6, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context7, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context8, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context9, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context10, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(style_context11, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider1);


    gtk_box_pack_start(GTK_BOX(legend_box1), label1, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label2, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label3, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label4, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label5, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label6, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label7, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label8, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label9, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label10, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(legend_box1), label11, TRUE, TRUE, 2);


    gtk_widget_show_all(window);
    
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-image: url(\"background4.png\"); }", -1, NULL);
    // Obtaining a window style context and adding a CSS provider
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(window));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
/**************************************************          END FUNCTIONS FOR DESIGN          **************************************************/



void create_game_interface_start(GameData *data);




/**************************************************          FUNCTION TO EXIT          **************************************************/
void close_window(GtkWidget *widget, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_window_close(GTK_WINDOW(window));
}

void close_window2(GtkWidget *widget, GameData *data) {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			grid1[i][j] = 0;
			grid_ships1[i][j] = 0;
			grid_ships2[i][j] = 0;
		}
	}
	for (int i = 0; i < GRID_SIZE; i++) {
		ships1[i].x = 0;
		ships1[i].y = 0;
		ships2[i].x = 0;
		ships2[i].y = 0;
		ships1[i].horizontal = 1;
		ships2[i].horizontal = 1;
	}
    ships1[0].size_leftover = 4;
    ships2[0].size_leftover = 4;

    ships1[1].size_leftover = 3;
    ships2[1].size_leftover = 3;
    ships1[2].size_leftover = 3;
    ships2[2].size_leftover = 3;

    ships1[3].size_leftover = 2;
    ships2[3].size_leftover = 2;
    ships1[4].size_leftover = 2;
    ships2[4].size_leftover = 2;
    ships1[5].size_leftover = 2;
    ships2[5].size_leftover = 2;

    ships1[6].size_leftover = 1;
    ships2[6].size_leftover = 1;
    ships1[7].size_leftover = 1;
    ships2[7].size_leftover = 1;
    ships1[8].size_leftover = 1;
    ships2[8].size_leftover = 1;
    ships1[9].size_leftover = 1;
    ships2[9].size_leftover = 1;
	
	for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
                data->player1_grid[x][y].is_hit = FALSE;
                data->player1_grid[x][y].is_shot = FALSE;
                data->player1_grid[x][y].is_ship = FALSE;
				data->player2_grid[x][y].is_hit = FALSE;
                data->player2_grid[x][y].is_shot = FALSE;
                data->player2_grid[x][y].is_ship = FALSE;
                gtk_button_set_image(GTK_BUTTON(data->player1_grid[x][y].button), NULL);
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[x][y].button), "");
        }
    }

    GtkWidget *window = GTK_WIDGET(data->window);
    gtk_window_close(GTK_WINDOW(window));
    
    GameData data_start;
    create_game_interface_start(&data_start);
    gtk_main();
}



void repositioning_flot(GtkWidget *widget, GameData *data) {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			grid1[i][j] = 0;
			grid_ships1[i][j] = 0;
			grid_ships2[i][j] = 0;
		}
	}
	for (int i = 0; i < GRID_SIZE; i++) {
		ships1[i].x = 0;
		ships1[i].y = 0;
		ships2[i].x = 0;
		ships2[i].y = 0;
		ships1[i].horizontal = 1;
		ships2[i].horizontal = 1;		
	}
	for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
                data->player1_grid[x][y].is_hit = FALSE;
                data->player1_grid[x][y].is_shot = FALSE;
                data->player1_grid[x][y].is_ship = FALSE;
				data->player2_grid[x][y].is_hit = FALSE;
                data->player2_grid[x][y].is_shot = FALSE;
                data->player2_grid[x][y].is_ship = FALSE;
                gtk_button_set_image(GTK_BUTTON(data->player1_grid[x][y].button), NULL);
                gtk_button_set_label(GTK_BUTTON(data->player2_grid[x][y].button), "");
        }
    }
	
    place_ships1(data);
    place_ships2(data);
    flot();
    visualize_ships(data);
}
/**************************************************          END FUNCTION TO EXIT          **************************************************/




/**************************************************          FUNCTION FOR DESIGN FOR LEVEL DIFFICULT          **************************************************/
void create_game_interface_difficult(GameData *data) {

    data->window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // gtk_window_set_default_size(GTK_WINDOW(data->window), 2000, 2000);
    // gtk_window_maximize(GTK_WINDOW(data->window));
    gtk_window_fullscreen(GTK_WINDOW(data->window));


    gtk_container_set_border_width(GTK_CONTAINER(data->window), 20);
    gtk_window_set_title(data->window, "🚀⛵💥   BATAILLE NAVALE   💥⛵🚀");


    // Positioning the window in the centre of the screen
    gtk_window_set_position(GTK_WINDOW(data->window), GTK_WIN_POS_CENTER);


    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(data->window), GTK_WIDGET(main_box));


    GtkWidget *button_box_up = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_up));


    GtkWidget *legend_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(legend_box));


    GtkWidget *grid_box = (GtkWidget*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(grid_box));


    GtkWidget *button_box_down = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_down));
    gtk_box_set_spacing(GTK_BOX(button_box_down), 250);


    GtkWidget *button_box_difficulty = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_difficulty));
    
    
    GtkWidget *legend_box_attention = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(legend_box_attention));


    GtkWidget *button_box_control = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_control));


    GtkWidget *legend_difficulty = gtk_label_new("↘️↘️   LE NIVEAU DE DIFFICULTE   ↙️↙️");
    // Obtaining a label style context
    GtkStyleContext *style_context1 = gtk_widget_get_style_context(legend_difficulty);
    // Use CSS to change color
    const char *css1 = "label { color: #FF7F50; font-size: 24px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider1, css1, -1, NULL);
    gtk_style_context_add_provider(style_context1, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider1);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), legend_difficulty, TRUE, TRUE, 1);
    
    data->legend_attention = gtk_label_new("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
    gtk_box_pack_start(GTK_BOX(legend_box_attention), data->legend_attention, TRUE, TRUE, 1);
    
	
    GtkWidget *legend_control = (GtkWidget*) gtk_label_new("🎬⬇️   GESTION DU JEU   ⬇️🎬");
    // Obtaining a label style context
    GtkStyleContext *style_context2 = gtk_widget_get_style_context(legend_control);
    // Use CSS to change color
    const char *css2 = "label { color: #FF7F50; font-size: 24px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider2, css2, -1, NULL);
    gtk_style_context_add_provider(style_context2, GTK_STYLE_PROVIDER(css_provider2), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider2);
    gtk_box_pack_start(GTK_BOX(button_box_control), legend_control, TRUE, TRUE, 1);


    GtkWidget *legend1 = (GtkWidget*) gtk_label_new("\n\t🇫🇷🇫🇷   SUP GALILEE MARINE NATIONALE   🇫🇷🇫🇷\t\t\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context3 = gtk_widget_get_style_context(legend1);
    // Use CSS to change color
    const char *css3 = "label { color: #FFF700; font-size: 22px; font-family: Arial Black; }";
    GtkCssProvider *css_provider3 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
    gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider3);
    gtk_box_pack_start(GTK_BOX(legend_box), legend1, FALSE, TRUE, 1);


    GtkWidget *legend2 = (GtkWidget*) gtk_label_new("\n\t\t\t\t\t\t\t\t\t\t🏴‍☠️🏴‍☠️   FLOTTE DE PIRATES   🏴‍☠️🏴‍☠️\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context4 = gtk_widget_get_style_context(legend2);
    // Use CSS to change color
    const char *css4 = "label { color: #000000; font-size: 22px; font-family: Arial Black; }";
    GtkCssProvider *css_provider4 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider4, css4, -1, NULL);
    gtk_style_context_add_provider(style_context4, GTK_STYLE_PROVIDER(css_provider4), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider4);
    gtk_box_pack_start(GTK_BOX(legend_box), legend2, TRUE, TRUE, 25);


    data->grid_player1 = create_grid_difficult(data, TRUE);
    gtk_box_pack_start(GTK_BOX(grid_box), data->grid_player1, TRUE, FALSE, 5);

    
    GtkWidget *image = gtk_image_new_from_file("preview.png");
    gtk_box_pack_start(GTK_BOX(grid_box), image, TRUE, TRUE, 0);


    data->grid_player2 = create_grid_difficult(data, FALSE);
    gtk_box_pack_start(GTK_BOX(grid_box), data->grid_player2, TRUE, FALSE, 5);
    

    GtkWidget *button_rules = gtk_button_new_with_label("📜📜   REGLES   📜📜");
    g_signal_connect(G_OBJECT(button_rules), "clicked", G_CALLBACK(show_text_window_rules), NULL);


    GtkWidget *button_trailer = gtk_button_new_with_label("👀👀   BANDE – ANNONCE   👀👀");
    g_signal_connect(G_OBJECT(button_trailer), "clicked", G_CALLBACK(show_text_window_trailer), NULL);

    
    data->button_easy = gtk_button_new_with_label("😜   FACILE   😜\n");
    // g_signal_connect(G_OBJECT(data->button_easy), "clicked", G_CALLBACK(start_game_easy), data);


    data->button_average = gtk_button_new_with_label("😐   MOYEN   😐\n");
    // g_signal_connect(G_OBJECT(data->button_average), "clicked", G_CALLBACK(start_game_average), data);


    data->button_difficult = gtk_button_new_with_label("👉👉👉   🤔  DIFFICILE  🤔   👈👈👈\n");
    // g_signal_connect(G_OBJECT(data->button_difficult), "clicked", G_CALLBACK(start_game_difficult), data);
    
    
    gtk_widget_set_sensitive(data->button_easy, FALSE);
    gtk_widget_set_sensitive(data->button_average, FALSE);
    gtk_widget_set_sensitive(data->button_difficult, FALSE);


    data->button_restart = gtk_button_new_with_label("🔄   REDEMARRER DU JEU   🔄\n");
    g_signal_connect(G_OBJECT(data->button_restart), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(data->button_restart), "clicked", G_CALLBACK(close_window2), data);


    
    data->button_replace_flot = gtk_button_new_with_label("🔃⚓   REPLACER LA FLOTTE   ⚓🔃\n");
    g_signal_connect(G_OBJECT(data->button_replace_flot), "clicked", G_CALLBACK(repositioning_flot), data);
    
    
    GtkWidget *button_exit = gtk_button_new_with_label("🚪🚪   SORTIR   🚪🚪\n");
    g_signal_connect(G_OBJECT(button_exit), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(button_exit), "clicked", G_CALLBACK(close_window), data->window);


    gtk_box_pack_start(GTK_BOX(button_box_up), button_rules, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_up), button_trailer, TRUE, TRUE, 10);

    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_easy, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_average, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_difficult, TRUE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(button_box_control), data->button_restart, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_control), data->button_replace_flot, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_control), button_exit, TRUE, FALSE, 10);
    

    g_signal_connect(G_OBJECT(data->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Creating Provider CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-image: url(\"background2.jpg\"); }", -1, NULL);
    
    // Obtaining a window style context and adding a CSS provider
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(data->window));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_widget_show_all(GTK_WIDGET(data->window));
}
/**************************************************          END FUNCTION FOR DESIGN FOR LEVEL DIFFICULT          **************************************************/




/**************************************************          FUNCTION FOR DESIGN FOR LEVEL AVERAGE          **************************************************/
void create_game_interface_average(GameData *data) {

    data->window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // gtk_window_set_default_size(GTK_WINDOW(data->window), 2000, 2000);
    // gtk_window_maximize(GTK_WINDOW(data->window));
    gtk_window_fullscreen(GTK_WINDOW(data->window));


    gtk_container_set_border_width(GTK_CONTAINER(data->window), 20);
    gtk_window_set_title(data->window, "🚀⛵💥   BATAILLE NAVALE   💥⛵🚀");


    // Positioning the window in the centre of the screen
    gtk_window_set_position(GTK_WINDOW(data->window), GTK_WIN_POS_CENTER);


    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(data->window), GTK_WIDGET(main_box));


    GtkWidget *button_box_up = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_up));


    GtkWidget *legend_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(legend_box));


    GtkWidget *grid_box = (GtkWidget*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(grid_box));


    GtkWidget *button_box_down = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_down));
    gtk_box_set_spacing(GTK_BOX(button_box_down), 250);


    GtkWidget *button_box_difficulty = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_difficulty));

    GtkWidget *legend_box_attention = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(legend_box_attention));


    GtkWidget *button_box_control = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_control));


    GtkWidget *legend_difficulty = gtk_label_new("↘️↘️   LE NIVEAU DE DIFFICULTE   ↙️↙️");
    // Obtaining a label style context
    GtkStyleContext *style_context1 = gtk_widget_get_style_context(legend_difficulty);
    // Use CSS to change color
    const char *css1 = "label { color: #FF7F50; font-size: 24px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider1, css1, -1, NULL);
    gtk_style_context_add_provider(style_context1, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider1);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), legend_difficulty, TRUE, TRUE, 1);


    data->legend_attention = gtk_label_new("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
    gtk_box_pack_start(GTK_BOX(legend_box_attention), data->legend_attention, TRUE, TRUE, 1);


    GtkWidget *legend_control = (GtkWidget*) gtk_label_new("🎬⬇️   GESTION DU JEU   ⬇️🎬");
    // Obtaining a label style context
    GtkStyleContext *style_context2 = gtk_widget_get_style_context(legend_control);
    // Use CSS to change color
    const char *css2 = "label { color: #FF7F50; font-size: 24px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider2, css2, -1, NULL);
    gtk_style_context_add_provider(style_context2, GTK_STYLE_PROVIDER(css_provider2), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider2);
    gtk_box_pack_start(GTK_BOX(button_box_control), legend_control, TRUE, TRUE, 1);


        GtkWidget *legend1 = (GtkWidget*) gtk_label_new("\n\t🇫🇷🇫🇷   SUP GALILEE MARINE NATIONALE   🇫🇷🇫🇷\t\t\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context3 = gtk_widget_get_style_context(legend1);
    // Use CSS to change color
    const char *css3 = "label { color: #FFF700; font-size: 22px; font-family: Arial Black; }";
    GtkCssProvider *css_provider3 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
    gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider3);
    gtk_box_pack_start(GTK_BOX(legend_box), legend1, FALSE, TRUE, 1);


    GtkWidget *legend2 = (GtkWidget*) gtk_label_new("\n\t\t\t\t\t\t\t\t\t\t🏴‍☠️🏴‍☠️   FLOTTE DE PIRATES   🏴‍☠️🏴‍☠️\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context4 = gtk_widget_get_style_context(legend2);
    // Use CSS to change color
    const char *css4 = "label { color: #000000; font-size: 22px; font-family: Arial Black; }";
    GtkCssProvider *css_provider4 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider4, css4, -1, NULL);
    gtk_style_context_add_provider(style_context4, GTK_STYLE_PROVIDER(css_provider4), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider4);
    gtk_box_pack_start(GTK_BOX(legend_box), legend2, TRUE, TRUE, 25);


    data->grid_player1 = create_grid_average(data, TRUE);
    gtk_box_pack_start(GTK_BOX(grid_box), data->grid_player1, TRUE, FALSE, 5);


    GtkWidget *image = gtk_image_new_from_file("preview.png");
    gtk_box_pack_start(GTK_BOX(grid_box), image, TRUE, TRUE, 0);


    data->grid_player2 = create_grid_average(data, FALSE);
    gtk_box_pack_start(GTK_BOX(grid_box), data->grid_player2, TRUE, FALSE, 5);
    

    GtkWidget *button_rules = gtk_button_new_with_label("📜📜   REGLES   📜📜");
    g_signal_connect(G_OBJECT(button_rules), "clicked", G_CALLBACK(show_text_window_rules), NULL);


    GtkWidget *button_trailer = gtk_button_new_with_label("👀👀   BANDE – ANNONCE   👀👀");
    g_signal_connect(G_OBJECT(button_trailer), "clicked", G_CALLBACK(show_text_window_trailer), NULL);

    
    data->button_easy = gtk_button_new_with_label("😜   FACILE   😜\n");
    // g_signal_connect(G_OBJECT(data->button_easy), "clicked", G_CALLBACK(start_game_easy), data);


    data->button_average = gtk_button_new_with_label("👉👉👉   😐  MOYEN  😐   👈👈👈\n");
    // g_signal_connect(G_OBJECT(data->button_average), "clicked", G_CALLBACK(start_game_average), data);


    data->button_difficult = gtk_button_new_with_label("🤔   DIFFICILE   🤔\n");
    // g_signal_connect(G_OBJECT(data->button_difficult), "clicked", G_CALLBACK(start_game_difficult), data);
    
    
    gtk_widget_set_sensitive(data->button_easy, FALSE);
    gtk_widget_set_sensitive(data->button_average, FALSE);
    gtk_widget_set_sensitive(data->button_difficult, FALSE);


    data->button_restart = gtk_button_new_with_label("🔄   REDEMARRER DU JEU   🔄\n");
    g_signal_connect(G_OBJECT(data->button_restart), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(data->button_restart), "clicked", G_CALLBACK(close_window2), data);

    
    data->button_replace_flot = gtk_button_new_with_label("🔃⚓   REPLACER LA FLOTTE   ⚓🔃\n");
    g_signal_connect(G_OBJECT(data->button_replace_flot), "clicked", G_CALLBACK(repositioning_flot), data);
    
    
	GtkWidget *button_exit = gtk_button_new_with_label("🚪🚪   SORTIR   🚪🚪\n");
    g_signal_connect(G_OBJECT(button_exit), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(button_exit), "clicked", G_CALLBACK(close_window), data->window);


    gtk_box_pack_start(GTK_BOX(button_box_up), button_rules, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_up), button_trailer, TRUE, TRUE, 10);

    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_easy, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_average, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_difficult, TRUE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(button_box_control), data->button_restart, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_control), data->button_replace_flot, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_control), button_exit, TRUE, FALSE, 10);
    

    g_signal_connect(G_OBJECT(data->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Creating Provider CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-image: url(\"background2.jpg\"); }", -1, NULL);
    
    // Obtaining a window style context and adding a CSS provider
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(data->window));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_widget_show_all(GTK_WIDGET(data->window));
}
/**************************************************          END FUNCTION FOR DESIGN FOR LEVEL AVERAGE          **************************************************/




/**************************************************          FUNCTION FOR DESIGN FOR LEVEL EASY          **************************************************/
void create_game_interface_easy(GameData *data) {

    data->window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // gtk_window_set_default_size(GTK_WINDOW(data->window), 2000, 2000);
    // gtk_window_maximize(GTK_WINDOW(data->window));
    gtk_window_fullscreen(GTK_WINDOW(data->window));


    gtk_container_set_border_width(GTK_CONTAINER(data->window), 20);
    gtk_window_set_title(data->window, "🚀⛵💥   BATAILLE NAVALE   💥⛵🚀");


    // Positioning the window in the centre of the screen
    gtk_window_set_position(GTK_WINDOW(data->window), GTK_WIN_POS_CENTER);


    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(data->window), GTK_WIDGET(main_box));


    GtkWidget *button_box_up = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_up));


    GtkWidget *legend_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(legend_box));


    GtkWidget *grid_box = (GtkWidget*)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(grid_box));


    GtkWidget *button_box_down = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_down));
    gtk_box_set_spacing(GTK_BOX(button_box_down), 250);


    GtkWidget *button_box_difficulty = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_difficulty));

    GtkWidget *legend_box_attention = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(legend_box_attention));


    GtkWidget *button_box_control = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_control));


    GtkWidget *legend_difficulty = gtk_label_new("↘️↘️   LE NIVEAU DE DIFFICULTE   ↙️↙️");
    // Obtaining a label style context
    GtkStyleContext *style_context1 = gtk_widget_get_style_context(legend_difficulty);
    // Use CSS to change color
    const char *css1 = "label { color: #FF7F50; font-size: 24px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider1, css1, -1, NULL);
    gtk_style_context_add_provider(style_context1, GTK_STYLE_PROVIDER(css_provider1), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider1);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), legend_difficulty, TRUE, TRUE, 1);


    data->legend_attention = gtk_label_new("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
    gtk_box_pack_start(GTK_BOX(legend_box_attention), data->legend_attention, TRUE, TRUE, 1);


    GtkWidget *legend_control = (GtkWidget*) gtk_label_new("🎬⬇️   GESTION DU JEU   ⬇️🎬");
    // Obtaining a label style context
    GtkStyleContext *style_context2 = gtk_widget_get_style_context(legend_control);
    // Use CSS to change color
    const char *css2 = "label { color: #FF7F50; font-size: 24px; font-family: Helvetica; font-weight: bold; }";
    GtkCssProvider *css_provider2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider2, css2, -1, NULL);
    gtk_style_context_add_provider(style_context2, GTK_STYLE_PROVIDER(css_provider2), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider2);
    gtk_box_pack_start(GTK_BOX(button_box_control), legend_control, TRUE, TRUE, 1);


    GtkWidget *legend1 = (GtkWidget*) gtk_label_new("\n\t🇫🇷🇫🇷   SUP GALILEE MARINE NATIONALE   🇫🇷🇫🇷\t\t\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context3 = gtk_widget_get_style_context(legend1);
    // Use CSS to change color
    const char *css3 = "label { color: #FFF700; font-size: 22px; font-family: Arial Black; }";
    GtkCssProvider *css_provider3 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider3, css3, -1, NULL);
    gtk_style_context_add_provider(style_context3, GTK_STYLE_PROVIDER(css_provider3), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider3);
    gtk_box_pack_start(GTK_BOX(legend_box), legend1, FALSE, TRUE, 1);


    GtkWidget *legend2 = (GtkWidget*) gtk_label_new("\n\t\t\t\t\t\t\t\t\t\t🏴‍☠️🏴‍☠️   FLOTTE DE PIRATES   🏴‍☠️🏴‍☠️\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context4 = gtk_widget_get_style_context(legend2);
    // Use CSS to change color
    const char *css4 = "label { color: #000000; font-size: 22px; font-family: Arial Black; }";
    GtkCssProvider *css_provider4 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider4, css4, -1, NULL);
    gtk_style_context_add_provider(style_context4, GTK_STYLE_PROVIDER(css_provider4), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider4);
    gtk_box_pack_start(GTK_BOX(legend_box), legend2, TRUE, TRUE, 25);


    data->grid_player1 = create_grid_easy(data, TRUE);
    gtk_box_pack_start(GTK_BOX(grid_box), data->grid_player1, TRUE, FALSE, 5);


    GtkWidget *image = gtk_image_new_from_file("preview.png");
    gtk_box_pack_start(GTK_BOX(grid_box), image, TRUE, TRUE, 0);


    data->grid_player2 = create_grid_easy(data, FALSE);
    gtk_box_pack_start(GTK_BOX(grid_box), data->grid_player2, TRUE, FALSE, 5);
    

    GtkWidget *button_rules = gtk_button_new_with_label("📜📜   REGLES   📜📜");
    g_signal_connect(G_OBJECT(button_rules), "clicked", G_CALLBACK(show_text_window_rules), NULL);


    GtkWidget *button_trailer = gtk_button_new_with_label("👀👀   BANDE – ANNONCE   👀👀");
    g_signal_connect(G_OBJECT(button_trailer), "clicked", G_CALLBACK(show_text_window_trailer), NULL);

    
    data->button_easy = gtk_button_new_with_label("👉👉👉   😜  FACILE  😜   👈👈👈\n");
    // g_signal_connect(G_OBJECT(data->button_easy), "clicked", G_CALLBACK(start_game_easy), data);


    data->button_average = gtk_button_new_with_label("😐   MOYEN   😐\n");
    // g_signal_connect(G_OBJECT(data->button_average), "clicked", G_CALLBACK(start_game_average), data);


    data->button_difficult = gtk_button_new_with_label("🤔   DIFFICILE   🤔\n");
    // g_signal_connect(G_OBJECT(data->button_difficult), "clicked", G_CALLBACK(start_game_difficult), data);
    
    
    gtk_widget_set_sensitive(data->button_easy, FALSE);
    gtk_widget_set_sensitive(data->button_average, FALSE);
    gtk_widget_set_sensitive(data->button_difficult, FALSE);


    data->button_restart = gtk_button_new_with_label("🔄   REDEMARRER DU JEU   🔄\n");
    g_signal_connect(G_OBJECT(data->button_restart), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(data->button_restart), "clicked", G_CALLBACK(close_window2), data);

    
    data->button_replace_flot = gtk_button_new_with_label("🔃⚓   REPLACER LA FLOTTE   ⚓🔃\n");
    g_signal_connect(G_OBJECT(data->button_replace_flot), "clicked", G_CALLBACK(repositioning_flot), data);
    
    
	GtkWidget *button_exit = gtk_button_new_with_label("🚪🚪   SORTIR   🚪🚪\n");
    g_signal_connect(G_OBJECT(button_exit), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(button_exit), "clicked", G_CALLBACK(close_window), data->window);


    gtk_box_pack_start(GTK_BOX(button_box_up), button_rules, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_up), button_trailer, TRUE, TRUE, 10);

    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_easy, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_average, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_difficult, TRUE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(button_box_control), data->button_restart, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_control), data->button_replace_flot, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(button_box_control), button_exit, TRUE, FALSE, 10);
    

    g_signal_connect(G_OBJECT(data->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Creating Provider CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-image: url(\"background2.jpg\"); }", -1, NULL);
    
    // Obtaining a window style context and adding a CSS provider
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(data->window));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_widget_show_all(GTK_WIDGET(data->window));
}
/**************************************************          END FUNCTION FOR DESIGN FOR LEVEL EASY          **************************************************/



/**************************************************          FUNCTION TO CONTROLE LEVEL OF THE GAME          **************************************************/
void start_game_difficult() {

	GameData data;
    data.player1_ships_left = NUM_DECKS;
    data.player2_ships_left = NUM_DECKS;
    data.is_player1_turn = TRUE;
 
    create_game_interface_difficult(&data);
    place_ships1(&data);
    place_ships2(&data);
    flot();
    visualize_ships(&data);
    
    gtk_main();
}


void start_game_average() {

	GameData data;
    data.player1_ships_left = NUM_DECKS;
    data.player2_ships_left = NUM_DECKS;
    data.is_player1_turn = TRUE;
 
    create_game_interface_average(&data);
    place_ships1(&data);
    place_ships2(&data);
    flot();
    visualize_ships(&data);
    
    gtk_main();
}


void start_game_easy() {

	GameData data;
    data.player1_ships_left = NUM_DECKS;
    data.player2_ships_left = NUM_DECKS;
    data.is_player1_turn = TRUE;
 
    create_game_interface_easy(&data);
    place_ships1(&data);
    place_ships2(&data);
    flot();
    visualize_ships(&data);
    
    gtk_main();
}


void on_button_difficult_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_widget_hide(window);
    start_game_difficult();
}


void on_button_average_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_widget_hide(window);
    start_game_average();
}


void on_button_easy_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_widget_hide(window);
    start_game_easy();
}
/**************************************************          END FUNCTION TO CONTROLE LEVEL OF THE GAME          **************************************************/





/**************************************************          FUNCTION FOR DESIGN FOR START "MENU"          **************************************************/
void create_game_interface_start(GameData *data) {

    GtkWidget *window_start = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window_start), 800, 400);
    // gtk_window_maximize(GTK_WINDOW(data->window));
    // gtk_window_fullscreen(GTK_WINDOW(data->window));


    gtk_container_set_border_width(GTK_CONTAINER(window_start), 40);
    gtk_window_set_title(GTK_WINDOW(window_start), "🚀⛵💥  BATAILLE NAVALE  💥⛵🚀");


    // Positioning the window in the centre of the screen
    gtk_window_set_position(GTK_WINDOW(window_start), GTK_WIN_POS_CENTER);


    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_container_add(GTK_CONTAINER(window_start), GTK_WIDGET(main_box));


    GtkWidget *button_box_up = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_up));
    
    
    GtkWidget *button_box_legend = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_legend));
    
    
    GtkWidget *button_box_down = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(main_box), GTK_WIDGET(button_box_down));


    GtkWidget *button_box_difficulty = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 175);
    gtk_container_add(GTK_CONTAINER(button_box_down), GTK_WIDGET(button_box_difficulty));


    GtkWidget *legend_turn;
    legend_turn = (GtkWidget*)gtk_label_new("\n\n⬇️▶️  CHOISIR LE NIVEAU  ▶️⬇️\n\n");
    // Obtaining a label style context
    GtkStyleContext *style_context = gtk_widget_get_style_context(legend_turn);
    // Use CSS to change color
    const char *css = "label { color: #FF9700; font-size: 23px; font-family: Arial Black; }";
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, css, -1, NULL);
    gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css_provider);
    gtk_box_pack_start(GTK_BOX(button_box_legend), legend_turn, TRUE, TRUE, 1);


    GtkWidget *button_rules = gtk_button_new_with_label("📜  REGLES  📜");
    g_signal_connect(G_OBJECT(button_rules), "clicked", G_CALLBACK(show_text_window_rules), NULL);


    GtkWidget *button_trailer = gtk_button_new_with_label("👀  BANDE – ANNONCE  👀");
    g_signal_connect(G_OBJECT(button_trailer), "clicked", G_CALLBACK(show_text_window_trailer), NULL);

    
    data->button_easy = gtk_button_new_with_label("😜  FACILE  😜");
    g_signal_connect(G_OBJECT(data->button_easy), "clicked", G_CALLBACK(on_button_easy_clicked), window_start);


    data->button_average = gtk_button_new_with_label("😐  MOYEN  😐");
    g_signal_connect(G_OBJECT(data->button_average), "clicked", G_CALLBACK(on_button_average_clicked), window_start);


    data->button_difficult = gtk_button_new_with_label("🤔  DIFFICILE  🤔");
    g_signal_connect(G_OBJECT(data->button_difficult), "clicked", G_CALLBACK(on_button_difficult_clicked), window_start);


    gtk_box_pack_start(GTK_BOX(button_box_up), button_rules, TRUE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box_up), button_trailer, TRUE, FALSE, 5);


    /*gtk_widget_override_color(GTK_BUTTON(data->button_difficult), GTK_STATE_FLAG_NORMAL, &(GdkRGBA){1, 0, 0, 0});*/
    
    /*GtkStyleContext *context_button_easy = gtk_widget_get_style_context(data->button_difficult);
    GdkRGBA color;
    gdk_rgba_parse(&color, "red");
    gtk_style_context_set_color(context_button_easy, &color);*/


    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_easy, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_average, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(button_box_difficulty), data->button_difficult, TRUE, FALSE, 1);
    

    g_signal_connect(G_OBJECT(window_start), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Creating Provider CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "window { background-image: url(\"background1.png\"); }", -1, NULL);
    
    // Obtaining a window style context and adding a CSS provider
    GtkStyleContext *context = gtk_widget_get_style_context(window_start);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_widget_show_all(GTK_WIDGET(window_start));
}
/**************************************************          END FUNCTION FOR DESIGN FOR START "MENU"          **************************************************/



/************************************************************          END DEFINITION FUNCTIONS          ************************************************************/





/************************************************************          MAIN          ************************************************************/
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL));
    
    GameData data_start;
    create_game_interface_start(&data_start);

    gtk_main();

    return EXIT_SUCCESS;
}
/************************************************************          END MAIN          *********************************************************/
