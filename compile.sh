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
    echo "Compiling Windows version (Fake NCurses, not Mono) as Static ..."

    mkdir win_sudoku -p

    # This version uses curses.cpp; a miniature version of ncurses I wrote so it can be statically compiled;  -pdcurses doesn't work with -static and then requires 30+ DLLs
    x86_64-w64-mingw32-g++ main.cpp sudoku.cpp generatepuzzle.cpp unixprint.cpp curses.cpp -std=c++14 -o win_sudoku/sudoku_solver.exe
    x86_64-w64-mingw32-g++ main.cpp sudoku.cpp generatepuzzle.cpp unixprint.cpp curses.cpp -std=c++14 -static -o win_sudoku/sudoku_solver_static.exe
    ./collect_dlls.sh win_sudoku/sudoku_solver.exe  /usr/x86_64-w64-mingw32/sys-root/mingw/bin win_sudoku

    mkdir win_sudoku_pdcurses -p

    echo "Compiling Windows version (PDCurses) ..."
    x86_64-w64-mingw32-g++ main.cpp sudoku.cpp generatepuzzle.cpp pdcursesprint.cpp curses.cpp -lpdcurses -std=c++14 -o win_sudoku_pdcurses/sudoku_solver_pdcurses.exe
    ./collect_dlls.sh win_sudoku_pdcurses/sudoku_solver_pdcurses.exe  /usr/x86_64-w64-mingw32/sys-root/mingw/bin win_sudoku_pdcurses

else
    echo "Windows cross-compiler not found - skipping Windows build"
fi

if command -v docker &> /dev/null; then
    echo "Compiling MSDOS version..."
    make -f Makefile_MSDOS msdos
fi
