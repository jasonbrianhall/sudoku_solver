import random
from docx import Document
from docx.shared import Inches
import sudoku_solver
import tempfile
import os

class SudokuPuzzleGenerator:
    def __init__(self):
        self.solver = sudoku_solver.Sudoku()
        
    def generate_puzzle(self, difficulty=0.5):
        """Generate a new Sudoku puzzle with given difficulty (0.0-1.0)"""
        self.solver.new_game()
        
        # First, create a solved puzzle
        numbers = list(range(9))
        # Fill in diagonal boxes first (these don't affect each other)
        for i in range(0, 9, 3):
            box_numbers = numbers.copy()
            random.shuffle(box_numbers)
            for row in range(3):
                for col in range(3):
                    self.solver.set_value(i + row, i + col, box_numbers[row * 3 + col])
        
        # Solve the rest
        if self.solver.solve() != 0:
            raise Exception("Failed to generate valid puzzle")
            
        # Now remove numbers to create the puzzle
        cells = [(i, j) for i in range(9) for j in range(9)]
        cells_to_remove = int(difficulty * 50)  # Adjust number based on difficulty
        random.shuffle(cells)
        
        # Store solution
        solution = [[self.solver.get_value(i, j) for j in range(9)] for i in range(9)]
        
        # Remove cells
        for i, j in cells[:cells_to_remove]:
            temp = self.solver.get_value(i, j)
            self.solver.set_value(i, j, -1)  # Clear cell
            
            # Check if puzzle still has unique solution
            if not self._has_unique_solution():
                self.solver.set_value(i, j, temp)  # Restore value
                
        return self._get_current_grid(), solution
    
    def _has_unique_solution(self):
        """Check if current puzzle state has a unique solution"""
        # Save current state
        temp_file = tempfile.NamedTemporaryFile(delete=False)
        self.solver.save_to_file(temp_file.name)
        
        # Try to solve
        result = self.solver.solve()
        if result != 0:
            return False
            
        # Restore state
        self.solver.load_from_file(temp_file.name)
        os.unlink(temp_file.name)
        return True
    
    def _get_current_grid(self):
        """Get current grid state"""
        return [[self.solver.get_value(i, j) for j in range(9)] for i in range(9)]

    def create_word_document(self, num_puzzles=1, filename="sudoku_puzzles.docx"):
        """Create a Word document with specified number of puzzles"""
        doc = Document()
        doc.add_heading('Sudoku Puzzles', 0)
        
        for puzzle_num in range(num_puzzles):
            puzzle, solution = self.generate_puzzle()
            
            # Add puzzle number
            doc.add_heading(f'Puzzle {puzzle_num + 1}', level=1)
            
            # Create puzzle table
            table = doc.add_table(rows=9, cols=9)
            table.style = 'Table Grid'
            
            # Fill puzzle
            for i in range(9):
                for j in range(9):
                    cell = table.cell(i, j)
                    value = puzzle[i][j]
                    cell.text = str(value + 1) if value != -1 else ''
                    
            doc.add_paragraph()  # Add space
            
            # Add solution if desired
            doc.add_heading(f'Solution {puzzle_num + 1}', level=2)
            table = doc.add_table(rows=9, cols=9)
            table.style = 'Table Grid'
            
            # Fill solution
            for i in range(9):
                for j in range(9):
                    cell = table.cell(i, j)
                    cell.text = str(solution[i][j] + 1)
                    
            doc.add_page_break()
            
        doc.save(filename)

if __name__ == "__main__":
    generator = SudokuPuzzleGenerator()
    generator.create_word_document(num_puzzles=5)
