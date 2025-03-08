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

// Helper function to apply CSS classes to widgets
static void apply_css_to_widget(GtkWidget *widget, const char *css_class) {
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, css_class);
}

// Update the display based on the current game state
static void update_display(SudokuApp *app) {
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            int value = app->game->GetValue(x, y);
            GtkWidget *button = app->buttons[y][x];
            ButtonData *data = button_data[button];
            
            if (value >= 0 && value <= 8) {
                gtk_button_set_label(GTK_BUTTON(button), std::to_string(value + 1).c_str());
            } else {
                gtk_button_set_label(GTK_BUTTON(button), "");
            }
            
            // Apply styling for original cells vs. user-entered values
            GtkStyleContext *context = gtk_widget_get_style_context(button);
            if (data->original) {
                gtk_style_context_add_class(context, "original-cell");
                gtk_style_context_remove_class(context, "user-cell");
            } else {
                gtk_style_context_add_class(context, "user-cell");
                gtk_style_context_remove_class(context, "original-cell");
            }
        }
    }
}

// Mark cells that were originally part of the puzzle (clues)
static void mark_original_cells(SudokuApp *app) {
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            int value = app->game->GetValue(x, y);
            app->original_cells[y][x] = value;  // Store original state
            
            GtkWidget *button = app->buttons[y][x];
            ButtonData *data = button_data[button];
            data->original = (value != -1);  // If the cell has a value, it's original
            
            // Apply styling
            GtkStyleContext *context = gtk_widget_get_style_context(button);
            if (data->original) {
                gtk_style_context_add_class(context, "original-cell");
                gtk_style_context_remove_class(context, "user-cell");
            } else {
                gtk_style_context_add_class(context, "user-cell");
                gtk_style_context_remove_class(context, "original-cell");
            }
        }
    }
}

// Update the timer display
static void update_timer(SudokuApp *app) {
    int hours = app->seconds_elapsed / 3600;
    int minutes = (app->seconds_elapsed % 3600) / 60;
    int seconds = app->seconds_elapsed % 60;
    
    char time_text[20];
    snprintf(time_text, sizeof(time_text), "Time: %02d:%02d:%02d", hours, minutes, seconds);
    gtk_label_set_text(GTK_LABEL(app->time_label), time_text);
}

// Timer callback function
static gboolean timer_callback(gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    app->seconds_elapsed++;
    update_timer(app);
    
    return G_SOURCE_CONTINUE;  // Continue the timer
}

// Cell click handler
static void on_cell_clicked(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    ButtonData *data = button_data[widget];
    
    // Don't allow changes to original clue cells
    if (data->original) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "This cell is part of the puzzle and cannot be changed");
        return;
    }
    
    // Get current value and increment it
    int current_value = app->game->GetValue(data->x, data->y);
    
    // Cycle through values: -1 (empty) -> 0 -> 1 -> ... -> 8 -> -1
    if (current_value == 8) {
        current_value = -1;  // Clear the cell
        app->game->ClearValue(data->x, data->y);
    } else {
        current_value++;
        app->game->SetValue(data->x, data->y, current_value);
    }
    
    // Update the display
    update_display(app);
    
    // Clear any previous status message
    if (strcmp(gtk_label_get_text(GTK_LABEL(app->status_label)), "This cell is part of the puzzle and cannot be changed") == 0) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "");
    }
}

// Keyboard event handler for individual cells
static gboolean on_cell_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    ButtonData *data = button_data[widget];
    
    // Don't allow changes to original clue cells
    if (data->original) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "This cell is part of the puzzle and cannot be changed");
        return TRUE;
    }
    
    // Handle number keys 1-9
    if (event->keyval >= GDK_KEY_1 && event->keyval <= GDK_KEY_9) {
        int value = event->keyval - GDK_KEY_1;  // 0-8 for internal representation
        app->game->SetValue(data->x, data->y, value);
        update_display(app);
        gtk_label_set_text(GTK_LABEL(app->status_label), "");
        return TRUE;
    }
    
    // Handle backspace or delete to clear cell
    if (event->keyval == GDK_KEY_BackSpace || event->keyval == GDK_KEY_Delete) {
        app->game->ClearValue(data->x, data->y);
        update_display(app);
        gtk_label_set_text(GTK_LABEL(app->status_label), "");
        return TRUE;
    }
    
    return FALSE;  // Let other handlers process the event
}

