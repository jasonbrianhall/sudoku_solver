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
    #ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 0xE0) {
        ch = _getch();
        switch (ch) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            case 59: return KEY_F(1);
            case 60: return KEY_F(2);
            case 61: return KEY_F(3);
            case 62: return KEY_F(4);
            case 63: return KEY_F(5);
            case 64: return KEY_F(6);
            case 65: return KEY_F(7);
            case 66: return KEY_F(8);
            case 67: return KEY_F(9);
            case 68: return KEY_F(10);
            case 133: return KEY_F(11);
            case 134: return KEY_F(12);
            default: return ch;
        }
    }
    return ch;
    #else
    // Unix implementation
    int c = getchar();
    if (c == 27) {
        c = getchar();
        if (c == '[') {
            c = getchar();
            switch (c) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
            }
        }
    }
    return c;
    #endif
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
