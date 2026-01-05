/*
 * sudoku_main.cpp - Sudoku GUI using Allegro 4 for DOS/DJGPP
 * With triple buffering, file menu, and proper layout
 */

/* Declare far pointer functions BEFORE including allegro.h */
#ifdef __DJGPP__
extern int _farsetsel(unsigned short selector);
extern void _farnspokeb(unsigned long addr, unsigned char val);
extern unsigned char _farnspeekb(unsigned long addr);
extern void _farnspokew(unsigned long addr, unsigned short val);
extern unsigned short _farnspeekw(unsigned long addr);
extern void _farnspokel(unsigned long addr, unsigned long val);
extern unsigned long _farnspeekl(unsigned long addr);
#endif

#include "sudoku.h"
#include "generatepuzzle.h"
#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* Platform detection */
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #define LINUX_BUILD 1
#else
    #define DOS_BUILD 1
#endif

/* Double buffering setup */
#define NUM_BUFFERS 2
BITMAP *buffers[NUM_BUFFERS];
int current_buffer = 0;
BITMAP *active_buffer = NULL;

/* Allegro keyboard constants */
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
#define MENU_BAR_HEIGHT 25
#define GRID_START_X 50
#define GRID_START_Y (MENU_BAR_HEIGHT + 30)
#define CELL_SIZE 40
#define GRID_WIDTH (9 * CELL_SIZE)
#define GRID_HEIGHT (9 * CELL_SIZE)

#define BUTTON_PANEL_X (GRID_START_X + GRID_WIDTH + 30)
#define BUTTON_PANEL_Y (GRID_START_Y + 10)
#define BUTTON_WIDTH 110
#define BUTTON_HEIGHT 22
#define BUTTON_SPACING 5

#define STATUS_BAR_Y (GRID_START_Y + GRID_HEIGHT + 20)
#define HELP_TEXT_Y (STATUS_BAR_Y + 30)

/* Colors - Allegro 4 palette colors */
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
    
    /* Menu state */
    bool show_file_menu;
    int file_menu_selected;
    
    bool show_options_menu;
    int options_menu_selected;
    
    bool show_help_menu;
    int help_menu_selected;
    
    /* Timing */
    time_t last_action_time;
    
    /* Board backup for puzzle generation */
    int original_clues[9][9];
};

/* Global GUI state */
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
    sudoku_gui.show_file_menu = false;
    sudoku_gui.file_menu_selected = 0;
    sudoku_gui.show_options_menu = false;
    sudoku_gui.options_menu_selected = 0;
    sudoku_gui.show_help_menu = false;
    sudoku_gui.help_menu_selected = 0;
    sudoku_gui.game.NewGame();
}

/* Screen dirty tracking (like beatchess) */
static int screen_dirty = 1;
static int screen_needs_full_redraw = 1;

/* Close button state (Linux only) */
volatile bool window_close_requested = false;

void close_button_callback() {
    window_close_requested = true;
}

/**
 * Initialize double buffering
 */
void init_double_buffers() {
    for (int i = 0; i < NUM_BUFFERS; i++) {
        buffers[i] = create_bitmap(640, 480);
        if (!buffers[i]) {
            allegro_exit();
            fprintf(stderr, "Failed to allocate buffer %d\n", i);
            exit(1);
        }
        /* Initialize buffers to white */
        clear_to_color(buffers[i], COLOR_WHITE);
    }
    current_buffer = 0;
    active_buffer = buffers[0];
}

/**
 * Swap buffers and get the next one to draw into
 * Only blits to screen if screen_dirty is true
 */
BITMAP* get_next_buffer_and_swap() {
    /* Only update screen if it's marked as dirty */
    if (screen_dirty) {
        vsync();  /* Wait for vertical sync */
        blit(active_buffer, screen, 0, 0, 0, 0, 640, 480);
        screen_dirty = 0;  /* Mark screen as clean after update */
    }
    
    /* Swap to next buffer for drawing */
    current_buffer = (current_buffer + 1) % NUM_BUFFERS;
    active_buffer = buffers[current_buffer];
    
    return active_buffer;
}

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

/* File menu items */
const char *file_menu_items[] = {
    "New Game      N",
    "New Puzzle    P",
    "",  /* separator */
    "Save Game     S",
    "Load Game     L",
    "",  /* separator */
    "Quit          Q"
};

#define NUM_FILE_MENU_ITEMS (sizeof(file_menu_items) / sizeof(file_menu_items[0]))

