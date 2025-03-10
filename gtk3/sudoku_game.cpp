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
#include "sudoku_game.h"


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
    
    // Increase buffer size to ensure it can handle the maximum potential output size
    char time_text[30]; // Increased from 20 to 30 to be safe
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

    // Handle 0 key to clear cell (both regular and keypad 0)
    if (event->keyval == GDK_KEY_0 || event->keyval == GDK_KEY_KP_0) {
        app->game->ClearValue(data->x, data->y);
        update_display(app);
        gtk_label_set_text(GTK_LABEL(app->status_label), "Cell cleared");
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

static void show_about_dialog(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Sudoku Game");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.0");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "© Jason Brian Hall");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
        "A simple Sudoku game implementation with GTK+.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), 
        "https://github.com/jasonbrianhall/sudoku_solver");
    gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog), "GitHub Repository");
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_MIT_X11);
    const gchar* authors[] = {"Jason Brian Hall", NULL};
        gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);    
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Helper function to show the How to Play dialog
static void show_how_to_play(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "How to Play Sudoku",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    
    // Set dialog size
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    
    // Create a scrolled window for the content
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(content_area), scrolled_window, TRUE, TRUE, 0);
    
    // Create a text view for the instructions
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    
    // Get the buffer and add the text
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    const char *instructions = 
        "SUDOKU RULES:\n\n"
        "1. Each puzzle consists of a 9x9 grid containing cells that must be filled with numbers from 1 to 9.\n\n"
        "2. At the start of the game, the puzzle will have some cells already filled in. These are the clues.\n\n"
        "3. The objective is to fill in the empty cells so that:\n"
        "   • Each row contains all numbers from 1 to 9 without repetition\n"
        "   • Each column contains all numbers from 1 to 9 without repetition\n"
        "   • Each 3x3 box (outlined by thicker borders) contains all numbers from 1 to 9\n\n"
        "CONTROLS:\n\n"
        "• Mouse Controls:\n"
        "   - Left-click: Increase the number in a cell (cycles through 1-9 and empty)\n"
        "   - Right-click: Decrease the number in a cell\n"
        "   - Middle-click: Clear the cell\n"
        "   - Scroll wheel: Increase/decrease the number\n\n"
        "• Keyboard Controls:\n"
        "   - Arrow keys: Navigate between cells\n"
        "   - Tab/Shift+Tab: Navigate forward/backward through cells\n"
        "   - Home/End: Move to first/last cell in current row\n"
        "   - Page Up/Down: Move to top/bottom of current column\n"
        "   - Numbers 1-9: Enter that number in the current cell\n"
        "   - 0, Space, Delete, Backspace: Clear the current cell\n\n"
        "• Function Keys:\n"
        "   - F1: New easy puzzle\n"
        "   - F2: New medium puzzle\n"
        "   - F3: New hard puzzle\n"
        "   - F4: New expert puzzle\n"
        "   - Shift+F1: New extreme puzzle\n\n"
        "• Shortcuts:\n"
        "   - Ctrl+C: Copy the board to clipboard\n"
        "   - Ctrl+H: Get a hint (fills a random empty cell)\n"
        "   - Ctrl+K: Check your solution\n\n"
        "DIFFICULTY LEVELS:\n\n"
        "• Easy: For beginners, with many clues provided\n"
        "• Medium: A balanced challenge\n"
        "• Hard: Requires more advanced techniques\n"
        "• Expert: For experienced players\n"
        "• Extreme: The ultimate Sudoku challenge\n\n"
        "FEATURES:\n\n"
        "• Get Hint: Reveals a correct number in a random empty cell\n"
        "• Check Solution: Verifies if your current solution is correct\n"
        "• Save/Load: Save your progress and continue later\n"
        "• Export to Excel: Export your puzzle to Excel XML format\n\n"
        "Happy solving!";
    
    gtk_text_buffer_set_text(buffer, instructions, -1);
    
    // Set margins to make the text more readable
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 12);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 12);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 12);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view), 12);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
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
// Enhanced keyboard navigation for the Sudoku game
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    // If a specific cell has focus, handle arrow keys for navigation
    if (app->currently_focused_button != nullptr) {
        ButtonData *data = button_data[app->currently_focused_button];
        int current_x = data->x;
        int current_y = data->y;
        
        switch (event->keyval) {
            case GDK_KEY_Up:
            case GDK_KEY_KP_Up:
                set_button_focus(app, current_x, current_y - 1);
                return TRUE;
            
            case GDK_KEY_Down:
            case GDK_KEY_KP_Down:
                set_button_focus(app, current_x, current_y + 1);
                return TRUE;
                
            case GDK_KEY_Left:
            case GDK_KEY_KP_Left:
                // Move to previous cell, wrap to end of previous row if at start
                if (current_x > 0) {
                    set_button_focus(app, current_x - 1, current_y);
                } else if (current_y > 0) {
                    // Wrap to end of previous row
                    set_button_focus(app, 8, current_y - 1);
                } else {
                    // Wrap to bottom-right if at top-left
                    set_button_focus(app, 8, 8);
                }
                return TRUE;
                
            case GDK_KEY_Right:
            case GDK_KEY_KP_Right:
                // Move to next cell, wrap to start of next row if at end
                if (current_x < 8) {
                    set_button_focus(app, current_x + 1, current_y);
                } else if (current_y < 8) {
                    // Wrap to start of next row
                    set_button_focus(app, 0, current_y + 1);
                } else {
                    // Wrap to top-left if at bottom-right
                    set_button_focus(app, 0, 0);
                }
                return TRUE;
                
            case GDK_KEY_Tab:
                // Similar to right key but with Shift+Tab support
                if ((event->state & GDK_SHIFT_MASK) != 0) {
                    // Shift+Tab: move backward
                    if (current_x > 0) {
                        set_button_focus(app, current_x - 1, current_y);
                    } else if (current_y > 0) {
                        set_button_focus(app, 8, current_y - 1);
                    } else {
                        set_button_focus(app, 8, 8);
                    }
                } else {
                    // Tab: move forward
                    if (current_x < 8) {
                        set_button_focus(app, current_x + 1, current_y);
                    } else if (current_y < 8) {
                        set_button_focus(app, 0, current_y + 1);
                    } else {
                        set_button_focus(app, 0, 0);
                    }
                }
                return TRUE;
                
            case GDK_KEY_Home:
                // Move to first cell in current row
                set_button_focus(app, 0, current_y);
                return TRUE;
                
            case GDK_KEY_End:
                // Move to last cell in current row
                set_button_focus(app, 8, current_y);
                return TRUE;
                
            case GDK_KEY_Page_Up:
                // Move to top of current column
                set_button_focus(app, current_x, 0);
                return TRUE;
                
            case GDK_KEY_Page_Down:
                // Move to bottom of current column
                set_button_focus(app, current_x, 8);
                return TRUE;
                
            // Add support for number keys directly
            case GDK_KEY_1:
            case GDK_KEY_2:
            case GDK_KEY_3:
            case GDK_KEY_4:
            case GDK_KEY_5:
            case GDK_KEY_6:
            case GDK_KEY_7:
            case GDK_KEY_8:
            case GDK_KEY_9:
            case GDK_KEY_KP_1:
            case GDK_KEY_KP_2:
            case GDK_KEY_KP_3:
            case GDK_KEY_KP_4:
            case GDK_KEY_KP_5:
            case GDK_KEY_KP_6:
            case GDK_KEY_KP_7:
            case GDK_KEY_KP_8:
            case GDK_KEY_KP_9:
                // Let the cell handle the key press
                return FALSE;
                
            case GDK_KEY_BackSpace:
            case GDK_KEY_Delete:
            case GDK_KEY_space:
                // Let the cell handle clearing
                return FALSE;
        }
    }
    
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
        copy_board_callback(nullptr, app);
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

