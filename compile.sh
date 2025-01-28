#!/bin/bash

# Check and compile Linux version if g++ is available
if command -v g++ &> /dev/null; then
    echo "Compiling Linux version..."
    g++ main.cpp sudoku.cpp generatepuzzle.cpp unixprint.cpp -lncurses -o sudoku_solver
else
    echo "g++ not found - skipping Linux build"
fi

# Check and compile Windows version if cross-compiler is available
if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Compiling Windows version (NCurses, not Mono) ..."

    # This version uses curses.cpp; a miniature version of curses I wrote so it can be statically compiled;  -pdcurses doesn't work with -static and then requires 30+ DLLs

    x86_64-w64-mingw32-g++ main.cpp sudoku.cpp generatepuzzle.cpp unixprint.cpp curses.cpp -std=c++14 -static -o sudoku_solver.exe
else
    echo "Windows cross-compiler not found - skipping Windows build"
fi

if command -v docker &> /dev/null; then
    echo "Compiling MSDOS version..."
    make -f Makefile_MSDOS msdos
fi