// Scroll event handler for cells
static gboolean on_cell_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    ButtonData *data = button_data[widget];
    
    // Don't allow changes to original clue cells
    if (data->original) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "This cell is part of the puzzle and cannot be changed");
        return TRUE;
    }
    
    int current_value = app->game->GetValue(data->x, data->y);
    
    if (event->direction == GDK_SCROLL_UP) {
        // Increment value on scroll up
        if (current_value == 8) {
            app->game->ClearValue(data->x, data->y);
        } else if (current_value == -1) {
            app->game->SetValue(data->x, data->y, 0);
        } else {
            app->game->SetValue(data->x, data->y, current_value + 1);
        }
    } else if (event->direction == GDK_SCROLL_DOWN) {
        // Decrement value on scroll down
        if (current_value == -1) {
            app->game->SetValue(data->x, data->y, 8);
        } else if (current_value == 0) {
            app->game->ClearValue(data->x, data->y);
        } else {
            app->game->SetValue(data->x, data->y, current_value - 1);
        }
    }
    
    update_display(app);
    gtk_label_set_text(GTK_LABEL(app->status_label), "");
    return TRUE;  // Event handled
}

// Mouse button press handler for cells
static gboolean on_cell_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    ButtonData *data = button_data[widget];
    
    // Don't allow changes to original clue cells
    if (data->original) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "This cell is part of the puzzle and cannot be changed");
        return TRUE;
    }
    
    // Handle right-click to decrement value
    if (event->button == 3) {  // Right button
        int current_value = app->game->GetValue(data->x, data->y);
        
        if (current_value == -1) {
            app->game->SetValue(data->x, data->y, 8);
        } else if (current_value == 0) {
            app->game->ClearValue(data->x, data->y);
        } else {
            app->game->SetValue(data->x, data->y, current_value - 1);
        }
        
        update_display(app);
        gtk_label_set_text(GTK_LABEL(app->status_label), "");
        return TRUE;  // Event handled
    }
    
    // Handle middle-click to clear cell
    if (event->button == 2) {  // Middle button
        app->game->ClearValue(data->x, data->y);
        update_display(app);
        gtk_label_set_text(GTK_LABEL(app->status_label), "");
        return TRUE;  // Event handled
    }
    
    return FALSE;  // Let other handlers process the event
}

// Global keyboard handler for function keys and shortcuts
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    // Handle function keys for generating puzzles of different difficulties
    if (event->keyval == GDK_KEY_F1) {
        if ((event->state & GDK_SHIFT_MASK) != 0) {
            start_new_game(app, "extreme");
        } else {
            start_new_game(app, "easy");
        }
        return TRUE;
    } else if (event->keyval == GDK_KEY_F2) {
        start_new_game(app, "medium");
        return TRUE;
    } else if (event->keyval == GDK_KEY_F3) {
        start_new_game(app, "hard");
        return TRUE;
    } else if (event->keyval == GDK_KEY_F4) {
        start_new_game(app, "expert");
        return TRUE;
    }
    
    // Handle Ctrl+C for copying the board
    if (event->keyval == GDK_KEY_c && (event->state & GDK_CONTROL_MASK) != 0) {
        // Create plain text content
        std::string plain_text = "Sudoku Puzzle\n";
        for (int y = 0; y < 9; y++) {
            if (y % 3 == 0) {
                plain_text += "+-----+-----+-----+\n";
            }
            for (int x = 0; x < 9; x++) {
                if (x % 3 == 0) {
                    plain_text += "|";
                }
                int value = app->game->GetValue(x, y);
                if (value >= 0 && value <= 8) {
                    plain_text += " " + std::to_string(value + 1) + " ";
                } else {
                    plain_text += " . ";
                }
            }
            plain_text += "|\n";
        }
        plain_text += "+-----+-----+-----+\n";
        
        // Copy to clipboard
        GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        gtk_clipboard_set_text(clipboard, plain_text.c_str(), -1);
        
        // Update status
        gtk_label_set_text(GTK_LABEL(app->status_label), "Board copied to clipboard");
        return TRUE;
    }
    
    // Handle Ctrl+H for hint
    if (event->keyval == GDK_KEY_h && (event->state & GDK_CONTROL_MASK) != 0) {
        provide_hint(app);
        return TRUE;
    }
    
    // Handle Ctrl+K to check solution
    if (event->keyval == GDK_KEY_k && (event->state & GDK_CONTROL_MASK) != 0) {
        check_solution(app);
        return TRUE;
    }
    
    return FALSE;  // Let other handlers process the event
}

