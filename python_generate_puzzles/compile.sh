#!/bin/bash
pip install -r requirements.txt

rm -rf build/
rm -f *.so
rm -f *.pyc
rm -f sudoku_solver*.so

python setup.py build_ext --inplace --force
