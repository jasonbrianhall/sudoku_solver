#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sudoku.h"

namespace py = pybind11;

PYBIND11_MODULE(sudoku_solver, m) {
    py::class_<Sudoku>(m, "Sudoku")
        .def(py::init<>())
        .def("new_game", &Sudoku::NewGame)
        .def("set_value", &Sudoku::SetValue)
        .def("get_value", &Sudoku::GetValue)
        .def("solve", &Sudoku::Solve)
        .def("is_valid_solution", &Sudoku::IsValidSolution)
        .def("export_to_excel_xml", &Sudoku::ExportToExcelXML)
        .def("load_from_file", &Sudoku::LoadFromFile)
        .def("save_to_file", &Sudoku::SaveToFile);
}

void Sudoku::print_debug(const char *format, ...) {}
