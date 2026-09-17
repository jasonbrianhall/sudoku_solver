#ifndef SUDOKU_WRAPPER_H
#define SUDOKU_WRAPPER_H

#include "sudoku.h"
#include <array>
#include <deque>
#include <string>

// Which algorithm the stepped solver has reached; values match the
// execution order used by SolveToAlgorithm() (and by the original
// AlgorithmType enum in win_main.cpp / sudoku_game.cpp).
enum class AlgorithmType {
    ALG_NONE = 0,
    ALG_STD_ELIM = 1,
    ALG_LINE_ELIM = 2,
    ALG_HIDDEN_SINGLES = 3,
    ALG_HIDDEN_PAIRS = 4,
    ALG_POINTING_PAIRS = 5,
    ALG_X_WING = 6,
    ALG_SWORDFISH = 7,
    ALG_NAKED_SETS = 8,
    ALG_XY_WING = 9,
    ALG_XYZ_WING = 10,
    ALG_SIMPLE_COLORING = 11,
    ALG_SOLVE_ALL = 12
};

// Portable, non-managed port of the C++/CLI `SudokuWrapper` ref class that
// used to live at the top of win_main.cpp / sudoku_game.cpp. Owns a native
// Sudoku board plus the UI-facing state that isn't part of the puzzle
// itself: which cells are original clues ("immutable"), which cells the
// player has earned a lock on ("quasi-immutable", game app only), and a
// bounded undo history.
class SudokuWrapper {
public:
    SudokuWrapper();
    ~SudokuWrapper();

    SudokuWrapper(const SudokuWrapper&) = delete;
    SudokuWrapper& operator=(const SudokuWrapper&) = delete;

    Sudoku* NativeSudoku() { return nativeSudoku; }

    int savedElapsedSeconds = 0;

    // --- Undo history -----------------------------------------------
    void SaveBoardState();
    bool Undo();
    bool CanUndo() const;
    int GetUndoCount() const;
    void ClearUndoHistory();

    // --- Board editing -------------------------------------------------
    void ClearBoardExceptImmutable();
    void SetValue(int x, int y, int value);
    int GetValue(int x, int y);
    void ClearValue(int x, int y);
    void NewGame();
    void Solve();
    void Clean();
    bool IsValidSolution();

    // --- Immutability ----------------------------------------------
    void MarkPuzzleAsGenerated();
    bool IsCellImmutable(int col, int row) const;
    bool IsCellQuasiImmutable(int col, int row) const;
    void SetQuasiImmutable(int col, int row);
    bool IsCellLocked(int col, int row) const;
    void ClearImmutability();

    // --- File I/O --------------------------------------------------------
    // Saves/loads a ".imm" sidecar file alongside the board file that
    // records which cells are original clues, plus the elapsed timer
    // value (game app only; solver app just passes 0).
    bool LoadFromFile(const std::string& filename);
    void SaveToFile(const std::string& filename, int elapsedSecs = 0);
    void ExportToExcelXML(const std::string& filename);

    // --- Solving techniques -------------------------------------------
    void StdElim();
    void LinElim();
    void FindHiddenSingles();
    void FindHiddenPairs();
    void FindPointingPairs();
    void FindNakedSets();
    void FindXWing();
    void FindSwordFish();
    void FindXYWing();
    void FindXYZWing();
    void FindSimpleColoring();

    AlgorithmType CurrentAlgorithm() const { return currentAlgorithm; }
    void SetCurrentAlgorithm(AlgorithmType a) { currentAlgorithm = a; }

    // Runs algorithms in order (basic eliminations first, then advanced
    // techniques one at a time) up to and including targetAlgorithm.
    void SolveToAlgorithm(AlgorithmType targetAlgorithm);

private:
    Sudoku* nativeSudoku;
    AlgorithmType currentAlgorithm = AlgorithmType::ALG_NONE;

    std::array<std::array<bool, 9>, 9> immutableCells{};
    std::array<std::array<bool, 9>, 9> quasiImmutableCells{};

    struct BoardState {
        int board[9][9][9];
    };
    static const size_t MAX_UNDO_STATES = 100;
    std::deque<BoardState> undoStack;
};

#endif // SUDOKU_WRAPPER_H
