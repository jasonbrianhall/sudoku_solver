#include <gtk/gtk.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cstring>
#include <ctime>
#include <functional>

// Include the existing Sudoku implementation
#include "sudoku.h"
#include "generatepuzzle.h"

// Game application structure
struct SudokuApp {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *status_label;
    GtkWidget *difficulty_label;
    GtkWidget *time_label;
    GtkWidget *hint_button;
    GtkWidget *check_button;
    std::vector<std::vector<GtkWidget*>> buttons;
    GtkWidget *currently_focused_button;
    Sudoku *game;
    PuzzleGenerator *generator;
    std::string current_difficulty;
    
    // UI state tracking
    bool game_started;
    guint timer_id;
    int seconds_elapsed;
    
    // Original puzzle for checking
    int original_cells[9][9];
};

// Structure to store button data
struct ButtonData {
    int x;
    int y;
    bool original; // If true, this was a clue cell and cannot be modified
};

// Map to store button data with widget pointers as keys
std::map<GtkWidget*, ButtonData*> button_data;

// Forward declarations
static void update_display(SudokuApp *app);
static void apply_css_to_widget(GtkWidget *widget, const char *css_class);
static void mark_original_cells(SudokuApp *app);
static void start_new_game(SudokuApp *app, const char *difficulty);
static void update_timer(SudokuApp *app);
static gboolean timer_callback(gpointer user_data);
static void provide_hint(SudokuApp *app);
static void check_solution(SudokuApp *app);
static void on_cell_clicked(GtkWidget *widget, gpointer user_data);
static gboolean on_cell_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static gboolean on_cell_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
static gboolean on_cell_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
static void new_game_difficulty(GtkWidget *widget, gpointer user_data);
static void save_game(GtkWidget *widget, gpointer user_data);
static void load_game(GtkWidget *widget, gpointer user_data);
static void export_to_excel(GtkWidget *widget, gpointer user_data);
static void on_hint_button_clicked(GtkWidget *widget, gpointer user_data);
static void on_check_button_clicked(GtkWidget *widget, gpointer user_data);
static void copy_board_callback(GtkWidget *widget, gpointer user_data);
static void hint_callback(GtkWidget *widget, gpointer user_data);
static void check_solution_callback(GtkWidget *widget, gpointer user_data);

static void set_button_focus(SudokuApp *app, int x, int y);
