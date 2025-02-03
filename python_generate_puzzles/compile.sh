#!/bin/bash

echo -en "*** WARNING ****\n\n"
echo "Requires about 16 GB of RAM to compile; crashes my Raspberry PI"
echo -en "which has 8 GB of RAM\n\n"
echo -en "Plase run this script in a virtual environment (python -m venv ~/venv/python3; . ~/venv/python3/bin/activate\n\n"
echo -en "*** WARNING ***\n\n"

echo "Installing Dependencies"
pip install -r requirements.txt

echo -en "\n\nCleaning up previous builds"
rm -rf build/
rm -f *.so
rm -f *.pyc
rm -f sudoku_solver*.so

echo -en "\n\nBuilding python/c++ hybrid"
python setup.py build_ext --inplace --force

echo -en "\n\nNow you can run various games \n\n \
generatepuzzles.py    --> Generates Sudoku Puzzles for MS-Word \n \
sudoku_game.py        --> Python version of the NCurses CLI Game \n \
sudoku_game_qt5.py    --> Sudoku Solver writen in QT5/Python (GUI)  \n \
sudoku_player_qt5.py  --> Sudoku User Game where you try to solve timed Sudoku puzzles of various difficulties \n \
"

echo -en "\n\nThe python puzzle generator can be ran with something like this\n   --> python generatepuzzles.py --easy 10 --medium 5 --hard 3 --expert 3 --extreme 2 --output my_sudoku_puzzles.docx\n\n"
