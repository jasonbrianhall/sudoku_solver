#!/usr/bin/env python

import curses
import time
import sys
import json

def init_board(board={}):
	for x in range(1,10):
		board[x]={}
		for y in range(1,10):
			board[x][y]=-1

def lineElim(board):
	for y in range(1,10):
		countvalues=0
		sumdata=0
		for x in range(1,10):
			if board[x][y]>=1 and board[x][y]<=9:
				sumdata+=board[x][y]
				countvalues+=1
		if countvalues==8:
			# 9*10/2 = 45
			remaining=45-sumdata
			for x in range(1,10):
				if board[x][y]==-1:
					setValue(board, x, y, remaining)
					break
	for x in range(1,10):
		countvalues=0
		sumdata=0
		for y in range(1,10):
			if board[x][y]>=1 and board[x][y]<=9:
				sumdata+=board[x][y]
				countvalues+=1
		if countvalues==8:
			# 9*10/2 = 45
			remaining=45-sumdata
			for y in range(1,10):
				if board[x][y]==-1:
					setValue(board, x, y, remaining)
					break
					
def squareElim(board):
	for k in 1,4,7:
		for l in 1,4,7:
			countvalues=0
			sumdata=0
			for x in range(k,k+3):
				for y in range(l,l+3):
					if board[x][y]>=1 and board[x][y]<=9:
						sumdata+=board[x][y]
						countvalues+=1
			#print(countvalues, sumdata)
			if countvalues==8:
				remaining=45-sumdata
				for x in range(k,k+3):
					for y in range(l,l+3):
						if board[x][y]==-1:
							setValue(board, x, y, remaining)
							break
		

				
def setValue(board, x,y, value):
	if value<=0 or value>=10:
		value=-1
		board[x][y]=-1
		return
		
	isvalid=True
	for newx in range(1,10):
		if board[newx][y]==value:
			isvalid=False
			break
	if isvalid==True:
		for newy in range(1,10):
			if board[x][newy]==value:
				isvalid=False
				break

	if isvalid==True:
		l=int((x-1)/3)*3+1
		k=int((y-1)/3)*3+1

		for newx in range(k,k+3):
			for newy in range(l,l+3):
				if board[newx][newy]==value:
					isvalid=False
					break

	
	if isvalid==True:	
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
		#stdscr.clear()

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
		elif key == ord("s"):
			lineElim(board)
			squareElim(board)


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
	'''board={}
	init_board(board)
	#for x in range(1,9):
		#setValue(board, x, 1, x)
	#lineElim(board)
	setValue(board, 4, 2, 1)
	setValue(board, 5, 3, 1)
	
		
	print(json.dumps(board, indent=5))'''
		
	
		
	

