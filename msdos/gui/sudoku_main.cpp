/*
 * sudoku_main.cpp - Sudoku GUI using Allegro 4 for DOS/DJGPP
 * DOS-compatible version - no C++20 features
 * Compiles with DJGPP and older GCC versions
 */

#include "sudoku.h"
#include "generatepuzzle.h"

#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* Platform detection */
#ifdef MSDOS
    #define DOS_BUILD 1
#else
    #define LINUX_BUILD 1
#endif

/* Keyboard constants */
#ifndef KEY_UP
    #define KEY_UP 0x48
#endif
#ifndef KEY_DOWN
    #define KEY_DOWN 0x50
#endif
#ifndef KEY_LEFT
    #define KEY_LEFT 0x4B
#endif
#ifndef KEY_RIGHT
    #define KEY_RIGHT 0x4D
#endif
#ifndef KEY_ESC
    #define KEY_ESC 0x01
#endif
#ifndef KEY_ENTER
    #define KEY_ENTER 0x1C
#endif
#ifndef KEY_DEL
    #define KEY_DEL 0x53
#endif
#ifndef KEY_Z
    #define KEY_Z 0x2C
#endif
#ifndef KEY_Y
    #define KEY_Y 0x15
#endif

/* UI Layout */
#define GRID_START_X 50
#define GRID_START_Y 60
#define CELL_SIZE 40
#define GRID_WIDTH (9 * CELL_SIZE)
#define GRID_HEIGHT (9 * CELL_SIZE)

#define BUTTON_PANEL_X (GRID_START_X + GRID_WIDTH + 30)
#define BUTTON_PANEL_Y GRID_START_Y
#define BUTTON_WIDTH 120
#define BUTTON_HEIGHT 25
#define BUTTON_SPACING 8

#define STATUS_BAR_Y (GRID_START_Y + GRID_HEIGHT + 20)
#define HELP_TEXT_Y (STATUS_BAR_Y + 30)

/* Colors */
#define COLOR_BLACK 0
#define COLOR_WHITE 15
#define COLOR_LIGHT_GRAY 7
#define COLOR_DARK_GRAY 8
#define COLOR_BLUE 1
#define COLOR_RED 4
#define COLOR_GREEN 2
#define COLOR_YELLOW 14
#define COLOR_CYAN 3

/* Undo/Redo stack size */
#define MAX_HISTORY 100

/* Move history entry */
struct MoveHistory {
    int x;
    int y;
    int old_value;
    int new_value;
};

/* Sudoku GUI State */
struct SudokuGUI {
    Sudoku game;
    Sudoku backup_for_generation;
    
    int selected_row;
    int selected_col;
    
    int difficulty_level;
    bool is_solved;
    bool is_generating;
    
    /* Undo/Redo */
    MoveHistory history[MAX_HISTORY];
    int history_count;
    int history_index;
    
    /* UI State */
    char status_message[256];
    int status_timer;
    bool show_candidates;
    
    /* Timing */
    time_t last_action_time;
    
    /* Board backup for puzzle generation */
    int original_clues[9][9];
};

/* Initialize sudoku_gui using constructor-like function to avoid C++20 features */
static SudokuGUI sudoku_gui;

void init_sudoku_gui() {
    memset(&sudoku_gui, 0, sizeof(sudoku_gui));
    sudoku_gui.selected_row = 0;
    sudoku_gui.selected_col = 0;
    sudoku_gui.difficulty_level = 1;
    sudoku_gui.is_solved = false;
    sudoku_gui.is_generating = false;
    sudoku_gui.history_count = 0;
    sudoku_gui.history_index = 0;
    sudoku_gui.status_timer = 0;
    sudoku_gui.show_candidates = false;
    sudoku_gui.game.NewGame();
}

static int screen_dirty = 1;
static int screen_needs_full_redraw = 1;

void mark_screen_dirty() {
    screen_dirty = 1;
}

void mark_screen_clean() {
    screen_dirty = 0;
}

void mark_screen_needs_full_redraw() {
    screen_needs_full_redraw = 1;
    mark_screen_dirty();
}

void display_status(const char *message) {
    strncpy(sudoku_gui.status_message, message, sizeof(sudoku_gui.status_message) - 1);
    sudoku_gui.status_message[sizeof(sudoku_gui.status_message) - 1] = '\0';
    sudoku_gui.status_timer = 120;  /* Display for ~2 seconds at 60 FPS */
    mark_screen_dirty();
}

/* Button definitions */
struct Button {
    int x;
    int y;
    int width;
    int height;
    const char *label;
    int id;
};

