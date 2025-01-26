from sudoku_solver import Sudoku, PuzzleGenerator
from docx import Document

class SudokuPuzzleGenerator:
    def __init__(self):
        self.solver = Sudoku()
        self.generator = PuzzleGenerator(self.solver)
        
    def generate_puzzle(self, difficulty="easy"):
        """Generate a new Sudoku puzzle with given difficulty
        Available difficulties: easy, medium, hard, expert, extreme, ultraextreme"""
        # Generate the puzzle
        if not self.generator.generate_puzzle(difficulty):
            raise Exception("Failed to generate puzzle with difficulty: " + difficulty)
            
        # Get unsolved puzzle state
        puzzle = self._get_current_grid()
        
        # Solve to get solution
        if self.solver.solve() != 0:
            raise Exception("Failed to solve puzzle")
            
        # Get solution grid
        solution = self._get_current_grid()
        
        return puzzle, solution
    
    def _get_current_grid(self):
        return [[self.solver.get_value(i, j) for j in range(9)] for i in range(9)]

    def create_word_document(self, num_puzzles=1, filename="sudoku_puzzles.docx", difficulty="easy"):
        """Create a Word document with specified number of puzzles
        difficulty can be: easy, medium, hard, expert, extreme, ultraextreme"""
        doc = Document()
        doc.add_heading('Sudoku Puzzles', 0)
        
        for puzzle_num in range(num_puzzles):
            puzzle, solution = self.generate_puzzle(difficulty)
            
            doc.add_heading(f'Puzzle {puzzle_num + 1} ({difficulty})', level=1)
            
            table = doc.add_table(rows=9, cols=9)
            table.style = 'Table Grid'
            
            for i in range(9):
                for j in range(9):
                    cell = table.cell(i, j)
                    value = puzzle[i][j]
                    cell.text = str(value + 1) if value >= 0 else ''
                    
            doc.add_paragraph()
            
            doc.add_heading(f'Solution {puzzle_num + 1}', level=2)
            table = doc.add_table(rows=9, cols=9)
            table.style = 'Table Grid'
            
            for i in range(9):
                for j in range(9):
                    cell = table.cell(i, j)
                    value = solution[i][j]
                    cell.text = str(value + 1) if value >= 0 else ''
                    
            doc.add_page_break()
            
        doc.save(filename)
