from sudoku_generator import SudokuPuzzleGenerator

# Create puzzles in a Word document with different difficulty levels
generator = SudokuPuzzleGenerator()

# Specify number of puzzles for each difficulty level
puzzle_counts = {
    'easy': 10,    # 10 easy puzzles
    'medium': 5,   # 5 medium puzzles
    'hard': 3,     # 3 hard puzzles
    'extreme': 2   # 2 extreme puzzles
}

# Generate the document with all puzzles
generator.create_word_document(
    puzzle_counts=puzzle_counts,
    filename="my_sudoku_puzzles.docx"
)
