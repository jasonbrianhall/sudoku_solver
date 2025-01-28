#!/usr/bin/env python
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                           QGridLayout, QVBoxLayout, QHBoxLayout, QLabel, 
                           QMessageBox, QMenuBar, QMenu, QAction, QFileDialog,
                           QSizePolicy, QLayout)
from PyQt5.QtCore import Qt, QSize, QRect, QPoint
from PyQt5.QtGui import QFont, QPalette, QColor
import sys
from sudoku_solver import Sudoku, PuzzleGenerator

class QFlowLayout(QLayout):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._items = []
        self._hspacing = 0
        self._vspacing = 0
        
    def addItem(self, item):
        self._items.append(item)
        
    def horizontalSpacing(self):
        return self._hspacing
        
    def verticalSpacing(self):
        return self._vspacing
    
    def setHorizontalSpacing(self, spacing):
        self._hspacing = spacing
        
    def setVerticalSpacing(self, spacing):
        self._vspacing = spacing
        
    def count(self):
        return len(self._items)
        
    def itemAt(self, index):
        if 0 <= index < len(self._items):
            return self._items[index]
        return None
        
    def takeAt(self, index):
        if 0 <= index < len(self._items):
            return self._items.pop(index)
        return None
        
    def expandingDirections(self):
        return Qt.Orientations(0)
        
    def hasHeightForWidth(self):
        return True
        
    def heightForWidth(self, width):
        return self._doLayout(QRect(0, 0, width, 0), True)
        
    def setGeometry(self, rect):
        super().setGeometry(rect)
        self._doLayout(rect, False)
        
    def sizeHint(self):
        return self.minimumSize()
        
    def minimumSize(self):
        size = QSize()
        for item in self._items:
            size = size.expandedTo(item.minimumSize())
        margins = self.contentsMargins()
        size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom())
        return size
        
    def _doLayout(self, rect, test_only=False):
        x = rect.x()
        y = rect.y()
        line_height = 0
        spacing_x = self.horizontalSpacing()
        spacing_y = self.verticalSpacing()
        
        for item in self._items:
            style = item.widget().style()
            layout_spacing_x = style.layoutSpacing(
                QSizePolicy.PushButton, QSizePolicy.PushButton, Qt.Horizontal)
            layout_spacing_y = style.layoutSpacing(
                QSizePolicy.PushButton, QSizePolicy.PushButton, Qt.Vertical)
            space_x = spacing_x if spacing_x != -1 else layout_spacing_x
            space_y = spacing_y if spacing_y != -1 else layout_spacing_y
            
            next_x = x + item.sizeHint().width() + space_x
            if next_x - space_x > rect.right() and line_height > 0:
                x = rect.x()
                y = y + line_height + space_y
                next_x = x + item.sizeHint().width() + space_x
                line_height = 0
                
            if not test_only:
                item.setGeometry(QRect(QPoint(x, y), item.sizeHint()))
                
            x = next_x
            line_height = max(line_height, item.sizeHint().height())
            
        return y + line_height - rect.y()

class SudokuButton(QPushButton):
    def __init__(self, x, y, parent=None):
        super().__init__(parent)
        self.x = x
        self.y = y
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setMinimumSize(QSize(50, 50))
        self.setFont(QFont('Arial', 14))
        self.value = None
        
    def setValue(self, value):
        self.value = value
        self.setText(str(value + 1) if value is not None and 0 <= value <= 8 else "")
        
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            # Original behavior - increment
            super().mousePressEvent(event)
        elif event.button() == Qt.RightButton:
            # Decrement value
            if self.value is None:
                value = 8  # Start at 9 when right-clicking empty cell
            else:
                value = (self.value - 1) % 9
            self.value = value
            window = self.window()
            if isinstance(window, SudokuWindow):
                window.game.set_value(self.x, self.y, value)
                window.updateDisplay()
                
class SudokuWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.game = Sudoku()
        self.initUI()
        
    def initUI(self):
        self.setWindowTitle('Sudoku Solver')
        self.setMinimumSize(600, 700)  # Set minimum size instead of fixed
        
        # Create central widget and main layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        
        # Create menu bar
        self.createMenus()
        
        # Create toolbar with solving buttons in a flow layout
        solving_methods = [
            ('Standard\nElimination (S)', self.standardElimination),
            ('Line\nElimination (L)', self.lineElimination),
            ('Hidden\nSingles (N)', self.hiddenSingles),
            ('Hidden\nPairs (H)', self.hiddenPairs),
            ('Pointing\nPairs (P)', self.pointingPairs),
            ('X-Wing\n(X)', self.xWing),
            ('Solve All\n(A)', self.solveAll)
        ]
        
        toolbar_widget = QWidget()
        toolbar_flow = QFlowLayout()
        toolbar_flow.setHorizontalSpacing(5)
        toolbar_flow.setVerticalSpacing(5)
        
        for text, slot in solving_methods:
            btn = QPushButton(text)
            btn.clicked.connect(slot)
            btn.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
            btn.setMinimumWidth(100)
            toolbar_flow.addWidget(btn)
        
        toolbar_widget.setLayout(toolbar_flow)
        main_layout.addWidget(toolbar_widget)
        
        # Create grid layout for Sudoku board
        grid_widget = QWidget()
        grid_widget.setStyleSheet("background-color: white;")
        grid_widget.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        
        self.grid = QGridLayout(grid_widget)
        self.grid.setSpacing(0)
        
        # Create buttons for each cell
        self.buttons = [[None for _ in range(9)] for _ in range(9)]
        for y in range(9):
            for x in range(9):
                button = SudokuButton(x, y)
                button.clicked.connect(self.cellClicked)
                # Add borders
                style = "QWidget { border: 1px solid gray; "
                if x % 3 == 0:
                    style += "border-left: 2px solid black; "
                if x == 8:
                    style += "border-right: 2px solid black; "
                if y % 3 == 0:
                    style += "border-top: 2px solid black; "
                if y == 8:
                    style += "border-bottom: 2px solid black; "
                style += "}"
                button.setStyleSheet(style)
                self.grid.addWidget(button, y, x)
                self.buttons[y][x] = button
        
        # Make grid squares
        self.grid.setContentsMargins(0, 0, 0, 0)
        for i in range(9):
            self.grid.setColumnStretch(i, 1)
            self.grid.setRowStretch(i, 1)
            
        main_layout.addWidget(grid_widget)
        
        # Status label for invalid solution warning
        self.status_label = QLabel()
        self.status_label.setStyleSheet("color: red;")
        main_layout.addWidget(self.status_label)
        
        # Initialize empty board
        self.updateDisplay()

    def createMenus(self):
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu('File')
        
        new_action = QAction('New Game', self)
        new_action.setShortcut('Ctrl+N')
        new_action.triggered.connect(self.newGame)
        file_menu.addAction(new_action)
        
        save_action = QAction('Save Game', self)
        save_action.setShortcut('Ctrl+S')
        save_action.triggered.connect(self.saveGame)
        file_menu.addAction(save_action)
        
        load_action = QAction('Load Game', self)
        load_action.setShortcut('Ctrl+O')
        load_action.triggered.connect(self.loadGame)
        file_menu.addAction(load_action)

        file_menu.addSeparator()
        
        export_action = QAction('Export to Word Document...', self)
        export_action.triggered.connect(self.exportToWord)
        file_menu.addAction(export_action)
        
        # Generate menu
        generate_menu = menubar.addMenu('Generate')
        
        difficulties = {
            'easy': ('Easy (F1)', 'F1'),
            'medium': ('Medium (F2)', 'F2'),
            'hard': ('Hard (F3)', 'F3'),
            'expert': ('Expert (F4)', 'F4'),
            'extreme': ('Extreme (Shift+F1)', 'Shift+F1')
        }
        
        for diff, (label, shortcut) in difficulties.items():
            action = QAction(label, self)
            if shortcut.startswith('Shift+'):
                key = getattr(Qt, f'Key_{shortcut.split("+")[1]}')
                action.setShortcut(Qt.ShiftModifier | key)
            else:
                action.setShortcut(shortcut)
            action.triggered.connect(lambda checked, d=diff: self.generatePuzzle(d))
            generate_menu.addAction(action)

    def exportToWord(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Export to Word", "", "Word Documents (*.docx)")
        if filename:
            if not filename.endswith('.docx'):
                filename += '.docx'
            try:
                # Create a dictionary with counts for current puzzle's difficulty
                current_difficulty = self.getCurrentDifficulty()  # You'll need to track/determine this
                puzzle_counts = {current_difficulty: 1}
                
                # Use the SudokuPuzzleGenerator to create the document
                from sudoku_generator import SudokuPuzzleGenerator
                generator = SudokuPuzzleGenerator()
                generator.create_word_document(puzzle_counts=puzzle_counts, filename=filename)
                
                QMessageBox.information(self, "Success", f"Puzzle exported to {filename}")
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Failed to export puzzle: {str(e)}")

    def handleFunctionKey(self, key):
        difficulty_map = {
            Qt.Key_F1: 'easy',
            Qt.Key_F2: 'medium',
            Qt.Key_F3: 'hard',
            Qt.Key_F4: 'expert'
        }
        # Handle Shift+F1 for extreme
        if key == Qt.Key_F1 and QApplication.keyboardModifiers() & Qt.ShiftModifier:
            return 'extreme'
        return difficulty_map.get(key)

    def keyPressEvent(self, event):
        difficulty = self.handleFunctionKey(event.key())
        if difficulty:
            self.generatePuzzle(difficulty)
        else:
            super().keyPressEvent(event)

    def generatePuzzle(self, difficulty):
        generator = PuzzleGenerator(self.game)
        generator.generate_puzzle(difficulty)
        self.game.clean()
        self.current_difficulty = difficulty  # Track the current puzzle difficulty
        self.updateDisplay()

    def getCurrentDifficulty(self):
        # Return the current puzzle's difficulty level
        return getattr(self, 'current_difficulty', 'medium')  # Default to medium if not set

    def cellClicked(self):
        button = self.sender()
        value = button.value
        # Cycle through numbers 1-9
        value = ((value + 1) if value is not None else 0) % 9
        self.game.set_value(button.x, button.y, value)
        self.updateDisplay()
        
    def updateDisplay(self):
        for y in range(9):
            for x in range(9):
                value = self.game.get_value(x, y)
                self.buttons[y][x].setValue(value)
        
        # Update invalid solution warning
        if not self.game.is_valid_solution():
            self.status_label.setText("Invalid solution! Please check your entries.")
        else:
            self.status_label.setText("")
            
    def standardElimination(self):
        self.game.std_elim()
        self.updateDisplay()
        
    def lineElimination(self):
        self.game.lin_elim()
        self.updateDisplay()
        
    def hiddenSingles(self):
        self.game.find_hidden_singles()
        self.updateDisplay()
        
    def hiddenPairs(self):
        self.game.find_hidden_pairs()
        self.updateDisplay()
        
    def pointingPairs(self):
        self.game.find_pointing_pairs()
        self.updateDisplay()
        
    def xWing(self):
        self.game.find_x_wing()
        self.updateDisplay()
        
    def solveAll(self):
        self.game.solve()
        self.updateDisplay()
        
    def newGame(self):
        self.game.new_game()
        self.updateDisplay()
        
    def saveGame(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Save Game", "", "Sudoku Files (*.sud)")
        if filename:
            if not filename.endswith('.sud'):
                filename += '.sud'
            self.game.save_to_file(filename)
            
    def loadGame(self):
        filename, _ = QFileDialog.getOpenFileName(self, "Load Game", "", "Sudoku Files (*.sud)")
        if filename:
            if not filename.endswith('.sud'):
                filename += '.sud'
            self.game.load_from_file(filename)
            self.updateDisplay()     
                   
    def generatePuzzle(self, difficulty):
        generator = PuzzleGenerator(self.game)
        generator.generate_puzzle(difficulty)
        self.game.clean()
        self.updateDisplay()

def main():
    app = QApplication(sys.argv)
    window = SudokuWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
