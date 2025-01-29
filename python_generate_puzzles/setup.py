#!/usr/bin/env python

from setuptools import setup, Extension
import pybind11
import sys

# Define platform-specific compiler and linker flags
extra_compile_args = []
extra_link_args = []

if sys.platform == "win32":
    extra_compile_args = ['/EHsc', '/std:c++14', '/O2']
    # Optionally add static runtime
    # extra_compile_args.extend(['/MT'])
else:
    extra_compile_args = ['-std=c++11', '-O3']
    if sys.platform == "darwin":
        extra_compile_args.extend(['-stdlib=libc++'])

ext_modules = [
    Extension(
        "sudoku_solver",
        ["sudoku_wrapper.cpp", "sudoku.cpp", "generatepuzzle.cpp"],
        include_dirs=[pybind11.get_include()],
        language='c++',
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
    ),
]

setup(
    name="sudoku_solver",
    ext_modules=ext_modules,
    python_requires=">=3.6",
)
