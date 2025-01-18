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
    sudoku.NewGame();
    
    // Fill in diagonal boxes first (these can be filled independently)
    for (int box = 0; box < 9; box += 4) {
        std::vector<int> values{0,1,2,3,4,5,6,7,8};
        std::shuffle(values.begin(), values.end(), rng);
        
        int boxRow = (box / 3) * 3;
        int boxCol = (box % 3) * 3;
        int idx = 0;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sudoku.SetValue(boxRow + i, boxCol + j, values[idx++]);
            }
        }
    }
    
    // Solve the rest of the grid to get a complete valid solution
    return sudoku.Solve() == 0;
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

    // Try to generate a valid puzzle
    const int maxAttempts = 50;
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        // Generate a complete valid solution
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

        // Target number of clues for this difficulty
        int targetClues = settings.minClues + 
            (rng() % (settings.maxClues - settings.minClues + 1));

        int currentState[9][9][9];
        bool validPuzzleFound = false;

        while (countClues() > targetClues && !positions.empty()) {
            // Try removing a number
            auto pos = positions.back();
            positions.pop_back();

            int backup[9][9][9];
            memcpy(backup, sudoku.board, sizeof(backup));
            
            sudoku.ClearValue(pos.first, pos.second);

            // Save the current unsolved state
            memcpy(currentState, sudoku.board, sizeof(currentState));

            // Run solver
            sudoku.Clean();
            if (sudoku.Solve() == 0) {
                // Verify all positions are filled
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
                    // Restore to unsolved state and continue
                    memcpy(sudoku.board, currentState, sizeof(currentState));
                } else {
                    // Not completely solvable - restore and skip
                    memcpy(sudoku.board, backup, sizeof(backup));
                    continue;
                }
            } else {
                // Not solvable - restore and skip
                memcpy(sudoku.board, backup, sizeof(backup));
                continue;
            }

            // For harder difficulties, verify required techniques
            bool valid = true;
            if (settings.allowXWing) {
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
                // Doesn't meet difficulty requirements - restore
                memcpy(sudoku.board, backup, sizeof(backup));
            }
        }

        // Final verification
        memcpy(currentState, sudoku.board, sizeof(currentState));
        sudoku.Clean();
        if (sudoku.Solve() == 0) {
            // Verify all positions are filled
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
                // Valid puzzle found - restore to unsolved state
                memcpy(sudoku.board, currentState, sizeof(currentState));
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
