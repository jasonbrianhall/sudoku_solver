#!/bin/bash

echo "Compile Linux Version"
pushd linux_cli
make
popd

echo "Compiling MSDOS Version"
pushd msdos
make
popd

echo "Compiling Various Python Versions"
pushd python_generate_puzzles
make
popd

echo "Compiling Windows CLI Versions"
pushd windows_cli
make
popd

echo -en "\n\nTo compile the windows form version, open SudokuSolver.vcxproj in Visual Studio
or run \"msbuild /p:Configuration=Release /p:Platform=x64 SudokuSolver.vcxproj\" from a Visual Studio command prompt\n\n"

