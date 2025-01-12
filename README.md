# Sudoku Solver

An interactive command-line Sudoku solver implemented in both C++ and Python, featuring a user-friendly ncurses interface for puzzle input and visualization.

## Features

- Interactive puzzle input with arrow key navigation
- Real-time validity checking of moves
- Automatic puzzle solving functionality
- Colored display for better visualization
- Cross-platform compatibility (Linux, macOS, Unix-like systems)
- Available in both C++ and Python implementations

## Getting Started

### Prerequisites

- For C++ version:
  - G++ compiler
  - ncurses library
  - Make (optional, for compilation)

- For Python version:
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
g++ sudoku.cpp -lncurses -o sudoku_solver
```

The Python version doesn't require compilation.

## Usage

### Running the Solver

For C++ version:
```bash
./sudoku_solver
```

For Python version:
```bash
python3 sudoku_solver.py
```

### Controls

- Arrow keys: Navigate the puzzle grid
- Numbers 1-9: Input values
- '0' or 'c': Clear current cell (Python version supports both)
- 's': Solve the puzzle (C++ version)
- 'q': Quit the program
- ESC: Quit (Python version only)

## Implementation Details

### Solving Algorithm

The solver implements several Sudoku solving techniques:

1. Standard Elimination (StdElim)
   - Eliminates possibilities in rows, columns, and 3x3 boxes
   - Applies basic Sudoku rules for number placement

2. Linear Elimination (LinElim)
   - Identifies unique candidates in rows, columns, and 3x3 sections
   - Places numbers when only one possible position exists

### Data Structure

- C++ Implementation:
  - Uses a 3D array `board[9][9][9]` to track possible values
  - Each cell maintains its own set of candidate numbers

- Python Implementation:
  - Uses nested dictionaries with lists for possible values
  - More flexible structure allowing for easy modification

## Performance

Both implementations offer real-time solving capabilities for most standard Sudoku puzzles. The C++ version may offer slightly better performance for complex puzzles due to lower-level memory management.

## Contributing

Contributions are welcome! Here are some ways you can contribute:

1. Implement additional solving techniques
2. Improve the user interface
3. Add puzzle generation functionality
4. Enhance error handling
5. Write tests
6. Improve documentation

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Special thanks to the ncurses library developers
- Inspired by classic Sudoku solving techniques
- Built with love for puzzle enthusiasts and developers alike
