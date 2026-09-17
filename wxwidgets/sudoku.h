#ifndef SUDOKU_DISPLAY_H
#define SUDOKU_DISPLAY_H

// Portable, thread-safe debug message queue (used by both Windows and Linux
// builds). The solver runs on a background thread; the UI polls this queue
// on a timer to drain messages without blocking the solver.
#include <mutex>
#include <deque>
#include <string>

struct DebugQueue {
    std::mutex mtx;
    std::deque<std::string> messages;
};

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
using std::string;

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
    bool IsValidSolution();
    int board[9][9][9];

    void ExportToExcelXML(const string& filename);

    DebugQueue debugQueue;
    // Returns the next queued debug message, or empty string if none pending.
    std::string get_next_debug_message();

private:
    static int debug_line;

    // Board Manipulation Functions
    int EliminatePossibility(int x, int y, int value);
    void RestoreBoard(int original_board[9][9][9], int board[9][9][9]);

    // Validation Functions
    bool IsValidUnit(std::vector<int>& values);

    // Candidate Management
    std::vector<int> GetCellCandidates(int x, int y);
    bool VectorsEqual(const std::vector<int>& v1, const std::vector<int>& v2);

 
};

#endif // SUDOKU_H