/* Options menu items */
const char *options_menu_items[] = {
    "Difficulty: Easy",
    "Difficulty: Medium",
    "Difficulty: Hard",
    "",  /* separator */
    "Generate New",
};

#define NUM_OPTIONS_MENU_ITEMS (sizeof(options_menu_items) / sizeof(options_menu_items[0]))

/* Help menu items */
const char *help_menu_items[] = {
    "How to Play",
    "Keyboard Shortcuts",
    "About Sudoku",
};

#define NUM_HELP_MENU_ITEMS (sizeof(help_menu_items) / sizeof(help_menu_items[0]))

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
    /* Difficulty buttons */
    {BUTTON_PANEL_X, BUTTON_PANEL_Y,                                    BUTTON_WIDTH, BUTTON_HEIGHT, "Easy", 1},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 1*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Medium", 2},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 2*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Hard", 3},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 3*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Expert", 4},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 4*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Extreme", 5},
    
    /* Game control buttons */
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 6*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Solve", 10},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 7*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Undo", 14},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 8*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Redo", 15},
    {BUTTON_PANEL_X, BUTTON_PANEL_Y + 9*(BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Clear", 11},
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
        for (i = 0; i < MAX_HISTORY - 1; i++) {
            sudoku_gui.history[i] = sudoku_gui.history[i + 1];
        }
        sudoku_gui.history_count = MAX_HISTORY - 1;
    }
    
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

/* Draw cell using Allegro 4 only */
void draw_menu_bar() {
    /* Menu bar background */
    rectfill(active_buffer, 0, 0, 640, MENU_BAR_HEIGHT, COLOR_BLUE);
    
    /* Menu buttons */
    textout_ex(active_buffer, font, "File", 5, 5, COLOR_WHITE, -1);
    textout_ex(active_buffer, font, "Options", 50, 5, COLOR_WHITE, -1);
    textout_ex(active_buffer, font, "Help", 125, 5, COLOR_WHITE, -1);
    
    /* Title */
    textout_ex(active_buffer, font, "Sudoku - Allegro Edition", 250, 5, COLOR_YELLOW, -1);
    
    /* Draw File dropdown menu if active */
    if (sudoku_gui.show_file_menu) {
        int menu_x = 0;
        int menu_y = MENU_BAR_HEIGHT;
        int menu_w = 180;
        int item_h = 18;
        int menu_h = NUM_FILE_MENU_ITEMS * item_h;
        
        /* Menu background with border */
        rectfill(active_buffer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, COLOR_BLUE);
        rect(active_buffer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, COLOR_WHITE);
        
        /* Menu items */
        for (int i = 0; i < NUM_FILE_MENU_ITEMS; i++) {
            int item_y = menu_y + i * item_h;
            
            /* Separator */
            if (strlen(file_menu_items[i]) == 0) {
                hline(active_buffer, menu_x + 5, item_y + item_h/2, menu_x + menu_w - 5, COLOR_DARK_GRAY);
                continue;
            }
            
            /* Highlight selected */
            if (i == sudoku_gui.file_menu_selected) {
                rectfill(active_buffer, menu_x + 2, item_y + 2, 
                        menu_x + menu_w - 2, item_y + item_h - 2, COLOR_CYAN);
            }
            
            /* Draw menu item text */
            int text_color = (i == sudoku_gui.file_menu_selected) ? COLOR_BLACK : COLOR_WHITE;
            textout_ex(active_buffer, font, file_menu_items[i], menu_x + 10, item_y + 3, text_color, -1);
        }
    }
    
    /* Draw Options dropdown menu if active */
    if (sudoku_gui.show_options_menu) {
        int menu_x = 50;
        int menu_y = MENU_BAR_HEIGHT;
        int menu_w = 180;
        int item_h = 18;
        int menu_h = NUM_OPTIONS_MENU_ITEMS * item_h;
        
        /* Menu background with border */
        rectfill(active_buffer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, COLOR_BLUE);
        rect(active_buffer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, COLOR_WHITE);
        
        /* Menu items */
        for (int i = 0; i < NUM_OPTIONS_MENU_ITEMS; i++) {
            int item_y = menu_y + i * item_h;
            
            /* Separator */
            if (strlen(options_menu_items[i]) == 0) {
                hline(active_buffer, menu_x + 5, item_y + item_h/2, menu_x + menu_w - 5, COLOR_DARK_GRAY);
                continue;
            }
            
            /* Highlight selected */
            if (i == sudoku_gui.options_menu_selected) {
                rectfill(active_buffer, menu_x + 2, item_y + 2, 
                        menu_x + menu_w - 2, item_y + item_h - 2, COLOR_CYAN);
            }
            
            /* Draw menu item text */
            int text_color = (i == sudoku_gui.options_menu_selected) ? COLOR_BLACK : COLOR_WHITE;
            textout_ex(active_buffer, font, options_menu_items[i], menu_x + 10, item_y + 3, text_color, -1);
        }
    }
    
    /* Draw Help dropdown menu if active */
    if (sudoku_gui.show_help_menu) {
        int menu_x = 125;
        int menu_y = MENU_BAR_HEIGHT;
        int menu_w = 180;
        int item_h = 18;
        int menu_h = NUM_HELP_MENU_ITEMS * item_h;
        
        /* Menu background with border */
        rectfill(active_buffer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, COLOR_BLUE);
        rect(active_buffer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, COLOR_WHITE);
        
        /* Menu items */
        for (int i = 0; i < NUM_HELP_MENU_ITEMS; i++) {
            int item_y = menu_y + i * item_h;
            
            /* Separator */
            if (strlen(help_menu_items[i]) == 0) {
                hline(active_buffer, menu_x + 5, item_y + item_h/2, menu_x + menu_w - 5, COLOR_DARK_GRAY);
                continue;
            }
            
            /* Highlight selected */
            if (i == sudoku_gui.help_menu_selected) {
                rectfill(active_buffer, menu_x + 2, item_y + 2, 
                        menu_x + menu_w - 2, item_y + item_h - 2, COLOR_CYAN);
            }
            
            /* Draw menu item text */
            int text_color = (i == sudoku_gui.help_menu_selected) ? COLOR_BLACK : COLOR_WHITE;
            textout_ex(active_buffer, font, help_menu_items[i], menu_x + 10, item_y + 3, text_color, -1);
        }
    }
}

/**
 * Check if a cell has a conflict (duplicate in row, column, or 3x3 box)
 */
bool has_conflict(int col, int row) {
    int value = sudoku_gui.game.GetValue(col, row);
    if (value < 0) return false;  /* Empty cell, no conflict */
    
    /* Check row for duplicates */
    for (int c = 0; c < 9; c++) {
        if (c != col && sudoku_gui.game.GetValue(c, row) == value) {
            return true;
        }
    }
    
    /* Check column for duplicates */
    for (int r = 0; r < 9; r++) {
        if (r != row && sudoku_gui.game.GetValue(col, r) == value) {
            return true;
        }
    }
    
    /* Check 3x3 box for duplicates */
    int box_row = (row / 3) * 3;
    int box_col = (col / 3) * 3;
    for (int r = box_row; r < box_row + 3; r++) {
        for (int c = box_col; c < box_col + 3; c++) {
            if ((r != row || c != col) && sudoku_gui.game.GetValue(c, r) == value) {
                return true;
            }
        }
    }
    
    return false;
}

void draw_cell(int x, int y, int value, bool selected, bool is_clue) {
    int screen_x = GRID_START_X + x * CELL_SIZE;
    int screen_y = GRID_START_Y + y * CELL_SIZE;
    int color, text_color, bg_color;
    char text[4];
    
    /* Check for conflicts */
    bool conflict = has_conflict(x, y);
    
    /* Background color */
    if (conflict) {
        color = COLOR_RED;      /* Red if there's a conflict */
        text_color = COLOR_WHITE;
    } else if (selected) {
        color = 14;             /* Light yellow for selected */
        text_color = COLOR_BLACK;
    } else {
        color = COLOR_WHITE;    /* All cells white, no checkerboard */
        text_color = COLOR_BLACK;
    }
    
    rectfill(active_buffer, screen_x, screen_y, screen_x + CELL_SIZE - 1, screen_y + CELL_SIZE - 1, color);
    
    /* Border */
    int border_color = COLOR_BLACK;
    rect(active_buffer, screen_x, screen_y, screen_x + CELL_SIZE - 1, screen_y + CELL_SIZE - 1, border_color);
    
    /* Value */
    if (value >= 0 && value <= 8) {
        if (conflict) {
            text_color = COLOR_WHITE;  /* White text on red background */
            bg_color = COLOR_RED;
        } else if (is_clue) {
            text_color = COLOR_BLACK;  /* Black for clues */
            bg_color = color;
        } else {
            text_color = COLOR_BLUE;   /* Blue for user-entered */
            bg_color = color;
        }
        snprintf(text, sizeof(text), "%d", value + 1);
        /* Use opaque text rendering with matching background color to prevent bleeding */
        textout_ex(active_buffer, font, text, screen_x + CELL_SIZE/2 - 3, screen_y + CELL_SIZE/2 - 4, 
                   text_color, bg_color);
    }
}

void draw_grid() {
    int i, j, value;
    bool is_clue;
    bool selected;
    
    /* Draw all cells */
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            value = sudoku_gui.game.GetValue(j, i);
            is_clue = (sudoku_gui.original_clues[i][j] != -1);
            selected = (sudoku_gui.selected_row == i && sudoku_gui.selected_col == j);
            
            draw_cell(j, i, value, selected, is_clue);
        }
    }
    
    /* Draw thick grid lines every 3 squares */
    for (i = 0; i <= 9; i++) {
        int thickness = (i % 3 == 0) ? 3 : 1;
        int screen_pos = GRID_START_X + i * CELL_SIZE;
        vline(active_buffer, screen_pos, GRID_START_Y, GRID_START_Y + GRID_HEIGHT - 1, COLOR_BLACK);
    }
    
    for (i = 0; i <= 9; i++) {
        int screen_pos = GRID_START_Y + i * CELL_SIZE;
        hline(active_buffer, GRID_START_X, screen_pos, GRID_START_X + GRID_WIDTH - 1, COLOR_BLACK);
    }
}

