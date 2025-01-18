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

bool PuzzleGenerator::generateValidSolution() {
    while (true) {  // Keep going until we succeed
        sudoku.NewGame();
        
        // Create list of all positions
        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                positions.emplace_back(i, j);
            }
        }
        std::shuffle(positions.begin(), positions.end(), rng);
        
        // Try to place up to 50 valid numbers
        for (int placed = 0; placed < 50; placed++) {
            int row = positions[placed].first;
            int col = positions[placed].second;
            
            bool foundValid = false;
            for (int val = 1; val <= 9; val++) {
                sudoku.SetValue(row, col, val);
                if (sudoku.IsValidSolution()) {
                    foundValid = true;
                    break;
                }
            }
            
            if (!foundValid) {
                break;  // Try a new board
            }
            
            // After each valid placement, try solving
            if (sudoku.Solve() == 0) {
                // Check if we have all 81 numbers
                bool allFilled = true;
                for (int i = 0; i < 9 && allFilled; i++) {
                    for (int j = 0; j < 9; j++) {
                        if (sudoku.GetValue(i, j) == -1) {
                            allFilled = false;
                            break;
                        }
                    }
                }
                
                if (allFilled && sudoku.IsValidSolution()) {
                    return true;  // Found a complete valid solution!
                }
            }
        }
        
        // If we get here, try again from the start
    }
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

bool PuzzleGenerator::generatePuzzle(const std::string& difficulty) {
    auto diffIt = difficultyLevels.find(difficulty);
    if (diffIt == difficultyLevels.end()) {
        return false;
    }
    const DifficultySettings& settings = diffIt->second;
    generateValidSolution();
    return true;
    const int maxAttempts = 50;
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        // Generate initial solution
        if (!generateValidSolution()) {
            continue;
        }

        // Store the complete solution
        int solution[9][9][9];
        memcpy(solution, sudoku.board, sizeof(solution));

        // Create list of positions to try removing
        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                positions.emplace_back(i, j);
            }
        }
        std::shuffle(positions.begin(), positions.end(), rng);

        int targetClues = settings.minClues + 
            (rng() % (settings.maxClues - settings.minClues + 1));

        int backup[9][9][9];  // Moved to outer scope
        
        while (countClues() > targetClues && !positions.empty()) {
            auto pos = positions.back();
            positions.pop_back();

            // Backup current state before removing number
            memcpy(backup, sudoku.board, sizeof(backup));
            
            sudoku.ClearValue(pos.first, pos.second);
            
            // Test if puzzle is still solvable
            sudoku.Clean();
            bool valid = (sudoku.Solve() == 0);
            
            if (valid) {
                // Check if all positions were filled
                for (int i = 0; i < 9 && valid; i++) {
                    for (int j = 0; j < 9; j++) {
                        if (sudoku.GetValue(i, j) == -1) {
                            valid = false;
                            break;
                        }
                    }
                }
            }

            // For harder difficulties, check if required techniques are needed
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
                // Invalid removal - restore previous state
                memcpy(sudoku.board, backup, sizeof(backup));
                continue;
            }

            // Valid removal - restore to unsolved state
            memcpy(sudoku.board, backup, sizeof(backup));
            sudoku.ClearValue(pos.first, pos.second);
        }

        // Final verification that puzzle is solvable
        memcpy(backup, sudoku.board, sizeof(backup));  // Save the unsolved state
        sudoku.Clean();
        if (sudoku.Solve() == 0) {
            bool complete = true;
            for (int i = 0; i < 9 && complete; i++) {
                for (int j = 0; j < 9; j++) {
                    if (sudoku.GetValue(i, j) == -1) {
                        complete = false;
                        break;
                    }
                }
            }
            
            if (complete && countClues() >= settings.minClues && countClues() <= settings.maxClues) {
                // Restore to unsolved state and return
                memcpy(sudoku.board, backup, sizeof(backup));
                return true;
            }
        }
    }

    return false;
}

// Helper methods remain the same
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
