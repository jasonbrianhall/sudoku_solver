# Sudoku Puzzle Generator

Generate custom Sudoku puzzles of varying difficulties and export them to a Word document. This tool creates both puzzles and their solutions, formatting them in a clean, professional layout suitable for printing or digital distribution.

## Features

- Generate puzzles across multiple difficulty levels:
  - Easy
  - Medium
  - Hard
  - Expert
  - Extreme
- Professional Word document output with:
  - Properly formatted 9x9 grids
  - Clear difficulty labels
  - Separate sections for puzzles and solutions
  - Consistent styling and spacing
  - Print-ready format

## Installation

1. Clone the repository and navigate to the project directory
2. Not required but recommended, make a virtual environment
``` bash
python -m venv ~/sudoku
. sudoku/bin/activate
```
3. Install the required dependencies and compile (look at compile.sh to see what's going on if interested):
```bash
./compile.sh
```

The main requirements are:
- python-docx (for Word document generation)
- pybind11 (for C++ bindings)
- setuptools
- g++ (since this library is dependent on C++ source code linked to the directory)

## Usage

### Command Line Interface

The simplest way to generate puzzles is using the command-line script `generatepuzzles.py`:

```bash
python generatepuzzles.py [--easy N] [--medium N] [--hard N] [--extreme N] [--output filename.docx]
```

Examples:
```bash
# Generate 10 easy puzzles
python generatepuzzles.py --easy 10

# Generate a mix of difficulties
python generatepuzzles.py --easy 5 --medium 3 --hard 2 --extreme 1

# Specify custom output filename
python generatepuzzles.py --medium 5 --output practice_puzzles.docx
```

### Python API

You can also generate puzzles programmatically using the Python API:

```python
from sudoku_generator import SudokuPuzzleGenerator

# Initialize the generator
generator = SudokuPuzzleGenerator()

# Create puzzles with specific counts for each difficulty
puzzle_counts = {
    'easy': 2,
    'medium': 3,
    'hard': 1,
    'extreme': 1
}

# Generate and save to Word document
generator.create_word_document(
    puzzle_counts=puzzle_counts,
    filename="my_sudoku_puzzles.docx"
)
```

To generate a single puzzle:

```python
generator = SudokuPuzzleGenerator()

# Generate one puzzle (returns puzzle and solution)
puzzle, solution = generator.generate_puzzle(difficulty="medium")

# puzzle and solution are 9x9 lists where:
# - Numbers 1-9 represent filled cells
# - -1 represents empty cells
```

## Document Format

The generated Word document includes:

1. Title page with "Sudoku Puzzles" heading
2. Puzzles section containing:
   - Each puzzle on a separate page
   - Difficulty level clearly marked
   - 9x9 grid with proper borders
   - Empty cells for solving
3. Solutions section containing:
   - Each solution on a separate page
   - Matching numbers for puzzle verification
   - Complete filled grids

The grids are formatted with:
- Thicker borders around 3x3 boxes
- Lighter borders between individual cells
- Centered numbers
- Consistent spacing and alignment
- Professional fonts and sizing

## Difficulty Levels

The difficulty levels correspond to different solving techniques required:

- **Easy**: Basic solving techniques (30-35 given numbers)
  - Single candidates
  - Row/column/box elimination
  
- **Medium**: Intermediate techniques (28-30 given numbers)
  - Hidden singles
  - Pointing pairs
  
- **Hard**: Advanced techniques (24-27 given numbers)
  - X-Wing patterns
  - Hidden pairs/triples
  
- **Extreme**: Expert techniques (20-23 given numbers)
  - Swordfish patterns
  - XY-Wing patterns
  - Complex chains

## Error Handling

The generator includes robust error handling:

```python
try:
    generator.create_word_document(puzzle_counts)
except ValueError as e:
    print(f"Invalid difficulty specified: {e}")
except Exception as e:
    print(f"Error generating puzzles: {e}")
```

## Limitations

- Minimum of 17 given numbers per puzzle (mathematically proven minimum for unique solutions)
- Maximum of 50 puzzles recommended per document for reasonable file sizes
- Generation time increases with difficulty level 
- Some extreme puzzles may take longer to generate due to complexity requirements (but on modern machines, just a few seconds)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## License

This project is licensed under the MIT License - see the LICENSE file for details in the root directory.
