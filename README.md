# Sudoku Solver

An interactive command-line Sudoku solver implemented in both C++ and Python, featuring a user-friendly ncurses interface for puzzle input and visualization. The C++ implementation offers robust solving capabilities with multiple advanced techniques, while the Python version serves as a basic prototype.

## Features

- Interactive puzzle input with arrow key navigation
- Real-time validity checking of moves
- Multiple solving techniques ranging from basic to advanced
- Colored display for better visualization
- Cross-platform compatibility (Linux, macOS, Unix-like systems)
- Comprehensive logging of solving steps
- Available in both C++ (full version) and Python (prototype)

## Getting Started

### Prerequisites

- For C++ version (recommended):
  - G++ compiler
  - ncurses library
  - Make (optional, for compilation)

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
g++ sudoku.cpp -lncurses -o sudoku_solver
```

The Python version is a prototype with limited functionality and doesn't require compilation. For the best solving experience, use the C++ implementation.

## Usage

### Running the Solver

For C++ version (recommended):
```bash
./sudoku_solver
```

For Python version (prototype):
```bash
python3 sudoku_solver.py
```

### Controls

- Arrow keys: Navigate the puzzle grid
- Numbers 1-9: Input values
- '0' or 'c': Clear current cell
- 'q': Quit the program
- ESC: Quit (Python version only)

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

### Recommended Solving Strategy

For the most efficient solving experience:

1. Alternate between Standard Elimination (S) and Line Elimination (L) until no more changes occur
2. When stuck, try one of the advanced techniques:
   - Start with Hidden Singles (N) and Hidden Pairs (H)
   - Progress to Pointing Pairs (P) and X-Wing (X)
   - Use Swordfish (F) and Naked Sets (K) for the most challenging situations
3. After finding new possibilities with advanced techniques, return to alternating between Standard and Line Elimination
4. Repeat this process until the puzzle is solved

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