static Button buttons[] = {
    {BUTTON_PANEL_X, BUTTON_PANEL_Y,                    BUTTON_WIDTH, BUTTON_HEIGHT, "Easy", 1},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + BUTTON_HEIGHT + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT, "Medium", 2},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 2*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Hard", 3},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 3*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Expert", 4},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 4*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Extreme", 5},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 6*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Solve", 10},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 7*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Undo", 14},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 8*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Redo", 15},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 9*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Clear", 11},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 10*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Save", 12},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 11*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Load", 13},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 13*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Quit", 20},
};

#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))

bool point_in_button(int x, int y, const Button *btn) {
    return x >= btn->x && x <= btn->x + btn->width &&
           y >= btn->y && y <= btn->y + btn->height;
}

int get_button_at(int x, int y) {
    int i;
    for (i = 0; i < NUM_BUTTONS; i++) {
        if (point_in_button(x, y, &buttons[i])) {
            return buttons[i].id;
        }
    }
    return -1;
}

bool screen_to_grid(int screen_x, int screen_y, int *grid_x, int *grid_y) {
    if (screen_x < GRID_START_X || screen_x >= GRID_START_X + GRID_WIDTH ||
        screen_y < GRID_START_Y || screen_y >= GRID_START_Y + GRID_HEIGHT) {
        return false;
    }
    
    *grid_x = (screen_x - GRID_START_X) / CELL_SIZE;
    *grid_y = (screen_y - GRID_START_Y) / CELL_SIZE;
    
    return *grid_x >= 0 && *grid_x < 9 && *grid_y >= 0 && *grid_y < 9;
}

/* Undo/Redo implementation */
void add_to_history(int x, int y, int old_value, int new_value) {
    if (sudoku_gui.history_count >= MAX_HISTORY) {
        int i;
        /* Shift history down */
        for (i = 0; i < MAX_HISTORY - 1; i++) {
            sudoku_gui.history[i] = sudoku_gui.history[i + 1];
        }
        sudoku_gui.history_count = MAX_HISTORY - 1;
    }
    
    /* Add new move */
    sudoku_gui.history[sudoku_gui.history_count].x = x;
    sudoku_gui.history[sudoku_gui.history_count].y = y;
    sudoku_gui.history[sudoku_gui.history_count].old_value = old_value;
    sudoku_gui.history[sudoku_gui.history_count].new_value = new_value;
    sudoku_gui.history_count++;
    sudoku_gui.history_index = sudoku_gui.history_count;
}

void undo_last_move() {
    MoveHistory *move;
    
    if (sudoku_gui.history_index <= 0) {
        display_status("Nothing to undo");
        return;
    }
    
    sudoku_gui.history_index--;
    move = &sudoku_gui.history[sudoku_gui.history_index];
    
    sudoku_gui.game.SetValue(move->x, move->y, move->old_value);
    display_status("Move undone");
    mark_screen_dirty();
}

void redo_last_move() {
    MoveHistory *move;
    
    if (sudoku_gui.history_index >= sudoku_gui.history_count) {
        display_status("Nothing to redo");
        return;
    }
    
    move = &sudoku_gui.history[sudoku_gui.history_index];
    sudoku_gui.game.SetValue(move->x, move->y, move->new_value);
    sudoku_gui.history_index++;
    
    display_status("Move redone");
    mark_screen_dirty();
}

/* Draw a cell with enhanced visuals */
void draw_cell(int x, int y, int value, bool selected, bool is_clue) {
    int screen_x = GRID_START_X + x * CELL_SIZE;
    int screen_y = GRID_START_Y + y * CELL_SIZE;
    
    /* Cell background */
    int bg_color;
    if (selected) {
        bg_color = COLOR_YELLOW;
    } else if (is_clue) {
        bg_color = COLOR_LIGHT_GRAY;
    } else {
        bg_color = ((x + y) % 2 == 0) ? COLOR_WHITE : 7;
    }
    
    rectfill(screen, screen_x, screen_y, 
             screen_x + CELL_SIZE - 1, screen_y + CELL_SIZE - 1, 
             bg_color);
    
    /* Borders */
    int border_thickness = 1;
    int i;
    if (x % 3 == 0) border_thickness = 2;
    if (y % 3 == 0) border_thickness = 2;
    
    for (i = 0; i < border_thickness; i++) {
        hline(screen, screen_x, screen_y + i, screen_x + CELL_SIZE - 1, COLOR_BLACK);
        vline(screen, screen_x + i, screen_y, screen_y + CELL_SIZE - 1, COLOR_BLACK);
    }
    
    /* Value */
    if (value >= 0 && value < 9) {
        char str[2];
        sprintf(str, "%d", value + 1);
        int text_x = screen_x + CELL_SIZE / 2 - 3;
        int text_y = screen_y + CELL_SIZE / 2 - 4;
        int text_color = is_clue ? COLOR_BLACK : COLOR_BLUE;
        textout_ex(screen, font, str, text_x, text_y, text_color, -1);
    }
}

