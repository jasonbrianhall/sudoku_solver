#!/bin/bash

# Check and compile Linux version if g++ is available
if command -v g++ &> /dev/null; then
    echo "Compiling Linux version..."
    g++ main.cpp sudoku.cpp -lncurses -o sudoku_solver
else
    echo "g++ not found - skipping Linux build"
fi

# Check and compile Windows version if cross-compiler is available
if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Compiling Windows version..."
    x86_64-w64-mingw32-g++ main.cpp sudoku.cpp -lpdcurses -std=c++14 -o sudoku_solver.exe
else
    echo "Windows cross-compiler not found - skipping Windows build"
fi
