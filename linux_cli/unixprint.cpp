#include "sudoku.h"

#ifdef _WIN32
	#include <curses.h>
#else
#ifdef MSDOS
	#include <curses.h>
        #include <stdarg.h>  // Required for va_list, va_start, va_end
        #include <stdio.h>   // Required for vsprintf
#else
	#include <ncurses.h>
#endif
#endif

#define _NCURSES

int debug_line=0;


void Sudoku::print_debug(const char *format, ...) {
    char buffer[256];  // Buffer for formatted string
    // Format the string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Move to position below grid
#ifndef MSDOS
    move(23 + debug_line, 0);
#else
    move(23, 0);
#endif    
    
    // Print the formatted string
    printw("%s", buffer);
    
    clrtoeol();  // Clear rest of line
        
    // Increment line counter, wrap around after 10 lines
    debug_line = (debug_line + 1) % 10;
}



