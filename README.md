# Sudoku Solver

An interactive Sudoku solver implemented in both C++ and Python, featuring a user-friendly ncurses interface for puzzle input and visualization. The C++ implementation offers robust solving capabilities with multiple advanced techniques, while the Python version serves as a basic prototype.

Their are two main C++ versions, a CLI version written in Ncurses and a GUI version designed for Microsoft Windows.  The C++ Windows/CLI versions can also generate puzzles to be solved.  

## Features

- Interactive puzzle input with arrow key navigation
- Real-time validity checking of moves
- Multiple solving techniques ranging from basic to advanced
- Colored display for better visualization
- Cross-platform compatibility (Linux, macOS, Unix-like systems, MS Windows)
- Comprehensive logging of solving steps
- Available C++ CLI, Windows GUI and Python (prototype)
- MS Windows version available
- Sudoku puzzle generation in C++ versions

## Getting Started

### Prerequisites

- For C++/Ncurses version:
  - G++ compiler
  - ncurses library
  - Make (optional, for compilation)

- For C++/Windows version:
  - Visual Studio with CMD support
  - msbuild

- For Python version (prototype):
  - Python 3.x
  - curses module (typically included in standard Python distributions)

### Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/sudoku_solver.git
cd sudoku_solver
```

2. Compile the C++ version:
```bash
chmod +x compile.sh
./compile.sh
```

Or manually compile:
```bash
g++ main.cpp sudoku.cpp -lncurses -o sudoku_solver
```
Or with Make Script:
```bash
make
```

Windows Version:
```
msbuild /p:Configuration=Release /p:Platform=x64 SudokuSolver.vcxproj
```

The Python version is a prototype with limited functionality and doesn't require compilation. For the best solving experience, use the C++ implementation.  The python version only has very basic algorithms implmeneted while the C++ versions (Windows and UNIX) can solve extreme puzzles due to the use of advanced algorithms.

## Usage

### Running the Solver

For C++ version:
```bash
./sudoku_solver
```

For Python version (prototype):
```bash
python3 sudoku_solver.py
```

For C#/C++ Windows Version
```cmd
sudoku_solver
```

### Controls

- Arrow keys: Navigate the puzzle grid
- Numbers 1-9: Input values
- '0' or 'c': Clear current cell
- 'q': Quit the program (CLI only)
- ESC: Quit (Python version only)
- Z: New Game
- F1-F4 and Shift F1:  Genaretes random puzzles, F1 being easy, F2, F3, F4 increasingly harder and shift F1 being the hardest
- F5-F8:  Quick Save
- Shft F5-F8:  Quick Load
- A - Automatically solve the puzzle
- Other keys for various algorithms (documented in interface)

### Solving Techniques (C++ version)

The solver implements several Sudoku solving techniques of increasing complexity:

1. Basic Techniques:
   - Standard Elimination (S): Eliminates possibilities in rows, columns, and boxes
   - Line Elimination (L): Identifies unique candidates in rows, columns, and boxes

2. Advanced Techniques:
   - Hidden Singles (N): Finds cells where a number can only go in one position
   - Hidden Pairs (H): Identifies pairs of numbers confined to two cells
   - Pointing Pairs (P): Finds candidates restricted to specific rows/columns within boxes
   - X-Wing (X): Locates rectangle patterns that eliminate candidates
   - Swordfish (F): Advanced triple-line elimination strategy
   - Naked Sets (K): Identifies groups of cells with confined candidates
   - XY-Wing (Y):  XY-Wing is a solving technique that looks for three cells forming a Y pattern, where two cells (the "wings") share a candidate with a pivot cell, and eliminations can be made from cells that see both wings.
   - XYZ-Wing (;): Advanced X-Wing Method

### Recommended Solving Strategy

For the most efficient solving experience:

1. Alternate between Standard Elimination (S) and Line Elimination (L) until no more changes occur
2. When stuck, try one of the advanced techniques:
   - Start with Hidden Singles (N) and Hidden Pairs (H)
   - Progress to Pointing Pairs (P) and X-Wing (X)
   - Use Swordfish (F), Naked Sets (K), and XY-Wing (Y) for the most challenging situations
3. After finding new possibilities with advanced techniques, return to alternating between Standard and Line Elimination
4. Repeat this process until the puzzle is solved

Pressing (A) for All Algorithms implements this strategy.

I've been testing it against https://sudoku.com/extreme/ and haven't had too many fail to solve.

## Implementation Details

### Data Structure

- C++ Implementation:
  - Uses a 3D array `board[9][9][9]` to track possible values
  - Each cell maintains its own set of candidate numbers
  - Comprehensive validation and backtracking support
  - Real-time logging of solving steps

- Python Implementation (Prototype):
  - Basic implementation using nested dictionaries
  - Limited to simple elimination techniques
  - Serves as a proof of concept

### Performance

The C++ implementation offers comprehensive solving capabilities suitable for most standard and advanced Sudoku puzzles. The Python version is a prototype and may struggle with more complex puzzles.

## Contributing

Contributions are welcome! Here are some ways you can help:

1. Implement additional solving techniques
2. Improve the user interface
3. Add puzzle generation functionality
4. Enhance error handling
5. Write tests
6. Improve documentation
7. Port advanced features to Python version

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Special thanks to the ncurses library developers
- Inspired by classic Sudoku solving techniques
- Built with love for puzzle enthusiasts and developers alike

---

## Keywords and Topics

### Puzzle Types & Applications
sudoku | sudoku solver | logic puzzle | puzzle game | puzzle solver | brain teaser | number puzzle

### Core Technologies
c++ | cpp | python | ncurses | terminal ui | tui | command line interface | cli | cross-platform | windows

### Technical Features
- Algorithms: backtracking | constraint programming | optimization | elimination algorithm
- Interface: terminal-based | command-line | ncurses-interface | interactive-ui
- Architecture: object-oriented | modular design | clean code

### Development
- Languages: C++ | Python3 | Modern C++
- Type: Open Source | FOSS | Free Software
- Platform: Linux | Unix | macOS | Cross-Platform
- Category: Game Development | Developer Tools | Educational

### Academic & Concepts
- Mathematics: combinatorics | constraint satisfaction | logic programming
- Techniques: pattern recognition | logical deduction | systematic solving
- Education: programming examples | algorithm implementation | data structures

### Repository Information
Author: Jason Hall jasonbrianhall@gmail.com
License: MIT  
Version: 1.0  
Last Updated: 2025-01

---

_This is an open-source Sudoku solver implementing advanced solving techniques through a terminal-based interface, available in both C++ and Python. Perfect for puzzle enthusiasts, developers learning algorithm implementation, or anyone interested in logic puzzle solving techniques._
