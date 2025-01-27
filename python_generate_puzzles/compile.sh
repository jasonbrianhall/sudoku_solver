#!/bin/bash
pip install -r requirements.txt

rm -rf build/
rm -f *.so
rm -f *.pyc
rm -f sudoku_solver*.so

python setup.py build_ext --inplace --force


python generatepuzzles.py --easy 10 --medium 5 --hard 3 --expert 3 --extreme 2 --output my_sudoku_puzzles.docx
