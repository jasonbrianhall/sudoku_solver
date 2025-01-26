from sudoku_generator import SudokuPuzzleGenerator

# Create 5 puzzles in a Word document with specified difficulty
generator = SudokuPuzzleGenerator()
generator.create_word_document(num_puzzles=5, 
                             filename="my_sudoku_puzzles.docx",
                             difficulty="easy")  # Can be: easy, medium, hard, expert, extreme, ultraextreme