// Starts a new game with the specified difficulty
static void start_new_game(SudokuApp *app, const char *difficulty) {
    // Stop any existing timer
    if (app->timer_id != 0) {
        g_source_remove(app->timer_id);
        app->timer_id = 0;
    }
    
    // Generate a new puzzle
    app->generator->generatePuzzle(difficulty);
    app->game->Clean();
    app->current_difficulty = difficulty;
    
    // Mark original cells
    mark_original_cells(app);
    
    // Update the display
    update_display(app);
    
    // Reset timer
    app->seconds_elapsed = 0;
    update_timer(app);
    app->timer_id = g_timeout_add(1000, timer_callback, app);  // Update every second
    
    // Update difficulty label
    std::string diff_text = "Difficulty: ";
    diff_text += difficulty;
    gtk_label_set_text(GTK_LABEL(app->difficulty_label), diff_text.c_str());
    
    // Clear status
    gtk_label_set_text(GTK_LABEL(app->status_label), "");
    
    // Game is now started
    app->game_started = true;
}

// Provide a hint to the player
static void provide_hint(SudokuApp *app) {
    if (!app->game_started) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Start a new game first!");
        return;
    }
    
    // Make a backup of the current board
    int backup[9][9][9];
    memcpy(backup, app->game->board, sizeof(backup));
    
    // Try to solve the puzzle
    if (app->game->Solve() == 0) {
        // Find an empty cell or a cell with an incorrect value
        bool hint_provided = false;
        
        for (int y = 0; y < 9 && !hint_provided; y++) {
            for (int x = 0; x < 9 && !hint_provided; x++) {
                ButtonData *data = button_data[app->buttons[y][x]];
                
                // Skip original cells
                if (data->original) continue;
                
                int current_value = app->game->GetValue(x, y);
                
                // If the cell is empty, provide a hint
                if (current_value == -1) {
                    // Restore the backup except for this one cell
                    memcpy(app->game->board, backup, sizeof(backup));
                    int solution_value = app->game->GetValue(x, y);
                    app->game->SetValue(x, y, solution_value);
                    
                    // Update display for just this cell
                    update_display(app);
                    
                    // Mark the cell as now original so it can't be changed
                    data->original = true;
                    gtk_style_context_add_class(gtk_widget_get_style_context(app->buttons[y][x]), "hint-cell");
                    
                    gtk_label_set_text(GTK_LABEL(app->status_label), "Hint provided");
                    hint_provided = true;
                    break;
                }
            }
        }
        
        if (!hint_provided) {
            // All cells are filled, check if any are incorrect
            for (int y = 0; y < 9 && !hint_provided; y++) {
                for (int x = 0; x < 9 && !hint_provided; x++) {
                    ButtonData *data = button_data[app->buttons[y][x]];
                    
                    // Skip original cells
                    if (data->original) continue;
                    
                    int current_value = backup[y][x][0];  // Get value from backup
                    int solution_value = app->game->GetValue(x, y);
                    
                    if (current_value != solution_value) {
                        // Restore the backup except for this one cell
                        memcpy(app->game->board, backup, sizeof(backup));
                        app->game->SetValue(x, y, solution_value);
                        
                        // Update display for just this cell
                        update_display(app);
                        
                        // Mark the cell as now original so it can't be changed
                        data->original = true;
                        gtk_style_context_add_class(gtk_widget_get_style_context(app->buttons[y][x]), "hint-cell");
                        
                        gtk_label_set_text(GTK_LABEL(app->status_label), "Corrected a wrong entry");
                        hint_provided = true;
                        break;
                    }
                }
            }
            
            if (!hint_provided) {
                // All cells are correct
                gtk_label_set_text(GTK_LABEL(app->status_label), "Your solution is already correct!");
                // Restore the backup
                memcpy(app->game->board, backup, sizeof(backup));
            }
        }
    } else {
        // Could not solve the puzzle
        gtk_label_set_text(GTK_LABEL(app->status_label), "Could not solve the puzzle from its current state");
        // Restore the backup
        memcpy(app->game->board, backup, sizeof(backup));
    }
}

