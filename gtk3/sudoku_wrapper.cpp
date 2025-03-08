#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sudoku.h"
#include "generatepuzzle.h"

namespace py = pybind11;

PYBIND11_MODULE(sudoku_solver, m) {
    py::class_<Sudoku>(m, "Sudoku")
        // Constructor
        .def(py::init<>())
        
        // Core Game Functions
        .def("new_game", &Sudoku::NewGame)
        .def("set_value", &Sudoku::SetValue)
        .def("get_value", &Sudoku::GetValue)
        .def("clear_value", &Sudoku::ClearValue)
        .def("load_from_file", &Sudoku::LoadFromFile)
        .def("save_to_file", &Sudoku::SaveToFile)
        
        // Main Solving Functions
        .def("solve", &Sudoku::Solve)
        .def("solve_basic", &Sudoku::SolveBasic)
        .def("legal_value", &Sudoku::LegalValue)
        .def("is_valid_solution", &Sudoku::IsValidSolution)
        
        // Debug and Logging
        .def("log_board", &Sudoku::LogBoard)
        //.def("print_debug", &Sudoku::print_debug)
        
        // Basic Solving Techniques
        .def("std_elim", &Sudoku::StdElim)
        .def("lin_elim", &Sudoku::LinElim)
        .def("find_hidden_singles", &Sudoku::FindHiddenSingles)
        
        // Advanced Solving Techniques
        .def("find_hidden_pairs", &Sudoku::FindHiddenPairs)
        .def("find_pointing_pairs", &Sudoku::FindPointingPairs)
        .def("find_naked_sets", &Sudoku::FindNakedSets)
        
        // Expert Solving Techniques
        .def("find_x_wing", &Sudoku::FindXWing)
        .def("find_sword_fish", &Sudoku::FindSwordFish)
        .def("find_xy_wing", &Sudoku::FindXYWing)
        .def("find_xyz_wing", &Sudoku::FindXYZWing)
        .def("find_simple_coloring", &Sudoku::FindSimpleColoring)
        .def("clean", &Sudoku::Clean)
        
        // Export Functions
        .def("export_to_excel_xml", &Sudoku::ExportToExcelXML);

    py::class_<PuzzleGenerator>(m, "PuzzleGenerator")
        .def(py::init<Sudoku&>())
        .def("generate_puzzle", &PuzzleGenerator::generatePuzzle);
}

void Sudoku::print_debug(const char *format, ...) {}
