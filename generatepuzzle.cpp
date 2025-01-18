#include <random>
#include <chrono>
#include <algorithm>
#include <set>
#include <cstring>
#include <map>
#include "sudoku.h"

class PuzzleGenerator {
private:
    Sudoku& sudoku;
    std::mt19937 rng;

    // Difficulty settings for each level
    struct DifficultySettings {
        int minClues;      // Minimum number of starting clues
        int maxClues;      // Maximum number of starting clues
        bool allowXWing;   // Whether X-Wing technique is needed
        bool allowSwordfish; // Whether Swordfish technique is needed
        bool allowXYWing;   // Whether XY-Wing technique is needed
        bool allowXYZWing;  // Whether XYZ-Wing technique is needed
    };

    const std::map<std::string, DifficultySettings> difficultyLevels = {
        {"easy",    {50, 55, false, false, false, false}},
        {"medium",  {36, 49, false, false, false, false}},
        {"hard",    {32, 35, true,  false, false, false}},
        {"expert",  {28, 31, true,  true,  true,  false}},
        {"extreme", {24, 27, true,  true,  true,  true}}
    };

    bool isUnique(const std::vector<std::pair<int, int>>& removedCells) {
        // Create a copy of the current board
        int backup[9][9][9];
        memcpy(backup, sudoku.board, sizeof(backup));

        // Try to solve the puzzle
        if (sudoku.Solve() != 0) {
            // Restore the board
            memcpy(sudoku.board, backup, sizeof(backup));
            return false;
        }

        // Store the solution
        int solution[9][9];
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                solution[i][j] = sudoku.GetValue(i, j);
            }
        }

        // Restore and try to find a different solution
        memcpy(sudoku.board, backup, sizeof(backup));
        
        // Modify one candidate in an empty cell
        bool foundDifferentSolution = false;
        for (const auto& cell : removedCells) {
            int row = cell.first;
            int col = cell.second;
            
            // Try each possible value except the known solution
            for (int val = 0; val < 9; val++) {
                if (val != solution[row][col]) {
                    // Try this alternative value
                    sudoku.SetValue(row, col, val);
                    if (sudoku.Solve() == 0) {
                        // Found a different solution
                        foundDifferentSolution = true;
                        break;
                    }
                    // Restore the cell for next attempt
                    memcpy(sudoku.board, backup, sizeof(backup));
                }
            }
            if (foundDifferentSolution) break;
        }

        // Restore the original board
        memcpy(sudoku.board, backup, sizeof(backup));
        return !foundDifferentSolution;
    }

    bool generateValidSolution() {
        // Start with an empty board
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
        
        // Solve the rest of the grid
        return sudoku.Solve() == 0;
    }

    int countClues() {
        int count = 0;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (sudoku.GetValue(i, j) != -1) count++;
            }
        }
        return count;
    }

    bool requiresAdvancedTechnique(const std::string& technique) {
        // Create a copy of the current board
        int backup[9][9][9];
        memcpy(backup, sudoku.board, sizeof(backup));

        // Try to solve without using the specified technique
        bool needsTechnique = false;
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
            
            // Only check other techniques if they're not the one we're testing for
            if (technique != "x-wing" && result == 0)
                result = sudoku.FindXWing();
            if (technique != "swordfish" && result == 0)
                result = sudoku.FindSwordFish();
            if (technique != "xy-wing" && result == 0)
                result = sudoku.FindXYWing();
            if (technique != "xyz-wing" && result == 0)
                result = sudoku.FindXYZWing();
                
        } while (result > 0);

        // If puzzle is not solved, it needs the technique
        needsTechnique = !sudoku.IsValidSolution();

        // Restore the board
        memcpy(sudoku.board, backup, sizeof(backup));
        return needsTechnique;
    }

public:
    PuzzleGenerator(Sudoku& s) : sudoku(s) {
        // Initialize random number generator with time-based seed
        rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    bool generatePuzzle(const std::string& difficulty) {
        // Check if difficulty level exists
        auto diffIt = difficultyLevels.find(difficulty);
        if (diffIt == difficultyLevels.end()) {
            return false;
        }
        const DifficultySettings& settings = diffIt->second;

        // Generate a valid complete solution
        if (!generateValidSolution()) {
            return false;
        }

        // Store the complete solution
        int solution[9][9][9];
        memcpy(solution, sudoku.board, sizeof(solution));

        // Create list of all positions
        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                positions.emplace_back(i, j);
            }
        }

        // Randomly remove numbers while maintaining uniqueness
        std::vector<std::pair<int, int>> removedCells;
        int targetClues = settings.minClues + 
            (rng() % (settings.maxClues - settings.minClues + 1));

        while (countClues() > targetClues && !positions.empty()) {
            // Randomly select a position to try removing
            int idx = rng() % positions.size();
            int row = positions[idx].first;
            int col = positions[idx].second;
            
            // Remove the last position from our list
            std::swap(positions[idx], positions.back());
            positions.pop_back();

            // Try removing the number
            int backup[9][9][9];
            memcpy(backup, sudoku.board, sizeof(backup));
            
            int removedValue = sudoku.GetValue(row, col);
            sudoku.ClearValue(row, col);
            removedCells.emplace_back(row, col);

            // Check if puzzle is still valid and meets difficulty requirements
            bool valid = isUnique(removedCells);
            
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
                // Restore the number if removing it made the puzzle invalid
                memcpy(sudoku.board, backup, sizeof(backup));
                removedCells.pop_back();
            }
        }

        return true;
    }
};
