#ifndef MINI_CURSES_H
#define MINI_CURSES_H

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

/* PDCurses standard key definitions */
#define KEY_OFFSET  0x101     /* PDCurses key offset */
#define KEY_UP      0x103     /* Move cursor up */
#define KEY_DOWN    0x102     /* Move cursor down */
#define KEY_LEFT    0x104     /* Move cursor left */
#define KEY_RIGHT   0x105     /* Move cursor right */
#define KEY_HOME    0x106     /* Home key */
#define KEY_BACKSPACE 0x107   /* Backspace */
#define KEY_F0      0x108     /* Function keys. F1 is KEY_F(1) */
#define KEY_F(n)    (KEY_F0+(n))  /* Value of function key n */
#define KEY_DL      0x148     /* Delete line */
#define KEY_IL      0x149     /* Insert line */
#define KEY_DC      0x14A     /* Delete character */
#define KEY_IC      0x14B     /* Insert character */
#define KEY_EIC     0x14C     /* Exit insert char mode */
#define KEY_CLEAR   0x14D     /* Clear screen */
#define KEY_EOS     0x14E     /* Clear to end of screen */
#define KEY_EOL     0x14F     /* Clear to end of line */
#define KEY_SF      0x150     /* Scroll 1 line forward */
#define KEY_SR      0x151     /* Scroll 1 line backward */
#define KEY_NPAGE   0x152     /* Next page */
#define KEY_PPAGE   0x153     /* Previous page */
#define KEY_STAB    0x154     /* Set tab */
#define KEY_CTAB    0x155     /* Clear tab */
#define KEY_CATAB   0x156     /* Clear all tabs */
#define KEY_ENTER   0x157     /* Enter or send */
#define KEY_SRESET  0x158     /* Soft/reset */
#define KEY_RESET   0x159     /* Hard/reset */
#define KEY_PRINT   0x15A     /* Print */
#define KEY_LL      0x15B     /* Home down/bottom */
#define KEY_A1      0x15C     /* Upper left of keypad */
#define KEY_A3      0x15D     /* Upper right of keypad */
#define KEY_B2      0x15E     /* Center of keypad */
#define KEY_C1      0x15F     /* Lower left of keypad */
#define KEY_C3      0x160     /* Lower right of keypad */
#define KEY_BTAB    0x161     /* Back tab */
#define KEY_BEG     0x162     /* Beginning key */
#define KEY_CANCEL  0x163     /* Cancel key */
#define KEY_CLOSE   0x164     /* Close key */
#define KEY_COMMAND 0x165     /* Command key */
#define KEY_COPY    0x166     /* Copy key */
#define KEY_CREATE  0x167     /* Create key */
#define KEY_END     0x168     /* End key */
#define KEY_EXIT    0x169     /* Exit key */
#define KEY_FIND    0x16A     /* Find key */
#define KEY_HELP    0x16B     /* Help key */
#define KEY_MARK    0x16C     /* Mark key */
#define KEY_MESSAGE 0x16D     /* Message key */
#define KEY_MOVE    0x16E     /* Move key */
#define KEY_NEXT    0x16F     /* Next key */

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

/* Cursor styles */
#define CURSOR_INVISIBLE 0
#define CURSOR_BLOCK    1
#define CURSOR_NORMAL   2

/* Function declarations */
int curs_set(int visibility);

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
int clrtoeol(void);

/* Global variable declarations */
extern WINDOW* stdscr;

#ifdef __cplusplus
}
#endif

#endif /* MINI_CURSES_H */
