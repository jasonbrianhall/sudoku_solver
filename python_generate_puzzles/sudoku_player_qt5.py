#!/usr/bin/env python

from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                           QGridLayout, QVBoxLayout, QHBoxLayout, QLabel, 
                           QMessageBox, QMenuBar, QMenu, QAction, QSizePolicy)
from PyQt5.QtCore import Qt, QSize, QTimer
from PyQt5.QtGui import QFont

import sys
from sudoku_solver import Sudoku, PuzzleGenerator
from besttimes import LeaderboardDialog, save_best_time

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
        self.current_difficulty = 'easy'

        # Enable focus and hover events
        self.setFocusPolicy(Qt.StrongFocus)
        
    def enterEvent(self, event):
        # When mouse enters the button, give it keyboard focus
        if not self.original:
            self.setFocus()
        super().enterEvent(event)
        
    def leaveEvent(self, event):
        # When mouse leaves, clear focus
        self.clearFocus()
        super().leaveEvent(event)
        
    def keyPressEvent(self, event):
        if not self.original:
            key = event.text()
            if key.isdigit():
                num = int(key)
                if 1 <= num <= 9:
                    # Convert 1-9 input to 0-8 internal value
                    self.updateCell(num - 1)
                else:
                    self.updateCell(None)
            elif event.key() == Qt.Key_Delete or event.key() == Qt.Key_Backspace:
                self.updateCell(None)
        super().keyPressEvent(event)
        
    def handleClick(self):
        # Called when button is clicked normally
        if not self.original:
            self.incrementValue()
        
    def setValue(self, value, is_original=False):
        self.value = value
        # Only set as original if it has a non-zero value
        self.original = is_original and value is not None and value > 0
    
        # Don't display zero values
        if value >= 0:
            self.setText(str(value + 1))
            if is_original:
                self.setStyleSheet(self.styleSheet() + "QWidget { font-weight: bold; }")
        else:
            self.setText("")
            self.setStyleSheet(self.styleSheet().replace("font-weight: bold;", ""))
            
    def setMistake(self, is_mistake):
        self.is_mistake = is_mistake
    
        # Build the style string
        style = "QWidget { border: 1px solid gray; "
    
        # Add thick borders for 3x3 grid based on position
        if self.x % 3 == 0:
            style += "border-left: 2px solid black; "
        if self.x == 8:
            style += "border-right: 2px solid black; "
        if self.y % 3 == 0:
            style += "border-top: 2px solid black; "
        if self.y == 8:
            style += "border-bottom: 2px solid black; "
    
        # Add background color based on mistake state
        if is_mistake:
            style += "background-color: #ffcccc; "
        else:
            style += "background-color: white; "
    
        # Add font weight if original
        if self.original:
            style += "font-weight: bold; "
        
        style += "}"
        self.setStyleSheet(style)
            
    def incrementValue(self):
        if self.value is None:  # Empty cell
            value = 0  # Start at 1
        elif self.value == 8:  # At 9
            value = -1  # Clear the cell
        else:
            value = self.value + 1
        self.updateCell(value)
        
    def decrementValue(self):
        if self.value is None or self.value<0:  # Empty cell
            value = 8  # Start at 1
        elif self.value == 0:  # At 1
            value = -1  # Clear the cell
        else:
            value = self.value - 1
        self.updateCell(value)
            
    def updateCell(self, value):
        if not self.original:
            self.value = value
            window = self.window()
            if isinstance(window, SudokuWindow):
                if value is None:
                    window.game.clear_value(self.x, self.y)
                else:
                    if value>=0:
                        window.game.set_value(self.x, self.y, value)
                if not value==None and value >= 0:
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
                self.updateCell(-1)
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
        self.grid.setSpacing(0)  # Remove space between cells
        self.grid.setContentsMargins(0, 0, 0, 0)  # Remove margins
    
        # Create buttons for each cell
        self.buttons = [[None for _ in range(9)] for _ in range(9)]
        for y in range(9):
            for x in range(9):
                button = SudokuButton(x, y, grid_widget)
            
                # Build the style string
                style = "QWidget { border: 1px solid gray; "
            
                # Add thick borders for 3x3 grid
                if x % 3 == 0:
                    style += "border-left: 2px solid black; "
                if x == 8:
                    style += "border-right: 2px solid black; "
                if y % 3 == 0:
                    style += "border-top: 2px solid black; "
                if y == 8:
                    style += "border-bottom: 2px solid black; "
                
                style += "background-color: white; }"
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
        
        view_menu = menubar.addMenu('View')
        best_times_action = QAction('Best Times', self)
        best_times_action.setShortcut('Ctrl+B')
        best_times_action.triggered.connect(self.showBestTimes)
        view_menu.addAction(best_times_action)

        # Add Cheat menu
        cheat_menu = menubar.addMenu('Cheat')
    
        # Add all solving techniques
        solving_techniques = [
            ('Standard Elimination', self.standardElimination, 'Ctrl+E'),
            ('Line Elimination', self.lineElimination, 'Ctrl+L'),
            ('Hidden Singles', self.hiddenSingles, 'Ctrl+H'),
            ('Hidden Pairs', self.hiddenPairs, 'Ctrl+P'),
            ('Pointing Pairs', self.pointingPairs, 'Ctrl+T'),
            ('X-Wing', self.xWing, 'Ctrl+X'),
            ('XY-Wing', self.xyWing, 'Ctrl+Y'),
            ('XYZ-Wing', self.xyzWing, 'Ctrl+Z'),
            ('Sword Fish', self.swordfish, 'Ctrl+F'),
            ('Solve All', self.solveAll, 'Ctrl+S')
        ]

        for label, slot, shortcut in solving_techniques:
            action = QAction(label, self)
            action.setShortcut(shortcut)
            action.triggered.connect(slot)
            cheat_menu.addAction(action)

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

    def xyWing(self):
        self.game.find_xy_wing()
        self.updateDisplay()

    def xyzWing(self):
        self.game.find_xyz_wing()
        self.updateDisplay()

    def swordfish(self):
        self.game.find_sword_fish()
        self.updateDisplay()

    def solveAll(self):
        self.game.solve()
        self.updateDisplay()

    def showBestTimes(self):
        dialog = LeaderboardDialog(self)
        dialog.exec_()


    def generatePuzzle(self, difficulty):
        self.current_difficulty = difficulty
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
                if value != -1:
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
                is_original = self.original_puzzle[y][x] !=-1
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
                if current_value !=-1 and current_value != self.solution[y][x]:
                    self.buttons[y][x].setMistake(True)

    def checkCompletion(self):
        if self.game.is_valid_solution():
            filled = all(self.game.get_value(x, y) >= 0 
                    for x in range(9) for y in range(9))
            if filled:
                self.timer.stop()
                made_top_10, rank, initials = save_best_time(
                    self, 
                    self.current_difficulty, 
                    self.elapsed_time
                )
        
                minutes = self.elapsed_time // 60
                seconds = self.elapsed_time % 60
                time_str = f"{minutes:02d}:{seconds:02d}"
        
                message = f"You solved the puzzle in {time_str}!"
                if made_top_10:
                    message += f"\nCongratulations {initials}! "
                    message += f"You made the leaderboard at rank #{rank}!"
            
                QMessageBox.information(self, "Puzzle Completed!", message)
            
                # Show the leaderboard if made top 10
                if made_top_10:
                    self.showBestTimes()
            
                # Generate a new puzzle with the same difficulty
                self.generatePuzzle(self.current_difficulty)
            
def main():
    app = QApplication(sys.argv)
    window = SudokuWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
