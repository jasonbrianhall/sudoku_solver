#include <iostream>
#include <cstdarg>
#include "sudoku.h"

// Implementation of the print_debug function
void Sudoku::print_debug(const char *format, ...) {
    // Create a buffer for the formatted string
    char buffer[512];
    
    // Format the string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Print to stderr
    std::cerr << buffer;
}
