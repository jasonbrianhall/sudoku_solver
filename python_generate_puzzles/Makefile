.PHONY: all clean install help

all: clean build

help:
	@echo "*** WARNING ****"
	@echo
	@echo "Requires about 16 GB of RAM to compile; crashes my Raspberry PI"
	@echo "which has 8 GB of RAM"
	@echo
	@echo "Please run this in a virtual environment:"
	@echo "python -m venv ~/venv/python3; . ~/venv/python3/bin/activate"
	@echo
	@echo "*** WARNING ***"
	@echo
	@echo "Available targets:"
	@echo "  make        - Clean and build the project"
	@echo "  make clean  - Remove build artifacts"
	@echo "  make install- Install dependencies"
	@echo "  make help   - Show this help message"
	@echo
	@echo "Available programs after build:"
	@echo "  generatepuzzles.py    - Generates Sudoku Puzzles for MS-Word"
	@echo "  sudoku_game.py        - Python version of the NCurses CLI Game"
	@echo "  sudoku_game_qt5.py    - Sudoku Solver written in QT5/Python (GUI)"
	@echo "  sudoku_player_qt5.py  - Sudoku User Game for timed puzzles"
	@echo
	@echo "Example usage for puzzle generator:"
	@echo "  python generatepuzzles.py --easy 10 --medium 5 --hard 3 \\"
	@echo "    --expert 3 --extreme 2 --output my_sudoku_puzzles.docx"

install:
	@echo "Installing Dependencies"
	pip install -r requirements.txt

clean:
	@echo "Cleaning up previous builds"
	rm -rf build/
	rm -f *.so
	rm -f *.pyc
	rm -f sudoku_solver*.so

build:
	@echo "Building python/c++ hybrid"
	python setup.py build_ext --inplace --force