void draw_grid() {
    int x, y;
    int value;
    bool selected;
    bool is_clue;
    
    /* Title */
    textout_ex(screen, font, "Sudoku Solver - Allegro 4 DOS", 50, 20, COLOR_BLACK, -1);
    
    /* Grid cells */
    for (y = 0; y < 9; y++) {
        for (x = 0; x < 9; x++) {
            value = sudoku_gui.game.GetValue(x, y);
            selected = (x == sudoku_gui.selected_col && y == sudoku_gui.selected_row);
            is_clue = (sudoku_gui.original_clues[y][x] != -1);
            
            draw_cell(x, y, value, selected, is_clue);
        }
    }
}

void draw_button(const Button *btn, bool highlighted) {
    int bg_color = highlighted ? COLOR_BLUE : COLOR_LIGHT_GRAY;
    int text_color = highlighted ? COLOR_WHITE : COLOR_BLACK;
    
    rectfill(screen, btn->x, btn->y, 
             btn->x + btn->width, btn->y + btn->height, 
             bg_color);
    
    rect(screen, btn->x, btn->y, 
         btn->x + btn->width, btn->y + btn->height, 
         COLOR_BLACK);
    
    textout_ex(screen, font, btn->label, btn->x + 5, btn->y + 5, text_color, -1);
}

void draw_buttons() {
    int i;
    textout_ex(screen, font, "Actions", BUTTON_PANEL_X, BUTTON_PANEL_Y - 30, COLOR_BLACK, -1);
    
    for (i = 0; i < NUM_BUTTONS; i++) {
        draw_button(&buttons[i], false);
    }
}

void draw_status_bar() {
    if (sudoku_gui.status_timer > 0) {
        textout_ex(screen, font, sudoku_gui.status_message, GRID_START_X, STATUS_BAR_Y, 
                  COLOR_BLACK, -1);
        sudoku_gui.status_timer--;
    }
}

void draw_help() {
    const char *help = "Arrow: Move | 1-9: Fill | 0: Clear | S: Solve | Q: Quit";
    textout_ex(screen, font, help, GRID_START_X, HELP_TEXT_Y, COLOR_BLACK, -1);
}

void redraw_screen() {
    if (!screen_dirty) return;
    
    if (screen_needs_full_redraw) {
        clear_to_color(screen, COLOR_WHITE);
        screen_needs_full_redraw = 0;
    }
    
    draw_grid();
    draw_buttons();
    draw_status_bar();
    draw_help();
    
    mark_screen_clean();
}

void generate_puzzle(int difficulty) {
    const char *difficulty_names[] = {"easy", "medium", "hard", "expert", "extreme"};
    
    if (difficulty < 0 || difficulty >= 5) return;
    
    sudoku_gui.is_generating = true;
    display_status("Generating puzzle...");
    
    PuzzleGenerator generator(sudoku_gui.game);
    if (generator.generatePuzzle(difficulty_names[difficulty])) {
        int y, x;
        /* Save clues for reference */
        for (y = 0; y < 9; y++) {
            for (x = 0; x < 9; x++) {
                sudoku_gui.original_clues[y][x] = sudoku_gui.game.GetValue(x, y);
            }
        }
        
        sudoku_gui.difficulty_level = difficulty;
        sudoku_gui.history_count = 0;
        sudoku_gui.history_index = 0;
        display_status("Puzzle generated!");
    } else {
        display_status("Failed to generate puzzle");
    }
    
    sudoku_gui.is_generating = false;
    mark_screen_dirty();
}

void handle_button_click(int button_id) {
    int i, j;
    
    switch (button_id) {
        case 1:
            generate_puzzle(0);
            break;
        case 2:
            generate_puzzle(1);
            break;
        case 3:
            generate_puzzle(2);
            break;
        case 4:
            generate_puzzle(3);
            break;
        case 5:
            generate_puzzle(4);
            break;
        case 10:
            sudoku_gui.game.Solve();
            display_status("Puzzle solved!");
            mark_screen_dirty();
            break;
        case 11:
            sudoku_gui.game.NewGame();
            for (i = 0; i < 9; i++) {
                for (j = 0; j < 9; j++) {
                    sudoku_gui.original_clues[i][j] = -1;
                }
            }
            sudoku_gui.history_count = 0;
            sudoku_gui.history_index = 0;
            display_status("Board cleared");
            mark_screen_dirty();
            break;
        case 12:
            sudoku_gui.game.SaveToFile("sudoku_save.txt");
            display_status("Game saved to sudoku_save.txt");
            break;
        case 13:
            if (sudoku_gui.game.LoadFromFile("sudoku_save.txt")) {
                display_status("Game loaded from sudoku_save.txt");
            } else {
                display_status("Failed to load game");
            }
            sudoku_gui.history_count = 0;
            sudoku_gui.history_index = 0;
            mark_screen_dirty();
            break;
        case 14:
            undo_last_move();
            break;
        case 15:
            redo_last_move();
            break;
        case 20:
            break;
    }
}

