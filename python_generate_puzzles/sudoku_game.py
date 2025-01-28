#!/usr/bin/env python

# This code uses the Sudoku C++ Wrapper as an example what you can do with the API

import curses
import sys
from sudoku_solver import Sudoku, PuzzleGenerator

class SudokuGame:
    def __init__(self):
        self.game = Sudoku()
        self.x_pos = 0
        self.y_pos = 0
        self.header_lines = 3
        self.show_invalid = False         

    def show_help(self, stdscr):
        stdscr.clear()
        
        stdscr.addstr("Sudoku Help\n\n")
        
        stdscr.addstr("Game Controls:\n")
        stdscr.addstr(" Arrow keys - Move cursor    F5/Shift - Save/Load Slot 1\n")
        stdscr.addstr(" 1-9       - Fill number     F6/Shift - Save/Load Slot 2\n")
        stdscr.addstr(" 0         - Clear cell      F7/Shift - Save/Load Slot 3\n")
        stdscr.addstr(" Q         - Quit game       F8/Shift - Save/Load Slot 4\n")
        stdscr.addstr("\n")
        
        stdscr.addstr("Puzzle Generation:\n")
        stdscr.addstr(" F1          - Generate Easy puzzle\n")
        stdscr.addstr(" F2          - Generate Medium puzzle\n")
        stdscr.addstr(" F3          - Generate Hard puzzle\n")
        stdscr.addstr(" F4          - Generate Expert puzzle\n")
        stdscr.addstr(" Shift+F1    - Generate Extreme puzzle\n")
        stdscr.addstr(" F5-F8       - Save Puzzle\n")
        stdscr.addstr(" Shift F5-F8 - Load Puzzle\n")
        stdscr.addstr(" F9-F12      - Save Puzzle as XML Spreadsheet (Excel compatible)\n")
        stdscr.addstr("\n")
        
        stdscr.addstr("Solving Techniques:\n")
        stdscr.addstr(" S - Standard elimination    N - Hidden singles    Y - Find XY Wing\n")
        stdscr.addstr(" L - Line elimination        K - Naked sets        ; - Find XYZ Wing\n")
        stdscr.addstr(" I - Hidden pairs            X - X-Wing            C - Simple Coloring\n")
        stdscr.addstr(" P - Pointing pairs          F - Swordfish         Z - New Game\n")
        stdscr.addstr(" A - Run all techniques\n")
        stdscr.addstr("\n")
        
        stdscr.addstr("Press any key to return to game...")
        
        stdscr.refresh()
        stdscr.getch()  # Wait for keypress
        stdscr.clear()  # Clear help screen before returning

    def init_colors(self):
        curses.start_color()
        curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)    # 3x3 borders
        curses.init_pair(2, curses.COLOR_BLUE, curses.COLOR_BLACK)   # Numbers
        curses.init_pair(3, curses.COLOR_WHITE, curses.COLOR_BLACK)  # Inner grid lines

    def print_header(self, stdscr):
        stdscr.addstr("Welcome to Sudoku Solver.\n")
        stdscr.addstr("Press (H) for Help\n")

    def draw_grid(self, stdscr):
        for y in range(9):

            stdscr.move(self.header_lines + y * 2, 0)
            # Draw horizontal lines
            if y % 3 == 0:
                stdscr.attron(curses.color_pair(1))
                stdscr.addstr("-" * 37)
                stdscr.attroff(curses.color_pair(1))
            else:
                stdscr.attron(curses.color_pair(3))
                for i in range(37):
                    if i % 4 == 0:
                        if i % 12 == 0:
                            stdscr.attroff(curses.color_pair(3))
                            stdscr.attron(curses.color_pair(1))
                            stdscr.addstr("+")
                            stdscr.attroff(curses.color_pair(1))
                            stdscr.attron(curses.color_pair(3))
                        else:
                            stdscr.addstr("+")
                    else:
                        stdscr.addstr("-")
                stdscr.attroff(curses.color_pair(3))
            stdscr.addstr("\n")

            # Draw cells and vertical lines
            for x in range(9):
                if x % 3 == 0:
                    stdscr.attron(curses.color_pair(1))
                    stdscr.addstr("|")
                    stdscr.attroff(curses.color_pair(1))
                else:
                    stdscr.attron(curses.color_pair(3))
                    stdscr.addstr("|")
                    stdscr.attroff(curses.color_pair(3))

                value = self.game.get_value(x, y)
                if 0 <= value <= 8:
                    stdscr.attron(curses.color_pair(2))
                    stdscr.addstr(f" {value + 1} ")
                    stdscr.attroff(curses.color_pair(2))
                else:
                    stdscr.addstr("   ")

            stdscr.attron(curses.color_pair(1))
            stdscr.addstr("|\n")
            stdscr.attroff(curses.color_pair(1))

        # Draw bottom border
        stdscr.attron(curses.color_pair(1))
        stdscr.addstr("-" * 37)
        stdscr.attroff(curses.color_pair(1))

        if self.show_invalid:
            stdscr.addstr("\n")  # New line after the grid
            stdscr.attron(curses.color_pair(1))  # Use red color for warning
            stdscr.addstr("Invalid solution! Please check your entries.")
            stdscr.attroff(curses.color_pair(1))



    def handle_input(self, key):
        if key == curses.KEY_LEFT:
            self.x_pos = (self.x_pos + 8) % 9
        elif key == curses.KEY_RIGHT:
            self.x_pos = (self.x_pos + 1) % 9
        elif key == curses.KEY_UP:
            self.y_pos = (self.y_pos + 8) % 9
        elif key == curses.KEY_DOWN:
            self.y_pos = (self.y_pos + 1) % 9
        elif key == ord('0'):
            self.game.clear_value(self.x_pos, self.y_pos)
        elif ord('1') <= key <= ord('9'):
            self.game.clean()
            self.game.set_value(self.x_pos, self.y_pos, key - ord('1'))
        elif key in [ord(c) for c in 'Hh']:
            self.game.show_intro()

        elif key in [ord(c) for c in 'Qq']:
            return False
        elif key in [ord(c) for c in 'Ss']:
            self.game.std_elim()
        elif key in [ord(c) for c in 'Ll']:
            self.game.lin_elim()
        elif key in [ord(c) for c in 'Cc']:
            self.game.find_simple_coloring()
        elif key in [ord(c) for c in 'Ii']:
            self.game.find_hidden_pairs()
        elif key in [ord(c) for c in 'Pp']:
            self.game.find_pointing_pairs()
        elif key in [ord(c) for c in 'Xx']:
            self.game.find_x_wing()
        elif key in [ord(c) for c in 'Yy']:
            self.game.find_xy_wing()
        elif key == ord(';'):
            self.game.find_xyz_wing()
        elif key in [ord(c) for c in 'Ff']:
            self.game.find_sword_fish()
        elif key in [ord(c) for c in 'Nn']:
            self.game.find_hidden_singles()
        elif key in [ord(c) for c in 'Kk']:
            self.game.find_naked_sets()
        elif key in [ord(c) for c in 'Aa']:
            self.game.solve()
        elif key in [ord(c) for c in 'Zz']:
            self.game.new_game()
        # Function key handling
        elif key == curses.KEY_F1:
            generator = PuzzleGenerator(self.game)
            generator.generate_puzzle("easy")
            self.game.clean()
        elif key == curses.KEY_F1 + 12:
            generator = PuzzleGenerator(self.game)
            generator.generate_puzzle("extreme")
            self.game.clean()
        elif key == curses.KEY_F2:
            generator = PuzzleGenerator(self.game)
            generator.generate_puzzle("medium")
            self.game.clean()
        elif key == curses.KEY_F3:
            generator = PuzzleGenerator(self.game)
            generator.generate_puzzle("hard")
            self.game.clean()
        elif key == curses.KEY_F4:
            generator = PuzzleGenerator(self.game)
            generator.generate_puzzle("expert")
            self.game.clean()
        elif key == curses.KEY_F5:
            self.game.save_to_file("sudoku_1.txt")
        elif key == curses.KEY_F6:
            self.game.save_to_file("sudoku_2.txt")
        elif key == curses.KEY_F7:
            self.game.save_to_file("sudoku_3.txt")
        elif key == curses.KEY_F8:
            self.game.save_to_file("sudoku_4.txt")
        elif key == curses.KEY_F5+12:
            self.game.load_from_file("sudoku_1.txt")
        elif key == curses.KEY_F6+12:
            self.game.load_from_file("sudoku_2.txt")
        elif key == curses.KEY_F7+12:
            self.game.load_from_file("sudoku_3.txt")
        elif key == curses.KEY_F8+12:
            self.game.load_from_file("sudoku_4.txt")
        elif key == curses.KEY_F9:
            self.game.export_to_excel_xml("puzzle1.xml")
        elif key == curses.KEY_F10:
            self.game.export_to_excel_xml("puzzle2.xml")
        elif key == curses.KEY_F11:
            self.game.export_to_excel_xml("puzzle3.xml")
        elif key == curses.KEY_F12:
            self.game.export_to_excel_xml("puzzle4.xml")
        self.show_invalid = not self.game.is_valid_solution()            
        return True

    def get_cursor_position(self):
        cursor_y = self.header_lines + self.y_pos * 2 + 1
        cursor_x = self.x_pos * 4 + 2
        return cursor_y, cursor_x

    def main(self, stdscr):
        curses.curs_set(1)  # Show cursor
        stdscr.keypad(True)
        self.init_colors()
        stdscr.clear()

        self.print_header(stdscr)

        while True:
            self.draw_grid(stdscr)        
            cursor_y, cursor_x = self.get_cursor_position()
            stdscr.move(cursor_y, cursor_x)
            stdscr.refresh()

            key = stdscr.getch()
            if not self.handle_input(key):
                break

def main():
    game = SudokuGame()
    curses.wrapper(game.main)

if __name__ == "__main__":
    main()
