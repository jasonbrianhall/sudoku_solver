#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "curses.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// Global variables
WINDOW* stdscr = NULL;
static int current_attr = A_NORMAL;
static int color_pairs[64][2] = {0};
static int current_cursor_state = CURSOR_NORMAL;

#ifdef _WIN32
static HANDLE hConsole = NULL;
static CONSOLE_CURSOR_INFO original_cursor;

static void set_cursor_style(int style) {
    if (hConsole == NULL) return;
    
    CONSOLE_CURSOR_INFO cursor = {0};
    switch(style) {
        case CURSOR_INVISIBLE:
            cursor.bVisible = FALSE;
            cursor.dwSize = 1;
            break;
        case CURSOR_BLOCK:
            cursor.bVisible = TRUE;
            cursor.dwSize = 100;  // 100% of cell height = block cursor
            break;
        case CURSOR_NORMAL:
        default:
            cursor.bVisible = TRUE;
            cursor.dwSize = 25;   // 25% of cell height = underscore
            break;
    }
    SetConsoleCursorInfo(hConsole, &cursor);
    current_cursor_state = style;
}
#else
// Unix implementation of getch
static struct termios orig_termios;

static void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

static void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

static int _getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    }
    return c;
}
#endif

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
            if (hConsole != INVALID_HANDLE_VALUE) {
                DWORD mode = 0;
                GetConsoleMode(hConsole, &mode);
                mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hConsole, mode);
                
                // Save original cursor info and set block cursor
                GetConsoleCursorInfo(hConsole, &original_cursor);
                set_cursor_style(CURSOR_BLOCK);
            }
            #else
            set_conio_terminal_mode();
            printf("\033[?25h");  // Show cursor
            #endif
            
            printf("\033[2J");      // Clear screen
            printf("\033[H");       // Move to home position
            fflush(stdout);
        }
    }
    return stdscr;
}

int endwin(void) {
    if (stdscr) {
        #ifdef _WIN32
        if (hConsole != NULL) {
            // Restore original cursor
            SetConsoleCursorInfo(hConsole, &original_cursor);
        }
        #endif
        
        printf("\033[0m");      // Reset all attributes
        printf("\033[2J");      // Clear screen
        printf("\033[H");       // Move to home position
        fflush(stdout);
        free(stdscr);
        stdscr = NULL;
    }
    return OK;
}

int curs_set(int visibility) {
    int old_state = current_cursor_state;
    #ifdef _WIN32
    set_cursor_style(visibility);
    #else
    switch(visibility) {
        case CURSOR_INVISIBLE:
            printf("\033[?25l");
            break;
        case CURSOR_NORMAL:
        case CURSOR_BLOCK:
            printf("\033[?25h");
            break;
    }
    fflush(stdout);
    current_cursor_state = visibility;
    #endif
    return old_state;
}


int refresh(void) {
    fflush(stdout);
    return OK;
}

int move(int y, int x) {
    if (stdscr) {
        stdscr->_cury = y;
        stdscr->_curx = x;
        printf("\033[%d;%dH", y+1, x + 1);
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

int clrtoeol(void) {
    printf("\033[K");  // ANSI escape sequence to clear from cursor to end of line
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

int COLOR_PAIR(int number) {
    return number;
}

int attron(int pair) {
    int foreground, background;
    if (pair >= 0 && pair < 64) {
        foreground = color_pairs[pair][0];
        background = color_pairs[pair][1];
        printf("\033[%d;%dm", 30 + foreground, 40 + background);
    }
    return OK;
}

int attroff(int pair) {
    printf("\033[0m");    return OK;
}
