#ifndef SUDOKU_DISPLAY_H
#define SUDOKU_DISPLAY_H

#ifdef _WIN32
#include <windows.h>
#else
#include <ncurses.h>
#define _NCURSES
#endif

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

class Sudoku {
public:
    // Constructor and Destructor
    Sudoku();
    ~Sudoku();

    // Core Game Functions
    int SetValue(int x, int y, int value);
    int GetValue(int x, int y);
    int ClearValue(int x, int y);
    void NewGame();
    bool LoadFromFile(const std::string& filename);
    void SaveToFile(const std::string& filename);
    
    // Main Solving Functions
    int Solve();
    int SolveBasic();
    bool LegalValue(int x, int y, int value);
    
    // Debug and Logging
    void LogBoard(std::ofstream& file, const char* algorithm_name);
    void print_debug(const char* format, ...);

   // Basic Solving Techniques
    int StdElim();           // Standard elimination
    int LinElim();           // Line-based elimination
    int FindHiddenSingles(); // Hidden singles technique

    // Advanced Solving Techniques
    int FindHiddenPairs();      // Hidden pairs technique
    int FindPointingPairs();    // Pointing pairs technique
    int FindNakedSets();        // Naked sets (pairs/triples/quads)
    void FindNakedSetInUnit(std::vector<std::pair<int, int>>& cells, 
                           const std::vector<int>& candidates, 
                           int& changed);

    // Expert Solving Techniques
    int FindXWing();           // X-Wing pattern
    int FindSwordFish();       // Swordfish pattern
    int FindXYWing();          // XY-Wing pattern
    int FindXYZWing();         // XYZ-Wing pattern
    int FindSimpleColoring();  // Simple coloring technique
    int Clean();



private:
    // Internal Board State
    int board[9][9][9];
    static int debug_line;

    // Board Manipulation Functions
    int EliminatePossibility(int x, int y, int value);
    void RestoreBoard(int original_board[9][9][9], int board[9][9][9]);

    // Validation Functions
    bool IsValidUnit(std::vector<int>& values);
    bool IsValidSolution();

    // Candidate Management
    std::vector<int> GetCellCandidates(int x, int y);
    bool VectorsEqual(const std::vector<int>& v1, const std::vector<int>& v2);

 
};

#endif // SUDOKU_H
