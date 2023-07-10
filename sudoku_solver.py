#!/usr/bin/env python

import curses
import time

def init_board(board={}):
	print("Hello World")
	for x in range(1,10):
		board[x]={}
		for y in range(1,10):
			board[x][y]=-1

def main(stdscr):
	# Initialization
	curses.curs_set(0)	# Hide the cursor
	stdscr.nodelay(True)  # Set non-blocking input

	# Calculate delay in seconds for 10 refreshes per second
	delay = 1 / 10
	board={}
	init_board(board=board)

	# Main loop
	while True:
		# Clear the screen
		stdscr.clear()

		# Get user input
		key = stdscr.getch()

		# Quit the program if 'q' is pressed
		if key == ord('q'):
			break

		# Display a message
		stdscr.addstr(0, 0, "Welcome to Sudoku Solver (Press 's' to solve, 0 to clear, 'q' to quit, and numbers to fill in the current position)\n\n")

		for y in range(1,10):
			for x in range(1,10):
				if not board[x][y]==-1:
					stdscr.addstr(y*2+1,(x-1)*4+4, str(board[x][y]))
				else:				
					stdscr.addstr(y*2+1,(x-1)*4+4, "0")

		for y in range(1,10):
			stdscr.addstr(y*2,2, "-"*37)
			for x in range(1,11):
				stdscr.addstr((y*2)+1,(x-1)*4+2, "|")
		stdscr.addstr(10*2,2, "-"*37)
				


		# Refresh the screen
		stdscr.refresh()

		# Sleep for the specified delay
		time.sleep(delay)

if __name__ == '__main__':
	# Initialize curses and run the main function
	curses.wrapper(main)