static void set_button_focus(SudokuApp *app, int x, int y) {
    // Validate coordinates
    if (x < 0) x = 0;
    if (x > 8) x = 8;
    if (y < 0) y = 0;
    if (y > 8) y = 8;
    
    // Remove focus styling from previously focused button
    if (app->currently_focused_button != nullptr) {
        GtkStyleContext *prev_context = gtk_widget_get_style_context(app->currently_focused_button);
        gtk_style_context_remove_class(prev_context, "focused-cell");
    }
    
    // Set focus to the button
    GtkWidget *button = app->buttons[y][x];
    gtk_widget_grab_focus(button);
    app->currently_focused_button = button;
    
    // Add focus styling
    GtkStyleContext *context = gtk_widget_get_style_context(button);
    gtk_style_context_add_class(context, "focused-cell");
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
    app->current_difficulty = difficulty;  // Store the difficulty string
    
    // Mark original cells
    mark_original_cells(app);
    
    // Update the display
    update_display(app);
    
    // Reset timer
    app->seconds_elapsed = 0;
    update_timer(app);
    app->timer_id = g_timeout_add(1000, timer_callback, app);  // Update every second
    
    // Update difficulty label - Fix for the garbled text
    std::string diff_text = "Difficulty: ";
    
    // Convert difficulty to display text with proper capitalization
    std::string display_diff;
    if (strcmp(difficulty, "easy") == 0) {
        display_diff = "Easy";
    } else if (strcmp(difficulty, "medium") == 0) {
        display_diff = "Medium";
    } else if (strcmp(difficulty, "hard") == 0) {
        display_diff = "Hard";
    } else if (strcmp(difficulty, "expert") == 0) {
        display_diff = "Expert";
    } else if (strcmp(difficulty, "extreme") == 0) {
        display_diff = "Extreme";
    } else {
        display_diff = difficulty;  // Use as is if not recognized
    }
    
    diff_text += display_diff;
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
    
    // Create a clean board with only original (clue) cells
    Sudoku clean_game;  // Create a fresh Sudoku instance
    
    // Copy only the original cells to the clean game
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            ButtonData *data = button_data[app->buttons[y][x]];
            if (data->original) {
                // Copy this cell's value to the clean game
                clean_game.SetValue(x, y, app->game->GetValue(x, y));
            }
        }
    }
    
    // Try to solve the puzzle from only the clue cells
    if (clean_game.Solve() == 0) {
        // First pass: collect all empty cells
        std::vector<std::pair<int, int>> empty_cells;
        
        // Find all empty cells in the current game state
        for (int y = 0; y < 9; y++) {
            for (int x = 0; x < 9; x++) {
                ButtonData *data = button_data[app->buttons[y][x]];
                
                // Skip original cells
                if (data->original) continue;
                
                // Check if the cell is empty
                if (app->game->GetValue(x, y) == -1) {
                    empty_cells.push_back(std::make_pair(x, y));
                }
            }
        }
        
        // If we found empty cells, give a hint for a random one
        if (!empty_cells.empty()) {
            // Pick a random empty cell
            int index = rand() % empty_cells.size();
            int x = empty_cells[index].first;
            int y = empty_cells[index].second;
            
            // Get the correct value from our solved clean game
            int correct_value = clean_game.GetValue(x, y);
            
            // Set the hint directly in the current game
            app->game->SetValue(x, y, correct_value);
            
            // Mark the cell as now original so it can't be changed
            ButtonData *data = button_data[app->buttons[y][x]];
            data->original = true;
            gtk_style_context_add_class(gtk_widget_get_style_context(app->buttons[y][x]), "hint-cell");
            
            // Update the display to show the hint
            update_display(app);
            
            gtk_label_set_text(GTK_LABEL(app->status_label), "Hint provided for a random empty cell");
            return;
        }
        
        // If no empty cells, look for incorrect cells
        std::vector<std::pair<int, int>> incorrect_cells;
        
        // Find cells with incorrect values
        for (int y = 0; y < 9; y++) {
            for (int x = 0; x < 9; x++) {
                ButtonData *data = button_data[app->buttons[y][x]];
                
                // Skip original cells
                if (data->original) continue;
                
                int current_value = app->game->GetValue(x, y);
                int correct_value = clean_game.GetValue(x, y);
                
                // If the cell has a value and it's incorrect
                if (current_value != -1 && current_value != correct_value) {
                    incorrect_cells.push_back(std::make_pair(x, y));
                }
            }
        }
        
        // If we found incorrect cells, correct a random one
        if (!incorrect_cells.empty()) {
            // Pick a random incorrect cell
            int index = rand() % incorrect_cells.size();
            int x = incorrect_cells[index].first;
            int y = incorrect_cells[index].second;
            
            // Apply the correct value from our solved clean game
            int correct_value = clean_game.GetValue(x, y);
            app->game->SetValue(x, y, correct_value);
            
            // Mark the cell as now original so it can't be changed
            ButtonData *data = button_data[app->buttons[y][x]];
            data->original = true;
            gtk_style_context_add_class(gtk_widget_get_style_context(app->buttons[y][x]), "hint-cell");
            
            // Update the display to show the correction
            update_display(app);
            
            gtk_label_set_text(GTK_LABEL(app->status_label), "Corrected a randomly selected wrong entry");
            return;
        }
        
        // If we get here, all cells are correctly filled
        gtk_label_set_text(GTK_LABEL(app->status_label), "Your solution is already correct!");
        return;
    } else {
        // Could not solve the puzzle from the original clues
        gtk_label_set_text(GTK_LABEL(app->status_label), "Could not solve the puzzle from its original state");
        
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
    
    // Make a backup of the current board
    int backup[9][9][9];
    memcpy(backup, app->game->board, sizeof(backup));
    
    // Create a clean board with only original (clue) cells
    Sudoku clean_game;  // Create a fresh Sudoku instance
    
    // Copy only the original cells to the clean game
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            ButtonData *data = button_data[app->buttons[y][x]];
            if (data->original) {
                // Copy this cell's value to the clean game
                clean_game.SetValue(x, y, app->game->GetValue(x, y));
            }
        }
    }
    
    // Try to solve the puzzle from only the clue cells
    if (clean_game.Solve() != 0) {
        gtk_label_set_text(GTK_LABEL(app->status_label), "Could not validate puzzle solution");
        return;
    }
    
    // Check for incorrect entries and highlight them
    bool has_errors = false;
    int error_count = 0;
    
    // Clear any previous error styling
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            GtkWidget *button = app->buttons[y][x];
            GtkStyleContext *context = gtk_widget_get_style_context(button);
            gtk_style_context_remove_class(context, "error-cell");
        }
    }
    
    // Check each non-empty, non-original cell
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            GtkWidget *button = app->buttons[y][x];
            ButtonData *data = button_data[button];
            
            // Skip original cells (they are always correct)
            if (data->original) continue;
            
            int current_value = app->game->GetValue(x, y);
            int correct_value = clean_game.GetValue(x, y);
            
            // Skip empty cells
            if (current_value == -1) continue;
            
            // If the value is incorrect, highlight it
            if (current_value != correct_value) {
                has_errors = true;
                error_count++;
                
                // Add error styling
                GtkStyleContext *context = gtk_widget_get_style_context(button);
                gtk_style_context_add_class(context, "error-cell");
            }
        }
    }
    
    // If no errors and all cells are filled, the solution is correct
    if (!has_errors && all_filled) {
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
        // Update status with appropriate message
        if (has_errors) {
            char error_message[100];
            snprintf(error_message, sizeof(error_message),
                    "Found %d incorrect %s. Check the highlighted cells in red.", 
                    error_count, error_count == 1 ? "entry" : "entries");
            gtk_label_set_text(GTK_LABEL(app->status_label), error_message);
        } else if (!all_filled) {
            gtk_label_set_text(GTK_LABEL(app->status_label), "The puzzle is not complete yet");
        }
    }
}

