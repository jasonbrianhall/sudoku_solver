#!/bin/bash

# Check and compile Windows version if cross-compiler is available
if command -v x86_64-w64-mingw32-g++ &> /dev/null; then

    mkdir win_sudoku_pdcurses -p

    echo "Compiling Windows version (PDCurses) ..."
    x86_64-w64-mingw32-g++ main.cpp sudoku.cpp generatepuzzle.cpp pdcursesprint.cpp -lpdcurses -std=c++14 -o win_sudoku_pdcurses/sudoku_solver_pdcurses.exe
    ./collect_dlls.sh win_sudoku_pdcurses/sudoku_solver_pdcurses.exe  /usr/x86_64-w64-mingw32/sys-root/mingw/bin win_sudoku_pdcurses

else
    echo "Windows cross-compiler not found - skipping Windows build"
fi
