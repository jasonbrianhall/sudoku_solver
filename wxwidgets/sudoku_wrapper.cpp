#include "sudoku_wrapper.h"
#include "generatepuzzle.h"
#include <cstring>
#include <fstream>

SudokuWrapper::SudokuWrapper() {
    nativeSudoku = new Sudoku();
}

SudokuWrapper::~SudokuWrapper() {
    delete nativeSudoku;
    nativeSudoku = nullptr;
}

// --- Undo history --------------------------------------------------------

void SudokuWrapper::SaveBoardState() {
    if (undoStack.size() < MAX_UNDO_STATES) {
        BoardState state;
        std::memcpy(state.board, nativeSudoku->board, sizeof(state.board));
        undoStack.push_back(std::move(state));
    }
}

bool SudokuWrapper::Undo() {
    if (undoStack.empty()) return false;
    const BoardState& state = undoStack.back();
    std::memcpy(nativeSudoku->board, state.board, sizeof(nativeSudoku->board));
    undoStack.pop_back();
    return true;
}

bool SudokuWrapper::CanUndo() const { return !undoStack.empty(); }
int SudokuWrapper::GetUndoCount() const { return static_cast<int>(undoStack.size()); }
void SudokuWrapper::ClearUndoHistory() { undoStack.clear(); }

// --- Board editing ---------------------------------------------------------

void SudokuWrapper::ClearBoardExceptImmutable() {
    for (int x = 0; x < 9; x++)
        for (int y = 0; y < 9; y++)
            if (!immutableCells[x][y])
                nativeSudoku->ClearValue(x, y);
}

void SudokuWrapper::SetValue(int x, int y, int value) { nativeSudoku->SetValue(x, y, value); }
int SudokuWrapper::GetValue(int x, int y) { return nativeSudoku->GetValue(x, y); }
void SudokuWrapper::ClearValue(int x, int y) { nativeSudoku->ClearValue(x, y); }

void SudokuWrapper::NewGame() {
    nativeSudoku->NewGame();
    currentAlgorithm = AlgorithmType::ALG_NONE;
    ClearImmutability();
}

void SudokuWrapper::Solve() { nativeSudoku->Solve(); }
void SudokuWrapper::Clean() { nativeSudoku->Clean(); }
bool SudokuWrapper::IsValidSolution() { return nativeSudoku->IsValidSolution(); }

// --- Immutability ------------------------------------------------------

void SudokuWrapper::MarkPuzzleAsGenerated() {
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            immutableCells[col][row] = (nativeSudoku->GetValue(col, row) != -1);
            quasiImmutableCells[col][row] = false;
        }
    }
}

bool SudokuWrapper::IsCellImmutable(int col, int row) const {
    if (col >= 0 && col < 9 && row >= 0 && row < 9) return immutableCells[col][row];
    return false;
}

bool SudokuWrapper::IsCellQuasiImmutable(int col, int row) const {
    if (col >= 0 && col < 9 && row >= 0 && row < 9) return quasiImmutableCells[col][row];
    return false;
}

void SudokuWrapper::SetQuasiImmutable(int col, int row) {
    if (col >= 0 && col < 9 && row >= 0 && row < 9) quasiImmutableCells[col][row] = true;
}

bool SudokuWrapper::IsCellLocked(int col, int row) const {
    return IsCellImmutable(col, row) || IsCellQuasiImmutable(col, row);
}

void SudokuWrapper::ClearImmutability() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            immutableCells[i][j] = false;
            quasiImmutableCells[i][j] = false;
        }
    }
}

// --- File I/O ----------------------------------------------------------

bool SudokuWrapper::LoadFromFile(const std::string& filename) {
    bool result = nativeSudoku->LoadFromFile(filename);
    if (result) {
        ClearImmutability();
        std::string sidecar = filename + ".imm";
        std::ifstream fin(sidecar);
        if (fin.is_open()) {
            for (int col = 0; col < 9; col++) {
                for (int row = 0; row < 9; row++) {
                    int v = 0;
                    fin >> v;
                    immutableCells[col][row] = (v == 1);
                }
            }
            int savedTime = 0;
            fin >> savedTime;
            savedElapsedSeconds = savedTime;
            fin.close();
        } else {
            savedElapsedSeconds = 0;
        }
    }
    return result;
}

