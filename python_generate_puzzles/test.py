#!/usr/bin/env python

from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                           QGridLayout, QVBoxLayout, QHBoxLayout, QLabel, 
                           QMessageBox, QMenuBar, QMenu, QAction, QSizePolicy)
from PyQt5.QtCore import Qt, QSize, QTimer
from PyQt5.QtGui import QFont

import sys
from sudoku_solver import Sudoku, PuzzleGenerator

class SudokuButton(QPushButton):
    def __init__(self, x, y, parent=None):
        super().__init__(parent)
        self.x = x
        self.y = y
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setMinimumSize(QSize(50, 50))
        self.setFont(QFont('Arial', 14))
        self.value = None
        self.original = False
        self.is_mistake = False
        self.clicked.connect(self.handleClick)  # Connect the click signal
        
    def handleClick(self):
        # Called when button is clicked normally
        if not self.original:
            self.incrementValue()
        
    def setValue(self, value, is_original=False):
        self.value = value
        # Only set as original if it has a non-zero value
        self.original = is_original and value >= 0
        
        # Don't display negative values
        if value >= 0:
            self.setText(str(value + 1))
            if self.original:
                self.setStyleSheet("QWidget { font-weight: bold; color: black; }")
            else:
                self.setStyleSheet("")
        else:
            self.setText("")
            self.setStyleSheet("")
            
    def setMistake(self, is_mistake):
        self.is_mistake = is_mistake
        if is_mistake:
            self.setStyleSheet("QWidget { background-color: #ffcccc; }")
        elif not self.original:
            self.setStyleSheet("")
            
    def incrementValue(self):
        if not self.original:
            if self.value is None or self.value == 8:
                self.updateCell(-1)  # Wrap around to 1 (stored as 0)
            else:
                self.updateCell(self.value + 1)
            
    def decrementValue(self):
        if not self.original:
            if self.value is None or self.value == 0:
                self.value=-1
            else:
                if self.value<=-1:
                     self.value=8
                else:
                     self.value--
                
        
    def updateCell(self, value):
        if not self.original:
            self.value = value
            window = self.window()
            if isinstance(window, SudokuWindow):
                if value is None:
                    window.game.clear_value(self.x, self.y)
                else:
                    if 0 <= value <= 8:
                        window.game.set_value(self.x, self.y, value)
                if value is not None:
                    self.setText(str(value + 1))
                else:
                    self.setText("")
                window.checkCompletion()
            if self.is_mistake:
                self.setMistake(False)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            if not self.original:
                self.incrementValue()
        elif event.button() == Qt.RightButton:
            if not self.original:
                self.decrementValue()
        elif event.button() == Qt.MiddleButton:
            if not self.original:
                self.updateCell(None)
        event.accept()
        
    def wheelEvent(self, event):
        if not self.original:
            delta = event.angleDelta().y()
            if delta > 0:  # Scroll up
                self.incrementValue()
            else:  # Scroll down
                self.decrementValue()
        event.accept()

class SudokuWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.game = Sudoku()
        self.solution = None
        self.timer = QTimer(self)  # Pass self as parent
        self.timer.timeout.connect(self.updateTimer)
        self.elapsed_time = 0
        self.original_puzzle = [[None for _ in range(9)] for _ in range(9)]
        self.initUI()
        self.generatePuzzle('easy')  # Generate easy puzzle by default
        
    def initUI(self):
        self.setWindowTitle('Sudoku Game')
        self.setMinimumSize(600, 700)
        
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        
        # Create menu bar
        self.createMenus()
        
        # Create top bar with timer and check mistakes button
        top_bar = QHBoxLayout()
        
        self.check_mistakes_btn = QPushButton('Check for Mistakes')
        self.check_mistakes_btn.clicked.connect(self.checkMistakes)
        top_bar.addWidget(self.check_mistakes_btn)
        
        self.timer_label = QLabel('Time: 00:00')
        self.timer_label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
        top_bar.addWidget(self.timer_label)
        
        main_layout.addLayout(top_bar)
        
        # Create grid layout for Sudoku board
        grid_widget = QWidget()
        grid_widget.setStyleSheet("background-color: white;")
        self.grid = QGridLayout(grid_widget)
        self.grid.setSpacing(0)
        
        # Create buttons for each cell
        self.buttons = [[None for _ in range(9)] for _ in range(9)]
        for y in range(9):
            for x in range(9):
                button = SudokuButton(x, y, grid_widget)  # Pass parent widget
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
        for i in range(9):
            self.grid.setColumnStretch(i, 1)
            self.grid.setRowStretch(i, 1)
            
        main_layout.addWidget(grid_widget)
        
        # Status label
        self.status_label = QLabel()
        self.status_label.setStyleSheet("color: red;")
        main_layout.addWidget(self.status_label)

    def createMenus(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu('File')
        
        new_menu = file_menu.addMenu('New Game')
        
        difficulties = {
            'Easy': ('easy', 'Ctrl+1'),
            'Medium': ('medium', 'Ctrl+2'),
            'Hard': ('hard', 'Ctrl+3'),
            'Expert': ('expert', 'Ctrl+4'),
            'Extreme': ('extreme', 'Ctrl+5')
        }
        
        for label, (diff, shortcut) in difficulties.items():
            action = QAction(label, self)
            action.setShortcut(shortcut)
            action.triggered.connect(lambda checked, d=diff: self.generatePuzzle(d))
            new_menu.addAction(action)
            
        file_menu.addSeparator()
        
        quit_action = QAction('Quit', self)
        quit_action.setShortcut('Ctrl+Q')
        quit_action.triggered.connect(self.close)
        file_menu.addAction(quit_action)

    def generatePuzzle(self, difficulty):
        # Stop existing timer if running
        self.timer.stop()
        self.elapsed_time = 0
        self.timer_label.setText('Time: 00:00')
        
        # Generate new puzzle
        generator = PuzzleGenerator(self.game)
        generator.generate_puzzle(difficulty)
        
        # Create a copy of the current puzzle and solve it
        temp_game = Sudoku()
        for x in range(9):
            for y in range(9):
                value = self.game.get_value(x, y)
                if value is not None:
                    temp_game.set_value(x, y, value)
        temp_game.solve()
        self.solution = [[temp_game.get_value(x, y) for x in range(9)] for y in range(9)]
        
        # Store original puzzle state
        self.original_puzzle = [[self.game.get_value(x, y) for x in range(9)] for y in range(9)]
        
        # Update display
        self.updateDisplay()
        
        # Start timer
        self.timer.start(1000)

    def updateDisplay(self):
        for y in range(9):
            for x in range(9):
                value = self.game.get_value(x, y)
                is_original = self.original_puzzle[y][x] is not None
                self.buttons[y][x].setValue(value, is_original)

    def updateTimer(self):
        self.elapsed_time += 1
        minutes = self.elapsed_time // 60
        seconds = self.elapsed_time % 60
        self.timer_label.setText(f'Time: {minutes:02d}:{seconds:02d}')

    def checkMistakes(self):
        for y in range(9):
            for x in range(9):
                current_value = self.game.get_value(x, y)
                if current_value is not None and current_value != self.solution[y][x]:
                    self.buttons[y][x].setMistake(True)

    def checkCompletion(self):
        if self.game.is_valid_solution():
            filled = all(self.game.get_value(x, y) != -1 
                        for x in range(9) for y in range(9))
            if filled:
                self.timer.stop()
                QMessageBox.information(self, "Congratulations!", 
                                      f"You solved the puzzle in {self.timer_label.text()[6:]}!")

def main():
    app = QApplication(sys.argv)
    window = SudokuWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
