#!/usr/bin/env python

import curses
import time
import sys

def init_board(board={}):
	print("Hello World")
	for x in range(1,10):
		board[x]={}
		for y in range(1,10):
			board[x][y]=-1

def setValue(board, x,y, value):
	if value<=0 or value>=10:
		value=-1
	board[x][y]=value
	

def main(stdscr):
	# Initialization
	curses.curs_set(2)
	stdscr.nodelay(True)  # Set non-blocking input

	# Calculate delay in seconds for 10 refreshes per second
	delay = 1 / 10
	board={}
	init_board(board=board)
	# Main loop
	cursorx=1
	cursory=1
	
	
	while True:
		# Clear the screen
		stdscr.clear()

		# Get user input
		key = stdscr.getch()

		# Quit the program if 'q' is pressed
		if key == ord('q') or key==27:
			break
		elif key == curses.KEY_LEFT:
			cursorx-=1
			if cursorx<=0:
				cursorx=9
		elif key == curses.KEY_RIGHT:
			cursorx+=1
			if cursorx>=10:
				cursorx=1
		elif key == curses.KEY_UP:
			cursory-=1
			if cursory<=0:
				cursory=9
		elif key == curses.KEY_DOWN:
			cursory+=1
			if cursory>=10:
				cursory=1
		elif key >= ord("0") and key<= ord("9"):
			setValue(board, cursorx, cursory, key-ord("0"))


		# Display a message
		stdscr.addstr(0, 0, "Welcome to Sudoku Solver (Press 's' to solve, 0 to clear, 'q' to quit, and numbers to fill in the current position)\n\n")

		for y in range(1,10):
			for x in range(1,10):
				if not board[x][y]==-1:
					stdscr.addstr(y*2+1,(x-1)*4+4, str(board[x][y]))
					
		for y in range(1,10):
			stdscr.addstr(y*2,2, "-"*37)
			for x in range(1,11):
				stdscr.addstr((y*2)+1,(x-1)*4+2, "|")
		stdscr.addstr(10*2,2, "-"*37)
		stdscr.move(cursory*2+1,(cursorx-1)*4+4)


		# Refresh the screen
		stdscr.refresh()

		# Sleep for the specified delay
		time.sleep(delay)

if __name__ == '__main__':
	# Initialize curses and run the main function
	curses.wrapper(main)