int main(void) {
    int key_code, key_ascii, key_scancode;
    int mx, my;
    int grid_x, grid_y;
    int button_id;
    int prev_mouse_b = 0;
    int old_val, new_val;
    bool need_redraw;
    bool running;
    
    /* Initialize Allegro */
    if (allegro_init() != 0) {
        printf("Failed to initialize Allegro\n");
        return 1;
    }
    
    install_keyboard();
    install_mouse();
    install_timer();
    
    /* Set graphics mode */
    if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) != 0) {
        printf("Error setting graphics mode\n");
        return 1;
    }
    
    show_mouse(screen);
    
    /* Initialize GUI */
    init_sudoku_gui();
    
    running = true;
    
    /* Game loop */
    while (running) {
        redraw_screen();
        
        /* Keyboard input */
        if (keypressed()) {
            key_code = readkey();
            key_ascii = key_code & 0xFF;
            key_scancode = (key_code >> 8) & 0xFF;
            
            if (key_ascii >= 'a' && key_ascii <= 'z') {
                key_ascii = key_ascii - 'a' + 'A';
            }
            
            need_redraw = true;
            
            switch (key_scancode) {
                case KEY_UP:
                    sudoku_gui.selected_row = (sudoku_gui.selected_row - 1 + 9) % 9;
                    break;
                case KEY_DOWN:
                    sudoku_gui.selected_row = (sudoku_gui.selected_row + 1) % 9;
                    break;
                case KEY_LEFT:
                    sudoku_gui.selected_col = (sudoku_gui.selected_col - 1 + 9) % 9;
                    break;
                case KEY_RIGHT:
                    sudoku_gui.selected_col = (sudoku_gui.selected_col + 1) % 9;
                    break;
                default:
                    need_redraw = false;
                    break;
            }
            
            if (!need_redraw) {
                switch (key_ascii) {
                    case '0':
                    case KEY_DEL:
                        old_val = sudoku_gui.game.GetValue(sudoku_gui.selected_col, 
                                                            sudoku_gui.selected_row);
                        add_to_history(sudoku_gui.selected_col, sudoku_gui.selected_row, 
                                     old_val, -1);
                        sudoku_gui.game.ClearValue(sudoku_gui.selected_col, 
                                                  sudoku_gui.selected_row);
                        need_redraw = true;
                        break;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        old_val = sudoku_gui.game.GetValue(sudoku_gui.selected_col, 
                                                            sudoku_gui.selected_row);
                        new_val = key_ascii - '1';
                        add_to_history(sudoku_gui.selected_col, sudoku_gui.selected_row, 
                                     old_val, new_val);
                        sudoku_gui.game.SetValue(sudoku_gui.selected_col, 
                                                sudoku_gui.selected_row, new_val);
                        need_redraw = true;
                        break;
                    case 'S':
                        sudoku_gui.game.Solve();
                        display_status("Puzzle solved!");
                        need_redraw = true;
                        break;
                    case 'Q':
                        running = false;
                        break;
                    case 'Z':
                        undo_last_move();
                        need_redraw = true;
                        break;
                    case 'Y':
                        redo_last_move();
                        need_redraw = true;
                        break;
                    case 'N':
                        sudoku_gui.game.NewGame();
                        sudoku_gui.history_count = 0;
                        sudoku_gui.history_index = 0;
                        need_redraw = true;
                        break;
                    default:
                        break;
                }
            }
            
            if (need_redraw) {
                mark_screen_dirty();
            }
        }
        
        /* Mouse input */
        if ((mouse_b & 1) && !(prev_mouse_b & 1)) {
            mx = mouse_x;
            my = mouse_y;
            
            if (screen_to_grid(mx, my, &grid_x, &grid_y)) {
                sudoku_gui.selected_col = grid_x;
                sudoku_gui.selected_row = grid_y;
                mark_screen_dirty();
            } else {
                button_id = get_button_at(mx, my);
                if (button_id >= 0) {
                    if (button_id == 20) {
                        running = false;
                    } else {
                        handle_button_click(button_id);
                        mark_screen_dirty();
                    }
                }
            }
        }
        
        prev_mouse_b = mouse_b;
        rest(10);
    }
    
    allegro_exit();
    return 0;
}

END_OF_MAIN()
