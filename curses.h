#ifndef MINI_CURSES_H
#define MINI_CURSES_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Color definitions */
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

/* Key definitions */
#define KEY_UP    0x415B1B
#define KEY_DOWN  0x425B1B
#define KEY_RIGHT 0x435B1B
#define KEY_LEFT  0x445B1B
#define KEY_F(n)  (0x5B31315B1B + (n))

/* Attributes */
#define A_NORMAL     0
#define A_BOLD       1
#define COLOR_PAIR(n) (n << 8)

#define OK 0

/* Window structure */
typedef struct _win_st {
    int _cury, _curx;
    int _rows, _cols;
    int _attrs;
} WINDOW;

/* Function declarations */
WINDOW* initscr(void);
int endwin(void);
int refresh(void);
int move(int y, int x);
int clear(void);
int getch(void);
int printw(const char* fmt, ...);
int wgetch(WINDOW *win);
int keypad(WINDOW* win, bool bf);
int start_color(void);
int init_pair(short pair, short f, short b);
int attron(int attrs);
int attroff(int attrs);

/* Global variable declarations */
extern WINDOW* stdscr;

#ifdef __cplusplus
}
#endif