// Check if the current solution is correct
static void check_solution(SudokuApp *app) {
    if (!app->game_started) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Start a new game first!");
        return;
    }
    
    // Check if all cells are filled
    bool all_filled = true;
    for (int y = 0; y < 9 && all_filled; y++) {
        for (int x = 0; x < 9; x++) {
            if (app->game->GetValue(x, y) == -1) {
                all_filled = false;
                break;
            }
        }
    }
    
    if (!all_filled) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "The puzzle is not complete yet");
        return;
    }
    
    // Check if the solution is valid
    if (app->game->IsValidSolution()) {
        // Stop the timer
        if (app->timer_id != 0) {
            g_source_remove(app->timer_id);
            app->timer_id = 0;
        }
        
        // Create success message with time
        int hours = app->seconds_elapsed / 3600;
        int minutes = (app->seconds_elapsed % 3600) / 60;
        int seconds = app->seconds_elapsed % 60;
        
        char message[100];
        snprintf(message, sizeof(message), 
                 "Congratulations! You solved the %s puzzle in %02d:%02d:%02d", 
                 app->current_difficulty.c_str(), hours, minutes, seconds);
        
        // Show a dialog for the win
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_INFO,
                                                  GTK_BUTTONS_OK,
                                                  "%s", message);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        // Update status
        gtk_label_set_text(GTK_LABEL(app->status_label), "Puzzle solved successfully!");
    } else {
        gtk_label_set_text(GTK_LABEL(app->status_label), "The solution is not correct. Keep trying!");
    }
}

// Menu callback for new game
static void new_game_difficulty(GtkWidget *widget, gpointer user_data) {
    const char *difficulty = static_cast<const char*>(user_data);
    SudokuApp *app = static_cast<SudokuApp*>(g_object_get_data(G_OBJECT(widget), "app"));
    
    start_new_game(app, difficulty);
}

// Menu callback for export to Excel
static void export_to_excel(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export to Excel XML",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Set up file filter for .xml files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Excel XML Files");
    gtk_file_filter_add_pattern(filter, "*.xml");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::string file_str(filename);
        
        // Append .xml extension if not present
        if (file_str.find(".xml") == std::string::npos) {
            file_str += ".xml";
        }
        
        app->game->ExportToExcelXML(file_str);
        gtk_label_set_text(GTK_LABEL(app->status_label), "Exported to Excel XML successfully");
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Menu callback for save game
static void save_game(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Game",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Set up file filter for .sud files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Sudoku Files");
    gtk_file_filter_add_pattern(filter, "*.sud");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::string file_str(filename);
        
        // Append .sud extension if not present
        if (file_str.find(".sud") == std::string::npos) {
            file_str += ".sud";
        }
        
        app->game->SaveToFile(file_str);
        gtk_label_set_text(GTK_LABEL(app->status_label), "Game saved successfully");
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Menu callback for load game
static void load_game(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Game",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Set up file filter for .sud files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Sudoku Files");
    gtk_file_filter_add_pattern(filter, "*.sud");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Stop existing timer
        if (app->timer_id != 0) {
            g_source_remove(app->timer_id);
            app->timer_id = 0;
        }
        
        // Load the game
        app->game->LoadFromFile(filename);
        g_free(filename);
        
        // Mark original cells - for loaded games, assume all cells are player editable
        for (int y = 0; y < 9; y++) {
            for (int x = 0; x < 9; x++) {
                GtkWidget *button = app->buttons[y][x];
                ButtonData *data = button_data[button];
                data->original = false;
                
                // Apply styling
                GtkStyleContext *context = gtk_widget_get_style_context(button);
                gtk_style_context_add_class(context, "user-cell");
                gtk_style_context_remove_class(context, "original-cell");
                gtk_style_context_remove_class(context, "hint-cell");
            }
        }
        
        // Update display
        update_display(app);
        
        // Reset timer and start it
        app->seconds_elapsed = 0;
        update_timer(app);
        app->timer_id = g_timeout_add(1000, timer_callback, app);
        
        // Set difficulty to unknown for loaded games
        gtk_label_set_text(GTK_LABEL(app->difficulty_label), "Difficulty: custom/loaded");
        
        // Update status
        gtk_label_set_text(GTK_LABEL(app->status_label), "Game loaded successfully");
        
        // Game is now started
        app->game_started = true;
    }
    
    gtk_widget_destroy(dialog);
}

// Hint button click handler
static void on_hint_button_clicked(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    provide_hint(app);
}

// Check button click handler
static void on_check_button_clicked(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    check_solution(app);
}

