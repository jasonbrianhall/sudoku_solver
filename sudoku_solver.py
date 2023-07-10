#!/usr/bin/env python

import curses
import time
import sys
import json

def init_board(board={}):
	for x in range(1,10):
		board[x]={}
		for y in range(1,10):
			board[x][y]=[1,2,3,4,5,6,7,8,9]
		
		
		
		
def update_board(board, x, y, value):
	# Update the specified cell with the given value

	if value<=0 or value>=10:
		board[x][y]=[1,2,3,4,5,6,7,8,9]
	else:
		if value in board[x][y]: 	
			board[x][y] = [value]
	
	for x in range(1,10):
		for y in range(1,10):
			if not len(board[x][y])==1:
				board[x][y]=[1,2,3,4,5,6,7,8,9]
	
	for x in range(1,10):
		for y in range(1,10):
			if len(board[x][y])==1:
				value=board[x][y][0]
				# Remove the value from the corresponding row
				for i in range(1, 10):
					if i != y and value in board[x][i]:
						board[x][i].remove(value)
			
				# Remove the value from the corresponding column
				for i in range(1, 10):
					if i != x and value in board[i][y]:
						board[i][y].remove(value)

				# Calculate the starting positions of the 3x3 area
				start_x = 1 if x <= 3 else 4 if x <= 6 else 7
				start_y = 1 if y <= 3 else 4 if y <= 6 else 7
		    
				# Remove the value from the corresponding 3x3 area
				for i in range(start_x, start_x + 3):
					for j in range(start_y, start_y + 3):
						if (i, j) != (x, y) and value in board[i][j]:
							board[i][j].remove(value)

	for k in range(1,10):
		counter=0
		for x in range(1,10):
			for y in range(1,10):
				if len(board[x][y])==1 and board[x][y][0]==k:
					counter+=1
		if counter==8:
			for x in range(1,10):
				for y in range(1,10):
					if k in board[x][y] and len(board[x][y])>1:
						board[x][y]==[k]
						

				
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
	screenupdate=False
	
	while True:
		# Clear the screen
		if screenupdate==True:
			stdscr.clear()
			screenupdate=False

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
			update_board(board, cursorx, cursory, key-ord("0"))
			screenupdate=True
		elif key == ord("c"):
			init_board(board)
			screenupdate=True

		# Display a message
		stdscr.addstr(0, 0, "Welcome to Sudoku Solver (Press 'c' to clear, 0 to clear, 'q' to quit, and numbers to fill in the current position)")

		for y in range(1,10):
			for x in range(1,10):
				if len(board[x][y])==1:
					stdscr.addstr(y*2+1,(x-1)*4+6, str(board[x][y][0]))
		
		for y in range(1,10):
			stdscr.addstr(y*2+1, 2, str(y))  # Add Y-axis numbering

		for x in range(1,10):
			stdscr.addstr(21, (x-1)*4+6, str(x))  # Add X-axis numbering'''
		


		for y in range(1, 11):
			stdscr.addstr(y*2, 4, "-"*37)
			for x in range(1, 11):
				if y<10:
					stdscr.addstr((y*2)+1, (x-1)*4+4, "|")

		
		stdscr.move(cursory*2+1,(cursorx-1)*4+6)


		# Refresh the screen
		stdscr.refresh()

		# Sleep for the specified delay
		time.sleep(delay)

if __name__ == '__main__':
	# Initialize curses and run the main function
	curses.wrapper(main)
	'''board={}
	init_board(board)
	#for x in range(1,9):
		#setValue(board, x, 1, x)
	#lineElim(board)
	update_board(board, 1, 1, 1)
	update_board(board, 1, 1, 0)
	print(ord("0"))
	
		
	print(json.dumps(board, indent=5))
	'''	
	
		
	