void draw_buttons() {
    int i;
    
    /* Draw section headers for better organization */
    int header_x = BUTTON_PANEL_X + 5;
    textout_ex(active_buffer, font, "Difficulty:", header_x, BUTTON_PANEL_Y - 15, COLOR_BLACK, -1);
    
    for (i = 0; i < 5; i++) {  /* Difficulty buttons */
        Button *btn = &buttons[i];
        rectfill(active_buffer, btn->x, btn->y, btn->x + btn->width - 1, btn->y + btn->height - 1, 
                COLOR_LIGHT_GRAY);
        rect(active_buffer, btn->x, btn->y, btn->x + btn->width - 1, btn->y + btn->height - 1, 
            COLOR_BLACK);
        textout_ex(active_buffer, font, btn->label, btn->x + 5, btn->y + 4, COLOR_BLACK, -1);
    }
    
    /* Game controls section */
    textout_ex(active_buffer, font, "Game:", header_x, BUTTON_PANEL_Y + 5 * (BUTTON_HEIGHT + BUTTON_SPACING) + 5, 
               COLOR_BLACK, -1);
    
    for (i = 5; i < 9; i++) {  /* Game control buttons */
        Button *btn = &buttons[i];
        rectfill(active_buffer, btn->x, btn->y, btn->x + btn->width - 1, btn->y + btn->height - 1, 
                COLOR_LIGHT_GRAY);
        rect(active_buffer, btn->x, btn->y, btn->x + btn->width - 1, btn->y + btn->height - 1, 
            COLOR_BLACK);
        textout_ex(active_buffer, font, btn->label, btn->x + 5, btn->y + 4, COLOR_BLACK, -1);
    }
}

