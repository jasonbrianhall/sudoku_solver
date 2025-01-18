#include "generatepuzzle.h"
#include <cstring>
#include <algorithm>
#include <chrono>

PuzzleGenerator::PuzzleGenerator(Sudoku& s) : sudoku(s), 
    difficultyLevels{
        {"easy",    {50, 55, false, false, false, false}},
        {"medium",  {36, 49, false, false, false, false}},
        {"hard",    {32, 35, true,  false, false, false}},
        {"expert",  {28, 31, true,  true,  true,  false}},
        {"extreme", {24, 27, true,  true,  true,  true}}
    } {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

bool PuzzleGenerator::isUnique(const std::vector<std::pair<int, int>>& removedCells) {
    int backup[9][9][9];
    memcpy(backup, sudoku.board, sizeof(backup));

    if (sudoku.Solve() != 0) {
        memcpy(sudoku.board, backup, sizeof(backup));
        return false;
    }

    int solution[9][9];
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            solution[i][j] = sudoku.GetValue(i, j);
        }
    }

    memcpy(sudoku.board, backup, sizeof(backup));
    
    for (const auto& cell : removedCells) {
        for (int val = 0; val < 9; val++) {
            if (val != solution[cell.first][cell.second]) {
                sudoku.SetValue(cell.first, cell.second, val);
                if (sudoku.Solve() == 0) {
                    memcpy(sudoku.board, backup, sizeof(backup));
                    return false;
                }
                memcpy(sudoku.board, backup, sizeof(backup));
            }
        }
    }

    return true;
}

bool PuzzleGenerator::generateValidSolution() {
    const int maxAttempts = 100;
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        sudoku.NewGame();
        
        // Create a list of all cells
        std::vector<std::pair<int, int>> cells;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                cells.emplace_back(i, j);
            }
        }
        std::shuffle(cells.begin(), cells.end(), rng);
        
        // Try to fill 25 random cells with valid values
        int filledCells = 0;
        for (const auto& cell : cells) {
            if (filledCells >= 25) break;
            
            std::vector<int> values(9);
            for (int i = 0; i < 9; i++) values[i] = i;
            std::shuffle(values.begin(), values.end(), rng);
            
            for (int val : values) {
                if (sudoku.LegalValue(cell.first, cell.second, val)) {
                    sudoku.SetValue(cell.first, cell.second, val);
                    filledCells++;
                    break;
                }
            }
        }
        
        // Try to solve the puzzle
        if (sudoku.Solve() == 0) {
            return true;
        }
    }
    return false;
}

int PuzzleGenerator::countClues() {
    int count = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (sudoku.GetValue(i, j) != -1) count++;
        }
    }
    return count;
}

bool PuzzleGenerator::requiresAdvancedTechnique(const std::string& technique) {
    int backup[9][9][9];
    memcpy(backup, sudoku.board, sizeof(backup));

    int result;
    do {
        result = sudoku.StdElim();
        if (result < 0) break;
        
        result = sudoku.LinElim();
        if (result < 0) break;
        
        result = sudoku.FindHiddenSingles();
        if (result < 0) break;
        
        result = sudoku.FindHiddenPairs();
        if (result < 0) break;
        
        if (technique != "x-wing" && result == 0)
            result = sudoku.FindXWing();
        if (technique != "swordfish" && result == 0)
            result = sudoku.FindSwordFish();
        if (technique != "xy-wing" && result == 0)
            result = sudoku.FindXYWing();
        if (technique != "xyz-wing" && result == 0)
            result = sudoku.FindXYZWing();
            
    } while (result > 0);

    bool needsTechnique = !sudoku.IsValidSolution();
    memcpy(sudoku.board, backup, sizeof(backup));
    return needsTechnique;
}

bool PuzzleGenerator::generatePuzzle(const std::string& difficulty) {
    auto diffIt = difficultyLevels.find(difficulty);
    if (diffIt == difficultyLevels.end()) {
        return false;
    }
    const DifficultySettings& settings = diffIt->second;

    const int maxAttempts = 50;
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        if (!generateValidSolution()) {
            continue;
        }

        int backup[9][9][9];
        memcpy(backup, sudoku.board, sizeof(backup));

        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                positions.emplace_back(i, j);
            }
        }
        std::shuffle(positions.begin(), positions.end(), rng);

        std::vector<std::pair<int, int>> removedCells;
        int targetClues = settings.minClues + 
            (rng() % (settings.maxClues - settings.minClues + 1));

        while (countClues() > targetClues && !positions.empty()) {
            auto pos = positions.back();
            positions.pop_back();

            int cellBackup[9][9][9];
            memcpy(cellBackup, sudoku.board, sizeof(cellBackup));
            
            sudoku.ClearValue(pos.first, pos.second);
            removedCells.push_back(pos);

            bool valid = isUnique(removedCells);
            
            if (valid && settings.allowXWing) {
                valid = requiresAdvancedTechnique("x-wing");
            }
            if (valid && settings.allowSwordfish) {
                valid = requiresAdvancedTechnique("swordfish");
            }
            if (valid && settings.allowXYWing) {
                valid = requiresAdvancedTechnique("xy-wing");
            }
            if (valid && settings.allowXYZWing) {
                valid = requiresAdvancedTechnique("xyz-wing");
            }

            if (!valid) {
                memcpy(sudoku.board, cellBackup, sizeof(cellBackup));
                removedCells.pop_back();
            }
        }

        if (countClues() >= settings.minClues && countClues() <= settings.maxClues) {
            return true;
        }

        // If we didn't get a valid puzzle, restore the board and try again
        memcpy(sudoku.board, backup, sizeof(backup));
    }

    return false;
}