// Function to handle copy board callback
static void copy_board_callback(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    // Create plain text content
    std::string plain_text = "Sudoku Puzzle\n";
    for (int y = 0; y < 9; y++) {
        if (y % 3 == 0) {
            plain_text += "+-----+-----+-----+\n";
        }
        for (int x = 0; x < 9; x++) {
            if (x % 3 == 0) {
                plain_text += "|";
            }
            int value = app->game->GetValue(x, y);
            if (value >= 0 && value <= 8) {
                plain_text += " " + std::to_string(value + 1) + " ";
            } else {
                plain_text += " . ";
            }
        }
        plain_text += "|\n";
    }
    plain_text += "+-----+-----+-----+\n";
    
    // Copy to clipboard
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, plain_text.c_str(), -1);
    
    // Update status
    gtk_label_set_text(GTK_LABEL(app->status_label), "Board copied to clipboard");
}

// Function to handle hint callback
static void hint_callback(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    provide_hint(app);
}

// Function to handle check solution callback
static void check_solution_callback(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    check_solution(app);
}

// GTK application activation handler
static void activate(GtkApplication *gtk_app, gpointer user_data) {
    // Initialize application structure
    SudokuApp *app = new SudokuApp();
    app->game = new Sudoku();
    app->generator = new PuzzleGenerator(*app->game);
    app->current_difficulty = "medium";
    app->buttons.resize(9, std::vector<GtkWidget*>(9));
    app->game_started = false;
    app->timer_id = 0;
    app->seconds_elapsed = 0;
    
    // Set all original cells to empty initially
    memset(app->original_cells, -1, sizeof(app->original_cells));
    
    // Seed random number generator for puzzle generation
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Create main window
    GtkWidget *window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(window), "Sudoku Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 700);
    app->window = window;
    
    // Connect key press event for global shortcuts
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), app);
    
    // Create main vertical container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // Create menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(main_box), menubar, FALSE, FALSE, 0);
    
    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    
    // File menu items
    GtkWidget *new_game_item = gtk_menu_item_new_with_label("New Game");
    GtkWidget *new_game_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_game_item), new_game_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_game_item);
    
    // Add difficulty options to New Game submenu
    std::map<std::string, std::string> difficulties = {
        {"easy", "Easy (F1)"},
        {"medium", "Medium (F2)"},
        {"hard", "Hard (F3)"},
        {"expert", "Expert (F4)"},
        {"extreme", "Extreme (Shift+F1)"}
    };
    
    for (const auto& diff : difficulties) {
        GtkWidget *diff_item = gtk_menu_item_new_with_label(diff.second.c_str());
        // Store app pointer in the widget
        g_object_set_data(G_OBJECT(diff_item), "app", app);
        g_signal_connect(diff_item, "activate", G_CALLBACK(new_game_difficulty), (gpointer)diff.first.c_str());
        gtk_menu_shell_append(GTK_MENU_SHELL(new_game_menu), diff_item);
    }
    
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save Game");
    g_signal_connect(save_item, "activate", G_CALLBACK(save_game), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    
    GtkWidget *load_item = gtk_menu_item_new_with_label("Load Game");
    g_signal_connect(load_item, "activate", G_CALLBACK(load_game), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), load_item);
    
    GtkWidget *separator1 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), separator1);
    
    GtkWidget *excel_item = gtk_menu_item_new_with_label("Export to Excel XML...");
    g_signal_connect(excel_item, "activate", G_CALLBACK(export_to_excel), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), excel_item);
    
    GtkWidget *separator2 = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), separator2);
    
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect_swapped(quit_item, "activate", G_CALLBACK(gtk_widget_destroy), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    
    // Edit menu
    GtkWidget *edit_menu = gtk_menu_new();
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Edit");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_item), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit_item);
    
    // Copy board option
    GtkWidget *copy_item = gtk_menu_item_new_with_label("Copy Board (Ctrl+C)");
    g_signal_connect(copy_item, "activate", G_CALLBACK(copy_board_callback), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), copy_item);
    
    // Add hint option
    GtkWidget *hint_item = gtk_menu_item_new_with_label("Get Hint (Ctrl+H)");
    g_signal_connect(hint_item, "activate", G_CALLBACK(hint_callback), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), hint_item);
    
    // Add check solution option
    GtkWidget *check_item = gtk_menu_item_new_with_label("Check Solution (Ctrl+K)");
    g_signal_connect(check_item, "activate", G_CALLBACK(check_solution_callback), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), check_item);
    
    // Create info bar (difficulty, timer, etc.)
    GtkWidget *info_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), info_bar, FALSE, FALSE, 0);
    
    // Difficulty label
    app->difficulty_label = gtk_label_new("Difficulty: not started");
    gtk_label_set_xalign(GTK_LABEL(app->difficulty_label), 0.0);  // Left align
    gtk_box_pack_start(GTK_BOX(info_bar), app->difficulty_label, TRUE, TRUE, 5);
    
    // Timer label
    app->time_label = gtk_label_new("Time: 00:00:00");
    gtk_label_set_xalign(GTK_LABEL(app->time_label), 1.0);  // Right align
    gtk_box_pack_start(GTK_BOX(info_bar), app->time_label, TRUE, TRUE, 5);
    
    // Create button bar
    GtkWidget *button_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(main_box), button_bar, FALSE, FALSE, 0);
    
    // Hint button
    app->hint_button = gtk_button_new_with_label("Get Hint (Ctrl+H)");
    g_signal_connect(app->hint_button, "clicked", G_CALLBACK(on_hint_button_clicked), app);
    gtk_box_pack_start(GTK_BOX(button_bar), app->hint_button, TRUE, TRUE, 0);
    
    // Check solution button
    app->check_button = gtk_button_new_with_label("Check Solution (Ctrl+K)");
    g_signal_connect(app->check_button, "clicked", G_CALLBACK(on_check_button_clicked), app);
    gtk_box_pack_start(GTK_BOX(button_bar), app->check_button, TRUE, TRUE, 0);
    
    // Create frame for Sudoku grid
    GtkWidget *grid_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(grid_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(main_box), grid_frame, TRUE, TRUE, 0);
    
    // Set up CSS styling for the grid
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "button { border-radius: 0; border: 1px solid gray; min-width: 50px; min-height: 50px; font-size: 20px; }"
        ".left-border { border-left: 2px solid black; }"
        ".right-border { border-right: 2px solid black; }"
        ".top-border { border-top: 2px solid black; }"
        ".bottom-border { border-bottom: 2px solid black; }"
        ".error-text { color: red; }"
        ".original-cell { font-weight: bold; color: black; }"
        ".user-cell { color: blue; }"
        ".hint-cell { color: green; font-weight: bold; }";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
    
    // Create Sudoku grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_container_add(GTK_CONTAINER(grid_frame), grid);
    app->grid = grid;
    
    // Create buttons for each cell
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            GtkWidget *button = gtk_button_new_with_label("");
            
            // Apply CSS for styling
            GtkStyleContext *button_context = gtk_widget_get_style_context(button);
            gtk_style_context_add_class(button_context, "sudoku-cell");
            
            // Add button to grid
            gtk_grid_attach(GTK_GRID(grid), button, x, y, 1, 1);
            app->buttons[y][x] = button;
            
            // Store coordinates in button data
            ButtonData *data = new ButtonData{x, y, false};
            button_data[button] = data;
            
            // Connect signals
            g_signal_connect(button, "clicked", G_CALLBACK(on_cell_clicked), app);
            g_signal_connect(button, "key-press-event", G_CALLBACK(on_cell_key_press), app);
            g_signal_connect(button, "scroll-event", G_CALLBACK(on_cell_scroll), app);
            g_signal_connect(button, "button-press-event", G_CALLBACK(on_cell_button_press), app);
            
            // Apply CSS classes for borders
            if (x % 3 == 0) apply_css_to_widget(button, "left-border");
            if (x == 8) apply_css_to_widget(button, "right-border");
            if (y % 3 == 0) apply_css_to_widget(button, "top-border");
            if (y == 8) apply_css_to_widget(button, "bottom-border");
        }
    }
    
    // Status label
    app->status_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(app->status_label), TRUE);
    apply_css_to_widget(app->status_label, "error-text");
    gtk_box_pack_start(GTK_BOX(main_box), app->status_label, FALSE, FALSE, 5);
    
    // Instructions label
    GtkWidget *instructions = gtk_label_new(
        "Instructions: Click cells to cycle through numbers, or use keyboard (1-9). "
        "Right-click to decrement, middle-click to clear. "
        "Use F1-F4 for new puzzles of different difficulties."
    );
    gtk_label_set_line_wrap(GTK_LABEL(instructions), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), instructions, FALSE, FALSE, 5);
    
    // Show everything
    gtk_widget_show_all(window);
    
    // Start with a medium difficulty game
    start_new_game(app, "medium");
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.sudoku.game", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