void draw_status_bar() {
    char buf[256];
    
    /* Status background */
    rectfill(active_buffer, 0, STATUS_BAR_Y, 640, STATUS_BAR_Y + 20, COLOR_LIGHT_GRAY);
    
    /* Status text */
    if (sudoku_gui.status_timer > 0) {
        snprintf(buf, sizeof(buf), "%s", sudoku_gui.status_message);
        textout_ex(active_buffer, font, buf, 10, STATUS_BAR_Y + 3, COLOR_BLACK, -1);
        sudoku_gui.status_timer--;
    }
}

void draw_help() {
    char *h1 = "Move: Arrows | Input: 1-9 | Clear: 0/Del";
    char *h2 = "Solve: S | Undo: Z | Redo: Y | New: N";
    textout_ex(active_buffer, font, h1, GRID_START_X, HELP_TEXT_Y, COLOR_BLACK, -1);
    textout_ex(active_buffer, font, h2, GRID_START_X, HELP_TEXT_Y + 12, COLOR_BLACK, -1);
}

void redraw_screen() {
    /* Always draw if full redraw needed, otherwise only if dirty */
    if (!screen_dirty && !screen_needs_full_redraw) return;
    
    /* Clear the buffer before drawing */
    if (screen_needs_full_redraw) {
        clear_to_color(active_buffer, COLOR_WHITE);
        screen_needs_full_redraw = 0;
    } else {
        clear_to_color(active_buffer, COLOR_WHITE);
    }
    
    /* Hide mouse cursor during drawing */
    scare_mouse();
    
    /* Draw game elements FIRST */
    draw_grid();
    draw_buttons();
    draw_status_bar();
    
    /* Draw menu LAST so it appears on top */
    draw_menu_bar();
    
    /* Show mouse cursor again */
    unscare_mouse();
    
    /* NOW swap/display the buffer AFTER drawing to it */
    if (screen_dirty) {
        vsync();  /* Wait for vertical sync */
        blit(active_buffer, screen, 0, 0, 0, 0, 640, 480);  /* Display what we just drew */
        screen_dirty = 0;  /* Mark screen as clean after update */
        
        /* Swap to next buffer for next frame */
        current_buffer = (current_buffer + 1) % NUM_BUFFERS;
        active_buffer = buffers[current_buffer];
    }
}

