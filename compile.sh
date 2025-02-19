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
sh compile.sh
popd

echo "Compiling Windows CLI Versions"
pushd windows_cli
sh compile.sh
popd

echo -en "To compile the windows form version, open SudokuSolver.vcxproj in Visual Studio
or run \"msbuild /p:Configuration=Release /p:Platform=x64 SudokuSolver.vcxproj\" from a Visual Studio command prompt\n\n"

