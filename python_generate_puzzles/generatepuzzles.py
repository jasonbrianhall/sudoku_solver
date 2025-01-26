from sudoku_generator import SudokuPuzzleGenerator

# Create 5 puzzles in a Word document
generator = SudokuPuzzleGenerator()
generator.create_word_document(num_puzzles=5, filename="my_sudoku_puzzles.docx")