void SudokuWrapper::SaveToFile(const std::string& filename, int elapsedSecs) {
    nativeSudoku->SaveToFile(filename);
    std::string sidecar = filename + ".imm";
    std::ofstream fout(sidecar);
    if (fout.is_open()) {
        for (int col = 0; col < 9; col++) {
            for (int row = 0; row < 9; row++) {
                fout << (immutableCells[col][row] ? 1 : 0);
                if (row < 8) fout << " ";
            }
            fout << "\n";
        }
        fout << elapsedSecs << "\n";
        fout.close();
    }
}

void SudokuWrapper::ExportToExcelXML(const std::string& filename) {
    nativeSudoku->ExportToExcelXML(filename);
}

// --- Solving techniques --------------------------------------------------

void SudokuWrapper::StdElim() { nativeSudoku->StdElim(); }
void SudokuWrapper::LinElim() { nativeSudoku->LinElim(); }
void SudokuWrapper::FindHiddenSingles() { nativeSudoku->FindHiddenSingles(); }
void SudokuWrapper::FindHiddenPairs() { nativeSudoku->FindHiddenPairs(); }
void SudokuWrapper::FindPointingPairs() { nativeSudoku->FindPointingPairs(); }
void SudokuWrapper::FindNakedSets() { nativeSudoku->FindNakedSets(); }
void SudokuWrapper::FindXWing() { nativeSudoku->FindXWing(); }
void SudokuWrapper::FindSwordFish() { nativeSudoku->FindSwordFish(); }
void SudokuWrapper::FindXYWing() { nativeSudoku->FindXYWing(); }
void SudokuWrapper::FindXYZWing() { nativeSudoku->FindXYZWing(); }
void SudokuWrapper::FindSimpleColoring() { nativeSudoku->FindSimpleColoring(); }

void SudokuWrapper::SolveToAlgorithm(AlgorithmType targetAlgorithm) {
    currentAlgorithm = targetAlgorithm;

    bool changes_made;
    do {
        changes_made = false;

        if (targetAlgorithm >= AlgorithmType::ALG_STD_ELIM) {
            bool basic_changes;
            do {
                basic_changes = false;

                if (targetAlgorithm >= AlgorithmType::ALG_STD_ELIM) {
                    if (nativeSudoku->StdElim() > 0) { basic_changes = true; changes_made = true; }
                    if (targetAlgorithm == AlgorithmType::ALG_STD_ELIM) return;
                }
                if (targetAlgorithm >= AlgorithmType::ALG_LINE_ELIM) {
                    if (nativeSudoku->LinElim() > 0) { basic_changes = true; changes_made = true; }
                    if (targetAlgorithm == AlgorithmType::ALG_LINE_ELIM) return;
                }
            } while (basic_changes);
        }

        if (!changes_made) {
            if (targetAlgorithm >= AlgorithmType::ALG_HIDDEN_SINGLES) {
                if (nativeSudoku->FindHiddenSingles() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_SINGLES) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_SINGLES) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_HIDDEN_PAIRS && !changes_made) {
                if (nativeSudoku->FindHiddenPairs() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_PAIRS) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_PAIRS) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_POINTING_PAIRS && !changes_made) {
                if (nativeSudoku->FindPointingPairs() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_POINTING_PAIRS) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_POINTING_PAIRS) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_X_WING && !changes_made) {
                if (nativeSudoku->FindXWing() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_X_WING) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_X_WING) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_SWORDFISH && !changes_made) {
                if (nativeSudoku->FindSwordFish() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_SWORDFISH) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_SWORDFISH) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_NAKED_SETS && !changes_made) {
                if (nativeSudoku->FindNakedSets() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_NAKED_SETS) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_NAKED_SETS) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_XY_WING && !changes_made) {
                if (nativeSudoku->FindXYWing() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_XY_WING) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_XY_WING) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_XYZ_WING && !changes_made) {
                if (nativeSudoku->FindXYZWing() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_XYZ_WING) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_XYZ_WING) return;
            }
            if (targetAlgorithm >= AlgorithmType::ALG_SIMPLE_COLORING && !changes_made) {
                if (nativeSudoku->FindSimpleColoring() > 0) {
                    changes_made = true;
                    if (targetAlgorithm == AlgorithmType::ALG_SIMPLE_COLORING) return;
                    continue;
                }
                if (targetAlgorithm == AlgorithmType::ALG_SIMPLE_COLORING) return;
            }
        }
    } while (changes_made);
}