void generate_puzzle(int difficulty) {
    const char *difficulty_names[] = {"easy", "medium", "hard", "expert", "extreme"};
    
    if (difficulty < 0 || difficulty >= 5) return;
    
    sudoku_gui.is_generating = true;
    display_status("Generating puzzle...");
    
    PuzzleGenerator generator(sudoku_gui.game);
    if (generator.generatePuzzle(difficulty_names[difficulty])) {
        int y, x;
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

void handle_file_menu_selection(int item) {
    int i, j;
    
    switch (item) {
        case 0:  /* New Game */
            sudoku_gui.game.NewGame();
            for (i = 0; i < 9; i++) {
                for (j = 0; j < 9; j++) {
                    sudoku_gui.original_clues[i][j] = -1;
                }
            }
            sudoku_gui.history_count = 0;
            sudoku_gui.history_index = 0;
            display_status("New game started");
            break;
        case 1:  /* New Puzzle */
            generate_puzzle(1);  /* Medium difficulty */
            break;
        case 3:  /* Save Game */
            sudoku_gui.game.SaveToFile("sudoku_save.txt");
            display_status("Game saved");
            break;
        case 4:  /* Load Game */
            if (sudoku_gui.game.LoadFromFile("sudoku_save.txt")) {
                display_status("Game loaded");
            } else {
                display_status("Load failed");
            }
            sudoku_gui.history_count = 0;
            sudoku_gui.history_index = 0;
            break;
        case 6:  /* Quit */
            exit(0);
            break;
    }
    
    sudoku_gui.show_file_menu = false;
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
        case 14:
            undo_last_move();
            break;
        case 15:
            redo_last_move();
            break;
    }
}

int main(void) {
    int key_code;
    int mx, my;
    int grid_x, grid_y;
    int button_id;
    int prev_mouse_b = 0;
    int old_val, new_val;
    bool running;
    
    /* Initialize Allegro - exactly like beatchess */
    if (allegro_init() != 0) {
        printf("Failed to initialize Allegro\n");
        return 1;
    }
    
    install_keyboard();
    install_mouse();
    install_timer();
    
    /* Set graphics mode - windowed on Linux, fullscreen on DOS */
    int gfx_result = -1;
    
    #ifdef LINUX_BUILD
    /* On Linux, try windowed mode first */
    gfx_result = set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
    if (gfx_result != 0) {
        /* Fallback to autodetect if windowed fails */
        gfx_result = set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0);
    }
    #else
    /* On DOS, use fullscreen autodetect */
    gfx_result = set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0);
    #endif
    
    if (gfx_result != 0) {
        allegro_exit();
        printf("Error setting graphics mode: %s\n", allegro_error);
        return 1;
    }
    
    show_mouse(screen);
    
    /* Register close button callback (Linux only) */
    #ifdef LINUX_BUILD
    set_close_button_callback(close_button_callback);
    #endif
    
    /* Initialize double buffering */
    init_double_buffers();
    
    /* Initialize GUI */
    init_sudoku_gui();
    
    /* Mark screen dirty to draw initial board */
    mark_screen_needs_full_redraw();
    
    int prev_mouse_x = -1;
    int prev_mouse_y = -1;
    
    running = true;
    
    /* Main game loop - like beatchess */
    while (running) {
        /* Always ensure mouse is visible (might be hidden from previous draw) */
        show_mouse(screen);
        
        /* Check if window close button was clicked (Linux) */
        if (window_close_requested) {
            running = false;
            break;
        }
        
        if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y) {
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
            
            /* If menu is open, update which item is highlighted based on mouse position */
            if (sudoku_gui.show_file_menu) {
                if (mouse_x >= 0 && mouse_x < 180 &&
                    mouse_y >= MENU_BAR_HEIGHT && mouse_y < MENU_BAR_HEIGHT + NUM_FILE_MENU_ITEMS * 18) {
                    /* Mouse is over menu - highlight the item under cursor */
                    int hovered_item = (mouse_y - MENU_BAR_HEIGHT) / 18;
                    /* Only select non-separator items */
                    if (hovered_item < NUM_FILE_MENU_ITEMS && strlen(file_menu_items[hovered_item]) > 0) {
                        sudoku_gui.file_menu_selected = hovered_item;
                    }
                } else {
                    /* Mouse moved away from menu - reset to first item */
                    sudoku_gui.file_menu_selected = 0;
                }
            } else if (sudoku_gui.show_options_menu) {
                if (mouse_x >= 50 && mouse_x < 230 &&
                    mouse_y >= MENU_BAR_HEIGHT && mouse_y < MENU_BAR_HEIGHT + NUM_OPTIONS_MENU_ITEMS * 18) {
                    /* Mouse is over menu - highlight the item under cursor */
                    int hovered_item = (mouse_y - MENU_BAR_HEIGHT) / 18;
                    /* Only select non-separator items */
                    if (hovered_item < NUM_OPTIONS_MENU_ITEMS && strlen(options_menu_items[hovered_item]) > 0) {
                        sudoku_gui.options_menu_selected = hovered_item;
                    }
                } else {
                    /* Mouse moved away from menu - reset to first item */
                    sudoku_gui.options_menu_selected = 0;
                }
            } else if (sudoku_gui.show_help_menu) {
                if (mouse_x >= 125 && mouse_x < 305 &&
                    mouse_y >= MENU_BAR_HEIGHT && mouse_y < MENU_BAR_HEIGHT + NUM_HELP_MENU_ITEMS * 18) {
                    /* Mouse is over menu - highlight the item under cursor */
                    int hovered_item = (mouse_y - MENU_BAR_HEIGHT) / 18;
                    /* Only select non-separator items */
                    if (hovered_item < NUM_HELP_MENU_ITEMS && strlen(help_menu_items[hovered_item]) > 0) {
                        sudoku_gui.help_menu_selected = hovered_item;
                    }
                } else {
                    /* Mouse moved away from menu - reset to first item */
                    sudoku_gui.help_menu_selected = 0;
                }
            }
            
            mark_screen_dirty();  /* Redraw when mouse moves */
        }
        /* Keyboard input - use keypressed/readkey like beatchess */
        if (keypressed()) {
            key_code = readkey();
            int key_ascii = key_code & 0xFF;
            int key_scancode = (key_code >> 8) & 0xFF;
            
            if (key_ascii >= 'a' && key_ascii <= 'z') {
                key_ascii = key_ascii - 'a' + 'A';
            }
            
            switch (key_scancode) {
                case KEY_UP:
                    if (sudoku_gui.show_file_menu) {
                        sudoku_gui.file_menu_selected = (sudoku_gui.file_menu_selected - 1 + NUM_FILE_MENU_ITEMS) % NUM_FILE_MENU_ITEMS;
                        /* Skip separators */
                        while (strlen(file_menu_items[sudoku_gui.file_menu_selected]) == 0) {
                            sudoku_gui.file_menu_selected = (sudoku_gui.file_menu_selected - 1 + NUM_FILE_MENU_ITEMS) % NUM_FILE_MENU_ITEMS;
                        }
                    } else {
                        sudoku_gui.selected_row = (sudoku_gui.selected_row - 1 + 9) % 9;
                    }
                    mark_screen_dirty();
                    break;
                case KEY_DOWN:
                    if (sudoku_gui.show_file_menu) {
                        sudoku_gui.file_menu_selected = (sudoku_gui.file_menu_selected + 1) % NUM_FILE_MENU_ITEMS;
                        /* Skip separators */
                        while (strlen(file_menu_items[sudoku_gui.file_menu_selected]) == 0) {
                            sudoku_gui.file_menu_selected = (sudoku_gui.file_menu_selected + 1) % NUM_FILE_MENU_ITEMS;
                        }
                    } else {
                        sudoku_gui.selected_row = (sudoku_gui.selected_row + 1) % 9;
                    }
                    mark_screen_dirty();
                    break;
                case KEY_LEFT:
                    sudoku_gui.selected_col = (sudoku_gui.selected_col - 1 + 9) % 9;
                    mark_screen_dirty();
                    break;
                case KEY_RIGHT:
                    sudoku_gui.selected_col = (sudoku_gui.selected_col + 1) % 9;
                    mark_screen_dirty();
                    break;
                default:
                    switch (key_ascii) {
                        case '0':
                        case KEY_DEL:
                            if (!sudoku_gui.show_file_menu) {
                                old_val = sudoku_gui.game.GetValue(sudoku_gui.selected_col, 
                                                                    sudoku_gui.selected_row);
                                add_to_history(sudoku_gui.selected_col, sudoku_gui.selected_row, 
                                             old_val, -1);
                                sudoku_gui.game.ClearValue(sudoku_gui.selected_col, 
                                                          sudoku_gui.selected_row);
                                mark_screen_dirty();
                            }
                            break;
                        case '1': case '2': case '3': case '4': case '5':
                        case '6': case '7': case '8': case '9':
                            if (!sudoku_gui.show_file_menu) {
                                old_val = sudoku_gui.game.GetValue(sudoku_gui.selected_col, 
                                                                    sudoku_gui.selected_row);
                                new_val = key_ascii - '1';
                                add_to_history(sudoku_gui.selected_col, sudoku_gui.selected_row, 
                                             old_val, new_val);
                                sudoku_gui.game.SetValue(sudoku_gui.selected_col, 
                                                        sudoku_gui.selected_row, new_val);
                                mark_screen_dirty();
                            }
                            break;
                        case 'S':
                            if (sudoku_gui.show_file_menu) {
                                sudoku_gui.show_file_menu = false;
                                sudoku_gui.game.SaveToFile("sudoku_save.txt");
                                display_status("Game saved");
                            } else {
                                sudoku_gui.game.Solve();
                                display_status("Puzzle solved!");
                            }
                            mark_screen_dirty();
                            break;
                        case 'Q':
                            if (sudoku_gui.show_file_menu) {
                                exit(0);
                            } else {
                                running = false;
                            }
                            break;
                        case 'N':
                            if (sudoku_gui.show_file_menu) {
                                sudoku_gui.show_file_menu = false;
                                handle_file_menu_selection(0);  /* New Game */
                            } else {
                                sudoku_gui.game.NewGame();
                                sudoku_gui.history_count = 0;
                                sudoku_gui.history_index = 0;
                                mark_screen_dirty();
                            }
                            break;
                        case 'P':
                            if (sudoku_gui.show_file_menu) {
                                sudoku_gui.show_file_menu = false;
                                handle_file_menu_selection(1);  /* New Puzzle */
                            }
                            break;
                        case 'L':
                            if (sudoku_gui.show_file_menu) {
                                sudoku_gui.show_file_menu = false;
                                if (sudoku_gui.game.LoadFromFile("sudoku_save.txt")) {
                                    display_status("Game loaded");
                                } else {
                                    display_status("Load failed");
                                }
                                sudoku_gui.history_count = 0;
                                sudoku_gui.history_index = 0;
                                mark_screen_dirty();
                            }
                            break;
                        case 'Z':
                            if (!sudoku_gui.show_file_menu) {
                                undo_last_move();
                            }
                            break;
                        case 'Y':
                            if (!sudoku_gui.show_file_menu) {
                                redo_last_move();
                            }
                            break;
                        case 'F':
                            /* Toggle File menu with Alt+F, but since we don't have Alt, use F */
                            sudoku_gui.show_file_menu = !sudoku_gui.show_file_menu;
                            sudoku_gui.file_menu_selected = 0;
                            mark_screen_dirty();
                            break;
                    }
                    /* Handle Enter key for menu selection */
                    if (key_scancode == KEY_ENTER && sudoku_gui.show_file_menu) {
                        handle_file_menu_selection(sudoku_gui.file_menu_selected);
                        mark_screen_dirty();
                    }
                    break;
            }
        }
        
        /* Mouse input - like beatchess */
        if ((mouse_b & 1) && !(prev_mouse_b & 1)) {
            mx = mouse_x;
            my = mouse_y;
            
            /* Bounds check */
            if (mx < 0) mx = 0;
            if (mx >= 640) mx = 639;
            if (my < 0) my = 0;
            if (my >= 480) my = 479;
            
            /* Check if click is on a File menu item (when menu is open) */
            if (sudoku_gui.show_file_menu && mx >= 0 && mx < 180 && 
                my >= MENU_BAR_HEIGHT && my < MENU_BAR_HEIGHT + NUM_FILE_MENU_ITEMS * 18) {
                int menu_item = (my - MENU_BAR_HEIGHT) / 18;
                if (menu_item < NUM_FILE_MENU_ITEMS && strlen(file_menu_items[menu_item]) > 0) {
                    handle_file_menu_selection(menu_item);
                }
                mark_screen_dirty();
            }
            /* Check if click is on an Options menu item (when menu is open) */
            else if (sudoku_gui.show_options_menu && mx >= 50 && mx < 230 && 
                my >= MENU_BAR_HEIGHT && my < MENU_BAR_HEIGHT + NUM_OPTIONS_MENU_ITEMS * 18) {
                int menu_item = (my - MENU_BAR_HEIGHT) / 18;
                if (menu_item < NUM_OPTIONS_MENU_ITEMS && strlen(options_menu_items[menu_item]) > 0) {
                    if (menu_item == 0) {
                        generate_puzzle(0);  /* Easy */
                    } else if (menu_item == 1) {
                        generate_puzzle(1);  /* Medium */
                    } else if (menu_item == 2) {
                        generate_puzzle(2);  /* Hard */
                    } else if (menu_item == 4) {
                        generate_puzzle(1);  /* Generate new at medium */
                    }
                }
                mark_screen_dirty();
            }
            /* Check if click is on a Help menu item (when menu is open) */
            else if (sudoku_gui.show_help_menu && mx >= 125 && mx < 305 && 
                my >= MENU_BAR_HEIGHT && my < MENU_BAR_HEIGHT + NUM_HELP_MENU_ITEMS * 18) {
                int menu_item = (my - MENU_BAR_HEIGHT) / 18;
                if (menu_item < NUM_HELP_MENU_ITEMS && strlen(help_menu_items[menu_item]) > 0) {
                    if (menu_item == 0) {
                        display_status("Sudoku: Fill the 9x9 grid so each row, column, and 3x3 box contains 1-9");
                    } else if (menu_item == 1) {
                        display_status("Arrows=Move, 1-9=Input, 0/Del=Clear, S=Solve, Z=Undo, Y=Redo, N=New");
                    } else if (menu_item == 2) {
                        display_status("Sudoku puzzle - fill grid with numbers 1-9. One number per cell, no repeats");
                    }
                }
                mark_screen_dirty();
            }
            /* Check if click is on the File menu button */
            else if (my < MENU_BAR_HEIGHT && mx >= 0 && mx < 50) {
                sudoku_gui.show_file_menu = !sudoku_gui.show_file_menu;
                sudoku_gui.show_options_menu = false;
                sudoku_gui.show_help_menu = false;
                sudoku_gui.file_menu_selected = 0;
                mark_screen_dirty();
            }
            /* Check if click is on the Options menu button */
            else if (my < MENU_BAR_HEIGHT && mx >= 50 && mx < 125) {
                sudoku_gui.show_options_menu = !sudoku_gui.show_options_menu;
                sudoku_gui.show_file_menu = false;
                sudoku_gui.show_help_menu = false;
                sudoku_gui.options_menu_selected = 0;
                mark_screen_dirty();
            }
            /* Check if click is on the Help menu button */
            else if (my < MENU_BAR_HEIGHT && mx >= 125 && mx < 180) {
                sudoku_gui.show_help_menu = !sudoku_gui.show_help_menu;
                sudoku_gui.show_file_menu = false;
                sudoku_gui.show_options_menu = false;
                sudoku_gui.help_menu_selected = 0;
                mark_screen_dirty();
            }
            /* If any menu is open and click is elsewhere - close all menus */
            else if (sudoku_gui.show_file_menu || sudoku_gui.show_options_menu || sudoku_gui.show_help_menu) {
                sudoku_gui.show_file_menu = false;
                sudoku_gui.show_options_menu = false;
                sudoku_gui.show_help_menu = false;
                mark_screen_dirty();
            }
            /* Check if click is on grid */
            else if (screen_to_grid(mx, my, &grid_x, &grid_y)) {
                sudoku_gui.selected_col = grid_x;
                sudoku_gui.selected_row = grid_y;
                mark_screen_dirty();
            }
            /* Check if click is on a button */
            else {
                button_id = get_button_at(mx, my);
                if (button_id >= 0) {
                    handle_button_click(button_id);
                    mark_screen_dirty();
                }
            }
        }
        
        prev_mouse_b = mouse_b;
        
        /* Redraw screen after processing all input */
        redraw_screen();
        
        rest(10);  /* 10ms delay like beatchess */
    }
    
    /* Cleanup */
    allegro_exit();
    return 0;
}

END_OF_MAIN()
