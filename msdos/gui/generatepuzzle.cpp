#include "generatepuzzle.h"
#include <cstring>
#include <algorithm>
#include <chrono>

PuzzleGenerator::PuzzleGenerator(Sudoku& s) : sudoku(s), 
    difficultyLevels{
        {"easy",    {50, 55, false, false, false, false     }},
        {"medium",  {36, 49, false, false, false, false     }},
        {"hard",    {32, 35, true,  false, false, false     }},
        {"expert",  {28, 31, true,  true,  true,  false     }},
        {"extreme", {24, 27, true,  true,  true,  true      }},
        {"ultraextreme", {17, 19, true,  true,  true,  true }}

    } {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}

bool PuzzleGenerator::generateValidSolution() {
    while (true) {  // Keep going until we succeed
        sudoku.print_debug("Attempting to create a valid board...\n");
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
                sudoku.print_debug("Attempt failed, starting over...\n");               
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
                    sudoku.print_debug("Success, created a unique solution...\n");
                    return true;  // Found a complete valid solution!
                }
            }
        }
        sudoku.print_debug("Attempt failed after 50 placments, starting over...\n");
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

    // Generate one valid solution - will always succeed
    generateValidSolution();

    // Store the complete solution
    int solution[9][9][9];
    memcpy(solution, sudoku.board, sizeof(solution));

    // Calculate how many numbers to remove based on difficulty
    int numbersToRemove;
    if (difficulty == "easy") {
        numbersToRemove = 31;
    } else if (difficulty == "medium") {
        numbersToRemove = 45;
    } else if (difficulty == "hard") {
        numbersToRemove = 49;
    } else if (difficulty == "expert") {
        numbersToRemove = 53;
    } else if (difficulty == "extreme") {
        numbersToRemove = 57;
    } else if (difficulty == "ultraextreme") {
        numbersToRemove = 64;
    } else { // extreme
        numbersToRemove = 57;
    }

    while (true) {
        // Restore the complete solution
        memcpy(sudoku.board, solution, sizeof(solution));

        // Create list of all positions
        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                positions.emplace_back(i, j);
            }
        }
        std::shuffle(positions.begin(), positions.end(), rng);

        // Remove the specified number of random positions
        for (int i = 0; i < numbersToRemove; i++) {
            int row = positions[i].first;
            int col = positions[i].second;
            sudoku.ClearValue(row, col);
        }

        // Try to solve the puzzle
        if (sudoku.Solve() == 0) {
            // Verify all cells are filled
            bool allFilled = true;
            for (int i = 0; i < 9 && allFilled; i++) {
                for (int j = 0; j < 9; j++) {
                    if (sudoku.GetValue(i, j) == -1) {
                        allFilled = false;
                        break;
                    }
                }
            }

            if (allFilled) {
                // Success - restore to unsolved state
                memcpy(sudoku.board, solution, sizeof(solution));
                for (int i = 0; i < numbersToRemove; i++) {
                    int row = positions[i].first;
                    int col = positions[i].second;
                    sudoku.ClearValue(row, col);
                }
                return true;
            }
        }
        // If we get here, try another removal pattern
    }
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
