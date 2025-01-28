#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "curses.h"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif

// Global variables
WINDOW* stdscr = NULL;
static int current_attr = A_NORMAL;
static int color_pairs[64][2] = {{0}};
static HANDLE hConsole = NULL;

// Implementation
WINDOW* initscr(void) {
    if (!stdscr) {
        stdscr = (WINDOW*)malloc(sizeof(WINDOW));
        if (stdscr) {
            stdscr->_curx = stdscr->_cury = 0;
            stdscr->_rows = 24;
            stdscr->_cols = 80;
            stdscr->_attrs = A_NORMAL;
            
            #ifdef _WIN32
            // Enable ANSI escape sequences in Windows 10+
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode = 0;
            GetConsoleMode(hConsole, &mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hConsole, mode);
            #endif
            
            printf("\033[?1049h");  // Enable alternate screen buffer
            printf("\033[2J");      // Clear screen
            printf("\033[H");       // Move to home position
            printf("\033[?25l");    // Hide cursor
            fflush(stdout);
        }
    }
    return stdscr;
}

int endwin(void) {
    if (stdscr) {
        printf("\033[?25h");    // Show cursor
        printf("\033[?1049l");  // Disable alternate screen buffer
        printf("\033[0m");      // Reset all attributes
        fflush(stdout);
        free(stdscr);
        stdscr = NULL;
    }
    return OK;
}

int refresh(void) {
    fflush(stdout);
    return OK;
}

int move(int y, int x) {
    if (stdscr) {
        stdscr->_cury = y;
        stdscr->_curx = x;
        printf("\033[%d;%dH", y + 1, x + 1);
        fflush(stdout);
    }
    return OK;
}

int clear(void) {
    printf("\033[2J");
    printf("\033[H");
    fflush(stdout);
    return OK;
}

// Fixed keyboard input handling
int getch(void) {
    int ch = _getch();
    if (ch == 0 || ch == 0xE0) {  // Extended keys
        ch = _getch();
        // Convert Windows scan codes to our key definitions
        switch (ch) {
            case 0x48: return KEY_UP;    // Up arrow
            case 0x50: return KEY_DOWN;   // Down arrow
            case 0x4B: return KEY_LEFT;   // Left arrow
            case 0x4D: return KEY_RIGHT;  // Right arrow
            case 0x3B: return KEY_F(1);   // F1
            case 0x3C: return KEY_F(2);   // F2
            case 0x3D: return KEY_F(3);   // F3
            case 0x3E: return KEY_F(4);   // F4
            case 0x3F: return KEY_F(5);   // F5
            case 0x40: return KEY_F(6);   // F6
            case 0x41: return KEY_F(7);   // F7
            case 0x42: return KEY_F(8);   // F8
            case 0x43: return KEY_F(9);   // F9
            case 0x44: return KEY_F(10);  // F10
            case 0x85: return KEY_F(11);  // F11
            case 0x86: return KEY_F(12);  // F12
            default: return ch;
        }
    }
    return ch;
}

int wgetch(WINDOW *win) {
    return getch();
}

int printw(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    return OK;
}

int keypad(WINDOW* win, bool bf) {
    return OK;
}

int start_color(void) {
    return OK;
}

int init_pair(short pair, short f, short b) {
    if (pair >= 0 && pair < 64) {
        color_pairs[pair][0] = f;
        color_pairs[pair][1] = b;
    }
    return OK;
}

// Windows-specific color mapping
static void set_console_color(int color) {
    #ifdef _WIN32
    static const WORD win_colors[] = {
        0,                                          // BLACK
        FOREGROUND_RED,                            // RED
        FOREGROUND_GREEN,                          // GREEN
        FOREGROUND_RED | FOREGROUND_GREEN,         // YELLOW
        FOREGROUND_BLUE,                           // BLUE
        FOREGROUND_RED | FOREGROUND_BLUE,          // MAGENTA
        FOREGROUND_GREEN | FOREGROUND_BLUE,        // CYAN
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE // WHITE
    };
    if (color >= 0 && color < 8 && hConsole != NULL) {
        SetConsoleTextAttribute(hConsole, win_colors[color]);
    }
    #else
    printf("\033[3%dm", color);
    #endif
}

int attron(int attrs) {
    current_attr |= attrs;
    if (attrs & A_BOLD) {
        printf("\033[1m");
    }
    if ((attrs >> 8) > 0) {
        int pair = (attrs >> 8) - 1;
        if (pair >= 0 && pair < 64) {
            set_console_color(color_pairs[pair][0]);
        }
    }
    fflush(stdout);
    return OK;
}

int attroff(int attrs) {
    current_attr &= ~attrs;
    printf("\033[0m");
    if (current_attr & A_BOLD) {
        printf("\033[1m");
    }
    fflush(stdout);
    return OK;
}
