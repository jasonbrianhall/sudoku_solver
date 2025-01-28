#include "sudoku.h"

#if defined(_WIN32) || defined(WIN32) || defined(__MINGW32__)
    #include "curses.h"
#else
    #include <ncurses.h>
    #define _NCURSES

void Sudoku::print_debug(const char *format, ...) {
    char buffer[256];  // Buffer for formatted string
    debug_line=0;
    // Format the string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Move to position below grid
    move(29 + debug_line, 0);
    
    // Print the formatted string
    printw("%s", buffer);
    
    clrtoeol();  // Clear rest of line
        
    // Increment line counter, wrap around after 20 lines
    debug_line = (debug_line + 1) % 20;
}


#endif