// Menu callback for new game
static void new_game_difficulty(GtkWidget *widget, gpointer user_data) {
    // Get the app pointer directly from user_data
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    // Get the difficulty string from the widget's data
    const char *difficulty = static_cast<const char*>(g_object_get_data(G_OBJECT(widget), "difficulty"));
    
    if (app && difficulty) {
        // Call start_new_game with the proper parameters
        start_new_game(app, difficulty);
    } else {
        g_warning("Failed to get app or difficulty data");
    }
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
// Function to handle copy board callback
static void copy_board_callback(GtkWidget *widget, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    // Create plain text content
    std::string plain_text = "Sudoku Puzzle\n";
    
    for (int y = 0; y < 9; y++) {
        // Add horizontal border
        if (y % 3 == 0) {
            plain_text += "+---------+---------+---------+\n";
        }
        
        for (int x = 0; x < 9; x++) {
            // Add vertical border
            if (x % 3 == 0) {
                plain_text += "|";
            }
            
            // Add cell value
            int value = app->game->GetValue(x, y);
            if (value >= 0 && value <= 8) {
                plain_text += " " + std::to_string(value + 1) + " ";
            } else {
                plain_text += " . ";
            }
        }
        
        // Add end of row
        plain_text += "|\n";
    }
    
    // Add bottom border
    plain_text += "+---------+---------+---------+\n";
    
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

static gboolean on_button_focus_in(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    SudokuApp *app = static_cast<SudokuApp*>(user_data);
    
    // Remove focus styling from previously focused button
    if (app->currently_focused_button != nullptr) {
        GtkStyleContext *prev_context = gtk_widget_get_style_context(app->currently_focused_button);
        gtk_style_context_remove_class(prev_context, "focused-cell");
    }
    
    // Set the new focused button and apply focus styling
    app->currently_focused_button = widget;
    
    // Add focus styling to current button
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, "focused-cell");
    
    // Announce focus change for screen readers
    ButtonData *data = button_data[widget];
    int cell_value = app->game->GetValue(data->x, data->y);
    std::string announcement;
    
    // Format announcement text based on cell value and position
    if (cell_value >= 0 && cell_value <= 8) {
        announcement = "Cell at row " + std::to_string(data->y + 1) + 
                       ", column " + std::to_string(data->x + 1) + 
                       " contains " + std::to_string(cell_value + 1);
    } else {
        announcement = "Empty cell at row " + std::to_string(data->y + 1) + 
                       ", column " + std::to_string(data->x + 1);
    }
    
    // Set the status text for screen readers
    gtk_label_set_text(GTK_LABEL(app->status_label), announcement.c_str());
    
    return FALSE;
}

static gboolean on_button_focus_out(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    // Remove focus styling when focus leaves
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_remove_class(context, "focused-cell");
    
    return FALSE;
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
    app->currently_focused_button = nullptr; // Initialize keyboard focus tracking
    
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
    
struct {
    const char* id;
    const char* label;
} difficulty_levels[] = {
    {"easy", "Easy (F1)"},
    {"medium", "Medium (F2)"},
    {"hard", "Hard (F3)"},
    {"expert", "Expert (F4)"},
    {"extreme", "Extreme (Shift+F1)"}
};

// Create difficulty menu items in the specified order
for (const auto& diff : difficulty_levels) {
    GtkWidget *diff_item = gtk_menu_item_new_with_label(diff.label);
    
    // Store the difficulty string in the widget using a persistent copy
    g_object_set_data_full(G_OBJECT(diff_item), "difficulty", 
                          g_strdup(diff.id), g_free);
    
    // Connect the signal passing app as user_data
    g_signal_connect(diff_item, "activate", G_CALLBACK(new_game_difficulty), app);
    
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
    
// Help menu
GtkWidget *help_menu = gtk_menu_new();
GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_item);

// Help menu items
GtkWidget *how_to_play_item = gtk_menu_item_new_with_label("How to Play");
g_signal_connect(how_to_play_item, "activate", G_CALLBACK(show_how_to_play), app);
gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), how_to_play_item);

