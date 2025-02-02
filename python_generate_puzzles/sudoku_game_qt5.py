#!/usr/bin/env python

from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QPushButton, 
                           QGridLayout, QVBoxLayout, QHBoxLayout, QLabel, 
                           QMessageBox, QMenuBar, QMenu, QAction, QFileDialog,
                           QSizePolicy, QLayout)
from PyQt5.QtCore import Qt, QSize, QRect, QPoint
from PyQt5.QtGui import QFont, QPalette, QColor, QImage

from PyQt5.QtWidgets import (QDialog, QSpinBox, QDialogButtonBox, QLineEdit, 
                           QVBoxLayout, QHBoxLayout)

import sys
from sudoku_solver import Sudoku, PuzzleGenerator
from PyQt5.QtCore import QThread
from sudoku_ocr import SudokuOCR
import tempfile
import os


class GeneratorThread(QThread):
    def __init__(self, puzzle_counts, filename):
        super().__init__()
        self.puzzle_counts = puzzle_counts
        self.filename = filename

    def run(self):
        from sudoku_generator import SudokuPuzzleGenerator
        generator = SudokuPuzzleGenerator()
        generator.create_word_document(puzzle_counts=self.puzzle_counts, filename=self.filename)


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

        # Enable focus and hover events
        self.setFocusPolicy(Qt.StrongFocus)
        
    def enterEvent(self, event):
        # When mouse enters the button, give it keyboard focus
        self.setFocus()
        super().enterEvent(event)
        
    def leaveEvent(self, event):
        # When mouse leaves, clear focus
        self.clearFocus()
        super().leaveEvent(event)
        
    def keyPressEvent(self, event):
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

        
    def setValue(self, value):
        self.value = value
        self.setText(str(value + 1) if value is not None and 0 <= value <= 8 else "")
        
    def incrementValue(self):
        if self.value is None:  # Empty cell
            value = 0  # Start at 1
        elif self.value == 8:  # At 9
            value = None  # Clear the cell
        else:
            value = self.value + 1
        self.updateCell(value)
        
    def decrementValue(self):
        if self.value<0:  # Empty cell
            value = 8  # Go to 9 (stored as 8)
        elif self.value == 0:  # At 1
            value = None  # Clear the cell
        else:
            value = self.value - 1
        self.updateCell(value)
        
    def updateCell(self, value):
        self.value = value
        window = self.window()
        if isinstance(window, SudokuWindow):
            if value is None:
                window.game.clear_value(self.x, self.y)
            else:
                # Ensure value is between 0 and 8 before setting
                if 0 <= value <= 8:
                    window.game.set_value(self.x, self.y, value)
            window.updateDisplay()
        
    def wheelEvent(self, event):
        delta = event.angleDelta().y()
        if delta > 0:  # Scroll up
            self.incrementValue()
        else:  # Scroll down
            self.decrementValue()
        event.accept()
        
    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.incrementValue()
        elif event.button() == Qt.RightButton:
            self.decrementValue()
        elif event.button() == Qt.MiddleButton:
            self.updateCell(None)
        # Mark the event as handled
        event.accept()
                
class GeneratePuzzlesDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Generate Puzzles")
        self.setModal(True)
        
        layout = QVBoxLayout(self)
        
        # Create spinboxes for each difficulty
        self.spinboxes = {}
        for difficulty in ['easy', 'medium', 'hard', 'expert', 'extreme']:
            hlayout = QHBoxLayout()
            label = QLabel(f"{difficulty.title()} puzzles:")
            spinbox = QSpinBox()
            spinbox.setRange(0, 100)
            spinbox.setValue(0)
            hlayout.addWidget(label)
            hlayout.addWidget(spinbox)
            layout.addLayout(hlayout)
            self.spinboxes[difficulty] = spinbox
            
        # Add export to Word section
        hlayout = QHBoxLayout()
        self.filename_edit = QLineEdit("sudoku_puzzles.docx")
        browse_button = QPushButton("Browse...")
        browse_button.clicked.connect(self.browse)
        hlayout.addWidget(QLabel("Output file:"))
        hlayout.addWidget(self.filename_edit)
        hlayout.addWidget(browse_button)
        layout.addLayout(hlayout)
        
        # Add buttons
        button_box = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)
        
    def browse(self):
        filename, _ = QFileDialog.getSaveFileName(
            self, "Save Word Document", "", "Word Documents (*.docx)")
        if filename:
            if not filename.endswith('.docx'):
                filename += '.docx'
            self.filename_edit.setText(filename)
            
    def get_puzzle_counts(self):
        return {diff: spin.value() for diff, spin in self.spinboxes.items()}
        
    def get_filename(self):
        return self.filename_edit.text()
                
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
            ('Standard\nElimination', self.standardElimination),
            ('Line\nElimination', self.lineElimination),
            ('Hidden\nSingles', self.hiddenSingles),
            ('Hidden\nPairs', self.hiddenPairs),
            ('Pointing\nPairs', self.pointingPairs),
            ('X-Wing\n', self.xWing),
            ('XY-Wing\n', self.xyWing),
            ('XYZ-Wing\n', self.xyzWing),
            ('Sword Fish\n', self.swordfish),
            ('Solve All\n', self.solveAll)
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
    
        generate_doc_action = QAction('Generate Puzzles to Word...', self)
        generate_doc_action.triggered.connect(self.generatePuzzlesToWord)
        file_menu.addAction(generate_doc_action)        

        load_action = QAction('OCR from Screenshot...', self)
        load_action.setShortcut('Ctrl+I')
        load_action.triggered.connect(self.importFromScreenshot)
        file_menu.addAction(load_action)

        load_action = QAction('OCR from Image...', self)
        load_action.setShortcut('Ctrl+I')
        load_action.triggered.connect(self.importFromImageFile)
        file_menu.addAction(load_action)


        file_menu.addSeparator()
    
        generate_doc_action = QAction('Genete Excel Compatible XML Spreadsheet...', self)
        generate_doc_action.triggered.connect(self.exportToExcel)
        file_menu.addAction(generate_doc_action)        


        file_menu.addSeparator()  # Add separator before Quit
        
        quit_action = QAction('Quit', self)
        quit_action.setShortcut('Ctrl+Q')  # Standard quit shortcut
        quit_action.triggered.connect(self.close)  # QMainWindow's close method
        file_menu.addAction(quit_action)


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

    def importFromImageFile(self):
        """Import Sudoku puzzle from an image file with enhanced error handling."""
    
        # Define supported image formats
        formats = {
            "PNG Files (*.png)": ".png",
            "JPEG Files (*.jpg *.jpeg)": ".jpg",
            "Bitmap Files (*.bmp)": ".bmp",
            "All Image Files (*.png *.jpg *.jpeg *.bmp)": ""
        }
    
        filter_string = ";;".join(formats.keys())
    
        filename, selected_filter = QFileDialog.getOpenFileName(
            self,
            "Import Sudoku from Image",
            "",  # Start in current directory
            filter_string
        )
    
        if not filename:
            return  # User cancelled
        
        try:
            # Verify file exists
            if not os.path.exists(filename):
                raise FileNotFoundError(f"Image file not found: {filename}")
            
            # Verify file is an image
            image = QImage(filename)
            if image.isNull():
                raise ValueError("Selected file is not a valid image")
            
            # Initialize OCR with the image file
            ocr = SudokuOCR(filename)
        
            # Process the image and get the grid
            grid = ocr.process()
        
            if not grid or len(grid) != 9 or any(len(row) != 9 for row in grid):
                raise ValueError("Failed to detect a valid 9x9 Sudoku grid in the image")
        
            # Clear current game
            self.game.new_game()
        
            # Track how many numbers were successfully imported
            numbers_imported = 0
        
            # Load the extracted grid into the game
            for y in range(9):
                for x in range(9):
                    value = grid[y][x]
                    if value != 0:  # Only set non-empty cells
                        if 1 <= value <= 9:       
                            # Convert from 1-9 to 0-8 for internal representation
                            self.game.set_value(x, y, value - 1)
                            numbers_imported += 1
                        else:
                            raise ValueError(f"Invalid number detected: {value}")
        
            # Update the display
            self.updateDisplay()
        
            # Show success message with statistics
            QMessageBox.information(
                self,
                "Import Successful",
                f"Successfully imported Sudoku puzzle!\n\n"
                f"Numbers detected: {numbers_imported}\n"
                f"Empty cells: {81 - numbers_imported}"
            )
        
        except FileNotFoundError as e:
            QMessageBox.critical(
                self,
                "File Error",
                f"Could not find or open the image file:\n{str(e)}"
            )
        except ValueError as e:
            QMessageBox.critical(
                self,
                "Import Error",
                f"Invalid image or Sudoku grid:\n{str(e)}"
            )
        except Exception as e:
            QMessageBox.critical(
                self,
                "Unexpected Error",
                f"An unexpected error occurred while importing:\n{str(e)}\n\n"
                "Please ensure the image is clear and contains a valid Sudoku puzzle."
            )

    def importFromScreenshot(self):
        """Import Sudoku puzzle from the clipboard (screenshot)."""
        clipboard = QApplication.clipboard()
        image = clipboard.image()
    
        if image.isNull():
            QMessageBox.warning(
                self,
                "No Screenshot",
                "No image found in clipboard. Please take a screenshot first."
            )
            return
        
        try:
            # Save the clipboard image to a temporary file
        
            temp_dir = tempfile.gettempdir()
            temp_path = os.path.join(temp_dir, 'sudoku_screenshot.png')
            image.save(temp_path, 'PNG')
        
            # Initialize OCR with the temporary image
            ocr = SudokuOCR(temp_path)
        
            # Process the image and get the grid
            grid = ocr.process()
        
            # Clean up temporary file
            try:
                os.remove(temp_path)
            except:
                pass  # Ignore cleanup errors
        
            # Clear current game
            self.game.new_game()
        
            # Load the extracted grid into the game
            for y in range(9):
                for x in range(9):
                    value = grid[y][x]
                    if value != 0:  # Only set non-empty cells
                        # Convert from 1-9 to 0-8 for internal representation
                        self.game.set_value(x, y, value - 1)
        
            # Update the display
            self.updateDisplay()
        
            QMessageBox.information(
                self,
                "Import Successful",
                "Sudoku puzzle has been imported from your screenshot."
            )
        
        except Exception as e:
            QMessageBox.critical(
                self,
                "Import Error",
                f"Failed to import Sudoku puzzle from screenshot:\n{str(e)}"
            )


    def generatePuzzlesToWord(self):
        dialog = GeneratePuzzlesDialog(self)
        if dialog.exec_() == QDialog.Accepted:
            puzzle_counts = dialog.get_puzzle_counts()
            filename = dialog.get_filename()
        
            # Check if at least one puzzle was requested
            if not any(puzzle_counts.values()):
                QMessageBox.warning(self, "No Puzzles", 
                                  "Please specify at least one puzzle to generate.")
                return
            
            # Create and run thread
            self.generator_thread = GeneratorThread(puzzle_counts, filename)
            self.generator_thread.finished.connect(
                lambda: QMessageBox.information(self, "Success", 
                                            f"Puzzles successfully generated and saved to:\n{filename}"))
            self.generator_thread.start()

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
        
    def newGame(self):
        self.game.new_game()
        self.updateDisplay()
       
    def exportToExcel(self):
            filename, _ = QFileDialog.getSaveFileName(self, "Export to Excel XML", "", "Excel XML Files (*.xml)")
            if filename:
                if not filename.endswith('.xml'):
                    filename += '.xml'
                self.game.export_to_excel_xml(filename)
        
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
