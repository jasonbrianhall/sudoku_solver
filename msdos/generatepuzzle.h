#ifndef GENERATEPUZZLE_H
#define GENERATEPUZZLE_H

#include <random>
#include <map>
#include <vector>
#include <string>
#include <chrono>
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

    const std::map<std::string, DifficultySettings> difficultyLevels;

    bool isUnique(const std::vector<std::pair<int, int>>& removedCells);
    bool generateValidSolution();
    int countClues();
    bool requiresAdvancedTechnique(const std::string& technique);

public:
    PuzzleGenerator(Sudoku& s);
    bool generatePuzzle(const std::string& difficulty);
};

#endif // GENERATEPUZZLE_H
