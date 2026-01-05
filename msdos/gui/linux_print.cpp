#include <cstdarg>
#include <cstdio>
#include "sudoku.h"

// Simple Linux implementation of print_debug
// Outputs debug messages to stdout
void Sudoku::print_debug(const char *format, ...) {
    char buffer[256];
    
    // Format the string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Output to stdout with [DEBUG] prefix
    printf("[DEBUG] %s", buffer);
    fflush(stdout);
}