GtkWidget *about_item = gtk_menu_item_new_with_label("About");
g_signal_connect(about_item, "activate", G_CALLBACK(show_about_dialog), app);
gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

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
    


    // Set up CSS styling for the grid (UPDATED with focus styles)
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
        ".hint-cell { color: green; font-weight: bold; }"
        ".error-cell { "
        "   color: red; "               /* Red text */
        "   background-color: #ffeded; " /* Light red background */
        "   border: 1px solid red; "    /* Red border */
        "}"
        ".focused-cell { "
        "   outline: 3px solid #ff8800; "  /* Bright orange outline */
        "   outline-offset: -3px; "        /* Keep the outline inside the button */
        "   background-color: #fff8e8; "   /* Light orange background */
        "   box-shadow: 0 0 8px #ff8800; " /* Glow effect */
        "   z-index: 100; "                /* Keep the focused cell on top */
        "}";

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
            
            // Make button focusable for keyboard navigation
            gtk_widget_set_can_focus(button, TRUE);
            
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
            
            // Track focus changes with enhanced focus handlers
            g_signal_connect(button, "focus-in-event", G_CALLBACK(on_button_focus_in), app);
            g_signal_connect(button, "focus-out-event", G_CALLBACK(on_button_focus_out), app);
            
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
        "Use arrow keys to navigate cells. Use F1-F4 for new puzzles of different difficulties."
    );
    gtk_label_set_line_wrap(GTK_LABEL(instructions), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), instructions, FALSE, FALSE, 5);
    
    // Show everything
    gtk_widget_show_all(window);
    
    // Start with a medium difficulty game
    start_new_game(app, "medium");
    
    // Set initial focus to center cell for better keyboard usability
    set_button_focus(app, 4, 4);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.sudoku.game", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
