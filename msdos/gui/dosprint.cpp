#ifdef MSDOS

#include <conio.h>
#include <stdarg.h>  // Required for va_list, va_start, va_end
#include <stdio.h>   // Required for vsprintf
#include "sudoku.h"


#define DEBUG_BUFFER_SIZE 10
void Sudoku::print_debug(const char *format, ...) {
    char buffer[256];
    
    // Format the string
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    // Move to fixed position at bottom of screen
    gotoxy(1, 23);  // Using line 23 for debug output
    
    // Clear the line first
    for(int i = 0; i < 79; i++) {
        putch(' ');
    }
    
    // Return to start of line and print the message
    gotoxy(1, 23);
    cputs(buffer);
}
#endif
