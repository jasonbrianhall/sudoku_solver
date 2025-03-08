CC = g++
CFLAGS = -std=c++11 -Wall -O2
GTK_FLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Object files
OBJS = sudoku.o debug.o generatepuzzle.o sudoku_game.o

# Main target
all: sudoku_game

# Rule for building the executable
sudoku_game: $(OBJS)
	$(CC) $(CFLAGS) $(GTK_FLAGS) -o $@ $^ $(GTK_LIBS)

# Rules for building object files
sudoku.o: sudoku.cpp sudoku.h
	$(CC) $(CFLAGS) -c $<

debug.o: debug.cpp sudoku.h
	$(CC) $(CFLAGS) -c $<

generatepuzzle.o: generatepuzzle.cpp generatepuzzle.h sudoku.h
	$(CC) $(CFLAGS) -c $<

sudoku_game.o: sudoku_game.cpp sudoku.h generatepuzzle.h
	$(CC) $(CFLAGS) $(GTK_FLAGS) -c $<

# Python wrapper (if needed)
wrapper: sudoku_wrapper.cpp sudoku.cpp debug.cpp generatepuzzle.cpp
	$(CC) $(CFLAGS) $(shell python3-config --includes) -shared -fPIC -o sudoku_solver.so $^

# Clean target
clean:
	rm -f $(OBJS) sudoku_game sudoku_solver.so

# Install target (optional)
install: sudoku_game
	install -m 755 sudoku_game /usr/local/bin

.PHONY: all clean install wrapper
