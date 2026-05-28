// Microsoft Windows Native Version

#define _HAS_STD_BYTE 0
#include <msclr/marshal_cppstd.h>
#include <fstream>
#include <windows.h>

#include "resource.h"
#include "sudoku.h"
#include "generatepuzzle.h"
#include "highscores.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace SudokuGame {

// Enum for stateful algorithm selection (matches execution order in Solve())
public enum class AlgorithmType {
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

public
ref class SudokuWrapper {
 private:
  Sudoku* nativeSudoku;
  AlgorithmType currentAlgorithm;
  array<array<bool>^>^ immutableCells;      // Original puzzle clues (salmon text)
  array<array<bool>^>^ quasiImmutableCells; // User-earned locks (green)
  
  // Undo history system
  ref class BoardState {
   public:
    array<int, 3>^ boardData;
    
    BoardState(int source[9][9][9]) {
      boardData = gcnew array<int, 3>(9, 9, 9);
      for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
          for (int z = 0; z < 9; z++) {
            boardData[x, y, z] = source[x][y][z];
          }
        }
      }
    }
    
    void CopyToBoard(int dest[9][9][9]) {
      for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
          for (int z = 0; z < 9; z++) {
            dest[x][y][z] = boardData[x, y, z];
          }
        }
      }
    }
  };
  
  System::Collections::Generic::Stack<BoardState^>^ undoStack;
  static const int MAX_UNDO_STATES = 100;

 public:
  int savedElapsedSeconds;

 public:
  SudokuWrapper() { 
    nativeSudoku = new Sudoku(); 
    currentAlgorithm = AlgorithmType::ALG_NONE;
    savedElapsedSeconds = 0;
    
    // Initialize immutable cells array
    immutableCells = gcnew array<array<bool>^>(9);
    quasiImmutableCells = gcnew array<array<bool>^>(9);
    for (int i = 0; i < 9; i++) {
      immutableCells[i] = gcnew array<bool>(9);
      quasiImmutableCells[i] = gcnew array<bool>(9);
      for (int j = 0; j < 9; j++) {
        immutableCells[i][j] = false;
        quasiImmutableCells[i][j] = false;
      }
    }
    
    // Initialize undo stack
    undoStack = gcnew System::Collections::Generic::Stack<BoardState^>();
  }

  ~SudokuWrapper() {
    if (nativeSudoku) {
      delete nativeSudoku;
      nativeSudoku = nullptr;
    }
  }
  
  // Save current board state to undo history
  void SaveBoardState() {
    if (undoStack->Count < MAX_UNDO_STATES) {
      BoardState^ state = gcnew BoardState(nativeSudoku->board);
      undoStack->Push(state);
    }
  }
  
  // Undo last change
  bool Undo() {
    if (undoStack->Count > 0) {
      BoardState^ state = undoStack->Pop();
      state->CopyToBoard(nativeSudoku->board);
      return true;
    }
    return false;
  }
  
  // Check if undo is available
  bool CanUndo() {
    return undoStack->Count > 0;
  }
  
  // Get undo history count
  int GetUndoCount() {
    return undoStack->Count;
  }
  
  // Clear undo history
  void ClearUndoHistory() {
    undoStack->Clear();
  }
  
  // Clear board except immutable cells
  void ClearBoardExceptImmutable() {
    for (int x = 0; x < 9; x++) {
      for (int y = 0; y < 9; y++) {
        if (!immutableCells[x][y]) {
          nativeSudoku->ClearValue(x, y);
        }
      }
    }
  }
  property Sudoku* NativeSudoku {
    Sudoku* get() { return nativeSudoku; }
  }
  
  property AlgorithmType CurrentAlgorithm {
    AlgorithmType get() { return currentAlgorithm; }
    void set(AlgorithmType value) { currentAlgorithm = value; }
  }
  
  // Mark all currently filled cells as immutable (from puzzle generation)
  void MarkPuzzleAsGenerated() {
    for (int row = 0; row < 9; row++) {
      for (int col = 0; col < 9; col++) {
        if (nativeSudoku->GetValue(col, row) != -1) {
          immutableCells[col][row] = true;
        } else {
          immutableCells[col][row] = false;
        }
        quasiImmutableCells[col][row] = false;
      }
    }
  }
  
  // Check if a cell is immutable (from puzzle generation)
  bool IsCellImmutable(int col, int row) {
    if (col >= 0 && col < 9 && row >= 0 && row < 9) {
      return immutableCells[col][row];
    }
    return false;
  }

  // Check if a cell is quasi-immutable (user-earned green lock)
  bool IsCellQuasiImmutable(int col, int row) {
    if (col >= 0 && col < 9 && row >= 0 && row < 9) {
      return quasiImmutableCells[col][row];
    }
    return false;
  }

  // Lock a cell as quasi-immutable
  void SetQuasiImmutable(int col, int row) {
    if (col >= 0 && col < 9 && row >= 0 && row < 9) {
      quasiImmutableCells[col][row] = true;
    }
  }

  // Check if a cell is locked (either type)
  bool IsCellLocked(int col, int row) {
    return IsCellImmutable(col, row) || IsCellQuasiImmutable(col, row);
  }

  // Reset immutability when starting new game
  void ClearImmutability() {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        immutableCells[i][j] = false;
        quasiImmutableCells[i][j] = false;
      }
    }
  }
  
  // Core game functions
  void SetValue(int x, int y, int value) {
    nativeSudoku->SetValue(x, y, value);
  }

  int GetValue(int x, int y) { return nativeSudoku->GetValue(x, y); }
  void NewGame() { 
    nativeSudoku->NewGame(); 
    currentAlgorithm = AlgorithmType::ALG_NONE;
    ClearImmutability();
  }
  void Solve() { nativeSudoku->Solve(); }
  void ClearValue(int x, int y) { nativeSudoku->ClearValue(x, y); }
  void Clean() { nativeSudoku->Clean(); }
  bool IsValidSolution() { return nativeSudoku->IsValidSolution(); }

  // File operations
  bool LoadFromFile(String ^ filename) {
    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
    bool result = nativeSudoku->LoadFromFile(std::string(wstr.begin(), wstr.end()));
    if (result) {
      ClearImmutability();
      String^ sidecar = filename + ".imm";
      std::wstring sidecarW = msclr::interop::marshal_as<std::wstring>(sidecar);
      std::ifstream fin(std::string(sidecarW.begin(), sidecarW.end()));
      if (fin.is_open()) {
        for (int col = 0; col < 9; col++) {
          for (int row = 0; row < 9; row++) {
            int v = 0;
            fin >> v;
            immutableCells[col][row] = (v == 1);
          }
        }
        // Load saved elapsed time
        int savedTime = 0;
        fin >> savedTime;
        savedElapsedSeconds = savedTime;
        fin.close();
      }
    }
    return result;
  }

  void SaveToFile(String ^ filename, int elapsedSecs) {
    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
    nativeSudoku->SaveToFile(std::string(wstr.begin(), wstr.end()));
    String^ sidecar = filename + ".imm";
    std::wstring sidecarW = msclr::interop::marshal_as<std::wstring>(sidecar);
    std::ofstream fout(std::string(sidecarW.begin(), sidecarW.end()));
    if (fout.is_open()) {
      for (int col = 0; col < 9; col++) {
        for (int row = 0; row < 9; row++) {
          fout << (immutableCells[col][row] ? 1 : 0);
          if (row < 8) fout << " ";
        }
        fout << "\n";
      }
      // Save elapsed time on final line
      fout << elapsedSecs << "\n";
      fout.close();
    }
  }

  void ExportToExcelXML(String ^ filename) {
    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
    nativeSudoku->ExportToExcelXML(std::string(wstr.begin(), wstr.end()));
  }


  // Basic solving techniques
  void StdElim() { nativeSudoku->StdElim(); }
  void LinElim() { nativeSudoku->LinElim(); }
  void FindHiddenSingles() { nativeSudoku->FindHiddenSingles(); }

  // Advanced solving techniques
  void FindHiddenPairs() { nativeSudoku->FindHiddenPairs(); }
  void FindPointingPairs() { nativeSudoku->FindPointingPairs(); }
  void FindNakedSets() { nativeSudoku->FindNakedSets(); }
  void FindXWing() { nativeSudoku->FindXWing(); }
  void FindSwordFish() { nativeSudoku->FindSwordFish(); }
  void FindXYWing() { nativeSudoku->FindXYWing(); }
  void FindXYZWing() { nativeSudoku->FindXYZWing(); }
  void FindSimpleColoring() { nativeSudoku->FindSimpleColoring(); }
  
  // Stateful solving - runs algorithms in sequence up to selected one
  void SolveToAlgorithm(AlgorithmType targetAlgorithm) {
    currentAlgorithm = targetAlgorithm;
    
    // Execute algorithms in order up to target
    bool changes_made;
    do {
      changes_made = false;
      
      // PHASE 1: Basic eliminations
      if (targetAlgorithm >= AlgorithmType::ALG_STD_ELIM) {
        bool basic_changes;
        do {
          basic_changes = false;
          
          // StdElim
          if (targetAlgorithm >= AlgorithmType::ALG_STD_ELIM) {
            if (nativeSudoku->StdElim() > 0) {
              basic_changes = true;
              changes_made = true;
            }
            if (targetAlgorithm == AlgorithmType::ALG_STD_ELIM) {
              return;
            }
          }
          
          // LinElim
          if (targetAlgorithm >= AlgorithmType::ALG_LINE_ELIM) {
            if (nativeSudoku->LinElim() > 0) {
              basic_changes = true;
              changes_made = true;
            }
            if (targetAlgorithm == AlgorithmType::ALG_LINE_ELIM) {
              return;
            }
          }
          
        } while (basic_changes);
      }
      
      // PHASE 2: Advanced techniques (one at a time, in order)
      if (!changes_made) {
        
        // Hidden Singles
        if (targetAlgorithm >= AlgorithmType::ALG_HIDDEN_SINGLES) {
          if (nativeSudoku->FindHiddenSingles() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_SINGLES) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_SINGLES) {
            return;
          }
        }
        
        // Hidden Pairs
        if (targetAlgorithm >= AlgorithmType::ALG_HIDDEN_PAIRS && !changes_made) {
          if (nativeSudoku->FindHiddenPairs() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_PAIRS) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_HIDDEN_PAIRS) {
            return;
          }
        }
        
        // Pointing Pairs
        if (targetAlgorithm >= AlgorithmType::ALG_POINTING_PAIRS && !changes_made) {
          if (nativeSudoku->FindPointingPairs() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_POINTING_PAIRS) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_POINTING_PAIRS) {
            return;
          }
        }
        
        // X-Wing
        if (targetAlgorithm >= AlgorithmType::ALG_X_WING && !changes_made) {
          if (nativeSudoku->FindXWing() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_X_WING) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_X_WING) {
            return;
          }
        }
        
        // Swordfish
        if (targetAlgorithm >= AlgorithmType::ALG_SWORDFISH && !changes_made) {
          if (nativeSudoku->FindSwordFish() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_SWORDFISH) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_SWORDFISH) {
            return;
          }
        }
        
        // Naked Sets
        if (targetAlgorithm >= AlgorithmType::ALG_NAKED_SETS && !changes_made) {
          if (nativeSudoku->FindNakedSets() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_NAKED_SETS) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_NAKED_SETS) {
            return;
          }
        }
        
        // XY-Wing
        if (targetAlgorithm >= AlgorithmType::ALG_XY_WING && !changes_made) {
          if (nativeSudoku->FindXYWing() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_XY_WING) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_XY_WING) {
            return;
          }
        }
        
        // XYZ-Wing
        if (targetAlgorithm >= AlgorithmType::ALG_XYZ_WING && !changes_made) {
          if (nativeSudoku->FindXYZWing() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_XYZ_WING) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_XYZ_WING) {
            return;
          }
        }
        
        // Simple Coloring
        if (targetAlgorithm >= AlgorithmType::ALG_SIMPLE_COLORING && !changes_made) {
          if (nativeSudoku->FindSimpleColoring() > 0) {
            changes_made = true;
            if (targetAlgorithm == AlgorithmType::ALG_SIMPLE_COLORING) {
              return;
            }
            continue;
          }
          if (targetAlgorithm == AlgorithmType::ALG_SIMPLE_COLORING) {
            return;
          }
        }
      }
      
    } while (changes_made);
  }
};  // End of SudokuWrapper class

std::string unix2dos(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2); // Reserving space to avoid multiple allocations

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\n' && (i == 0 || input[i - 1] != '\r')) {
            output.push_back('\r');
        }
        output.push_back(input[i]);
    }

    return output;
}

public ref class MainForm : public System::Windows::Forms::Form {
 private:
  SudokuWrapper ^ sudoku;
  array<TextBox ^, 2> ^ grid;
  array<TextBox ^, 2> ^ notes;  // 2D array for cell notes - changed to TextBox
  bool notesVisible;  // Track whether notes are currently visible
  MenuStrip ^ menuStrip;
  ToolStrip ^ toolStrip;
  StatusStrip ^ statusStrip;
  ToolStripStatusLabel ^ statusLabel;
  TextBox^ instructionsBox;
  TextBox^ debugBox;
  Panel^ gridContainer;
  ToolStripButton^ undoBtn;
  ToolStripButton^ clearBoardBtn;
  ToolStripLabel^ undoCountLabel;
  ToolStripStatusLabel^ timerLabel;
  Timer^ gameTimer;
  int elapsedSeconds;
  bool isSolving;  // prevents re-entrant solver calls
  // Sliding window of last 5 correct cell positions for quasi-immutable locking
  System::Collections::Generic::Queue<array<int>^>^ correctQueue;
  Highscores* highscores;
  String^ currentDifficulty;  // "easy","medium","hard","master","expert" or "" if none
  bool puzzleSolved;          // prevent multiple win triggers
  bool timerPaused;           // pause timer on window deactivate
  int highlightValue;         // currently highlighted digit (0-8, or -1 for none)
  bool colorblindMode;        // use font style instead of color for cell types
  float gridFontSize;         // current font size for grid numbers

  void GeneratePuzzle1(Object ^ sender, EventArgs ^ e) {
    sudoku->ExportToExcelXML("puzzle1.xml");
    UpdateStatus("Saved puzzle as Excel/puzzle1.xml");
  }

  void GeneratePuzzle2(Object ^ sender, EventArgs ^ e) {
    sudoku->ExportToExcelXML("puzzle2.xml");
    UpdateStatus("Saved puzzle as Excel/puzzle2.xml");
  }

  void GeneratePuzzle3(Object ^ sender, EventArgs ^ e) {
    sudoku->ExportToExcelXML("puzzle3.xml");
    UpdateStatus("Saved puzzle as Excel/puzzle3.xml");
  }

  void GeneratePuzzle4(Object ^ sender, EventArgs ^ e) {
    sudoku->ExportToExcelXML("puzzle4.xml");
    UpdateStatus("Saved puzzle as Excel/puzzle4.xml");
  }

  bool ConfirmNewGame() {
    if (puzzleSolved || currentDifficulty == "") return true;
    // Check if any user cells are filled
    bool hasProgress = false;
    for (int i = 0; i < 9 && !hasProgress; i++)
      for (int j = 0; j < 9 && !hasProgress; j++)
        if (!sudoku->IsCellImmutable(i, j) && sudoku->GetValue(i, j) != -1)
          hasProgress = true;
    if (!hasProgress) return true;
    return MessageBox::Show(this,
      "You have a game in progress. Start a new game?",
      "New Game",
      MessageBoxButtons::YesNo,
      MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes;
  }

  void GenerateEasy_Click(Object ^ sender, EventArgs ^ e) {
    if (!ConfirmNewGame()) return;
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("easy")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();
      currentDifficulty = "easy";
      puzzleSolved = false;
      correctQueue->Clear();
      ResetTimer();
      UpdateGrid();
      PlayNewGameSound();
      UpdateStatus("Generated new easy puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate easy puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateMedium_Click(Object ^ sender, EventArgs ^ e) {
    if (!ConfirmNewGame()) return;
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("medium")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();
      currentDifficulty = "medium";
      puzzleSolved = false;
      correctQueue->Clear();
      ResetTimer();
      UpdateGrid();
      PlayNewGameSound();
      UpdateStatus("Generated new medium puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate medium puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateHard_Click(Object ^ sender, EventArgs ^ e) {
    if (!ConfirmNewGame()) return;
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("hard")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();
      currentDifficulty = "hard";
      puzzleSolved = false;
      correctQueue->Clear();
      ResetTimer();
      UpdateGrid();
      PlayNewGameSound();
      UpdateStatus("Generated new hard puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate hard puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateExpert_Click(Object ^ sender, EventArgs ^ e) {
    if (!ConfirmNewGame()) return;
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("expert")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();
      currentDifficulty = "expert";
      puzzleSolved = false;
      correctQueue->Clear();
      ResetTimer();
      UpdateGrid();
      PlayNewGameSound();
      UpdateStatus("Generated new expert puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate expert puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateMaster_Click(Object ^ sender, EventArgs ^ e) {
    if (!ConfirmNewGame()) return;
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("extreme")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();
      currentDifficulty = "master";
      puzzleSolved = false;
      correctQueue->Clear();
      ResetTimer();
      UpdateGrid();
      PlayNewGameSound();
      UpdateStatus("Generated new extreme puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate extreme puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void InitializeComponent() {
    this->Size = System::Drawing::Size(800, 600);
    this->Text = L"Sudoku Game";
    this->StartPosition = FormStartPosition::CenterScreen;

    // Initialize StatusStrip
    statusStrip = gcnew StatusStrip();
    statusLabel = gcnew ToolStripStatusLabel("Ready");
    statusLabel->Spring = true;
    statusLabel->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
    statusStrip->Items->Add(statusLabel);
    timerLabel = gcnew ToolStripStatusLabel("00:00");
    timerLabel->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
    statusStrip->Items->Add(timerLabel);
    this->Controls->Add(statusStrip);

    // Initialize MenuStrip
    menuStrip = gcnew MenuStrip();
    ToolStripMenuItem^ fileMenu = gcnew ToolStripMenuItem("Game");

    // New Game submenu
    ToolStripMenuItem^ generateBoardMenu = gcnew ToolStripMenuItem("New Game");
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Easy F(1)", nullptr,
        gcnew EventHandler(this, &MainForm::GenerateEasy_Click)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Medium F(2)", nullptr,
        gcnew EventHandler(this, &MainForm::GenerateMedium_Click)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Hard F(3)", nullptr,
        gcnew EventHandler(this, &MainForm::GenerateHard_Click)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Master F(4)", nullptr,
        gcnew EventHandler(this, &MainForm::GenerateMaster_Click)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Expert Shift F(1)", nullptr,
        gcnew EventHandler(this, &MainForm::GenerateExpert_Click)));
    fileMenu->DropDownItems->Add(generateBoardMenu);

    // Save Slots
    ToolStripMenuItem^ saveMenu = gcnew ToolStripMenuItem("Save Game");
    int k;
    for (int i = 1; i <= 4; i++) {
        k=i+4;
        ToolStripMenuItem^ saveSlot = gcnew ToolStripMenuItem("Slot " + i + " F(" + k + ")");
        saveSlot->Tag = i;
        saveSlot->Click += gcnew EventHandler(this, &MainForm::SaveSlot_Click);
        saveMenu->DropDownItems->Add(saveSlot);
    }

    // Load Slots
    ToolStripMenuItem^ loadMenu = gcnew ToolStripMenuItem("Load Game");
    for (int i = 1; i <= 4; i++) {
        k=i+4;
        ToolStripMenuItem^ loadSlot = gcnew ToolStripMenuItem("Slot " + i + " Shift F(" + k + ")");
        loadSlot->Tag = i;
        loadSlot->Click += gcnew EventHandler(this, &MainForm::LoadSlot_Click);
        loadMenu->DropDownItems->Add(loadSlot);
    }

    fileMenu->DropDownItems->Add(saveMenu);
    fileMenu->DropDownItems->Add(loadMenu);
    fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "High Scores", nullptr, gcnew EventHandler(this, &MainForm::ViewHighscores_Click)));
    fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Quit", nullptr, gcnew EventHandler(this, &MainForm::Exit_Click)));

    // Help Menu
    ToolStripMenuItem^ helpMenu = gcnew ToolStripMenuItem("Help");

    // Font size submenu
    ToolStripMenuItem^ fontSizeMenu = gcnew ToolStripMenuItem("Font Size");
    array<String^>^ fontSizes = {"Small (14)", "Medium (20)", "Large (26)", "Extra Large (32)"};
    array<float>^ fontSizeVals = {14.0f, 20.0f, 26.0f, 32.0f};
    for (int i = 0; i < fontSizes->Length; i++) {
      ToolStripMenuItem^ item = gcnew ToolStripMenuItem(fontSizes[i]);
      item->Tag = fontSizeVals[i];
      item->Click += gcnew EventHandler(this, &MainForm::FontSize_Click);
      fontSizeMenu->DropDownItems->Add(item);
    }
    helpMenu->DropDownItems->Add(fontSizeMenu);

    helpMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Colorblind Mode", nullptr,
        gcnew EventHandler(this, &MainForm::ColorblindMode_Click)));
    helpMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "About", nullptr,
        gcnew EventHandler(this, &MainForm::About_Click)));
    helpMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Support the Author (Buy Me a Coffee)", nullptr,
        gcnew EventHandler(this, &MainForm::SupportAuthor_Click)));

    // Add Menus to MenuStrip
    menuStrip->Items->Add(fileMenu);
    menuStrip->Items->Add(helpMenu);

    // Attach MenuStrip to the Form
    this->MainMenuStrip = menuStrip;
    this->Controls->Add(menuStrip);

    // Initialize ToolStrip
    toolStrip = gcnew ToolStrip();
    
    // Add Toggle Notes button
    ToolStripButton^ toggleNotesBtn = gcnew ToolStripButton(
        "Toggle Notes (T)", nullptr,
        gcnew EventHandler(this, &MainForm::ToggleNotes_Click));
    toggleNotesBtn->AutoSize = false;
    toggleNotesBtn->Size = System::Drawing::Size(110, 25);
    toolStrip->Items->Add(toggleNotesBtn);
    
    toolStrip->Items->Add(gcnew ToolStripSeparator());

    toolStrip->Items->Add(gcnew ToolStripButton(
        "Copy Board", nullptr,
        gcnew EventHandler(this, &MainForm::CopyBoard_Click)));
    toolStrip->Items->Add(gcnew ToolStripSeparator());


    instructionsBox = gcnew TextBox();
    instructionsBox->Multiline = true;
    instructionsBox->ReadOnly = true;
    instructionsBox->BackColor = System::Drawing::Color::LightBlue; // Soothing background
    instructionsBox->BorderStyle = BorderStyle::FixedSingle; // Clean border
    instructionsBox->Location = Point(50, toolStrip->Height + menuStrip->Height + 5);
    instructionsBox->Size = System::Drawing::Size(700, 100);

    instructionsBox->Text = L"Instructions:\r\n\r\n"
        L"  - Use keypad (1-9) to enter numbers in cells. Middle mouse click or press 0 to clear.\r\n"
        L"  - Press 'T' or click 'Toggle Notes' to show/hide the notes areas under cells.\r\n"
        L"  - Click in a notes area to add candidate numbers (e.g., '2 5 8').\r\n"
        L"  - Press F1-F4 for new puzzles (easy to master). Press Shift+F1 for expert.\r\n"
        L"  - Press F5-F8 to save, Shift+F5-F8 to load. Arrow keys navigate cells.";

    instructionsBox->Font = gcnew System::Drawing::Font(L"Lucida Console", 9); // Consistent fixed-width font
    this->Controls->Add(instructionsBox);


    this->Controls->Add(toolStrip);

    // Initialize notes visibility state (hidden by default)
    notesVisible = false;

    // Initialize grid
    grid = gcnew array<TextBox ^, 2>(9, 9);
    notes = gcnew array<TextBox ^, 2>(9, 9);  // Initialize notes array as TextBox
    int gridTop = menuStrip->Height + toolStrip->Height + instructionsBox->Height + 25;

    // Cell dimensions: 60x70 pixels (45 for main cell, 25 for notes below)
    int cellWidth = 60;
    int cellHeight = 70;

    // Create a container panel for the Sudoku grid with white background
    gridContainer = gcnew Panel();
    gridContainer->Location = Point(50, gridTop);
    gridContainer->Size = System::Drawing::Size(540, 630);  // 9 * 60 = 540 wide, 9 * 70 = 630 tall
    gridContainer->BackColor = Color::White;
    this->Controls->Add(gridContainer);

    // Draw bold grid lines for 3x3 boxes FIRST (underneath cells)
    for (int i = 0; i <= 9; i++) {
      int thickness = (i % 3 == 0) ? 3 : 1;
      int offset = (i % 3 == 0) ? 1 : 0;
      
      Panel ^ vline = gcnew Panel();
      vline->BorderStyle = BorderStyle::None;
      vline->Location = Point(i * cellWidth - offset, 0);
      vline->Size = System::Drawing::Size(thickness, 630);
      vline->BackColor = Color::Black;
      gridContainer->Controls->Add(vline);

      Panel ^ hline = gcnew Panel();
      hline->BorderStyle = BorderStyle::None;
      hline->Location = Point(0, i * cellHeight - offset);
      hline->Size = System::Drawing::Size(540, thickness);
      hline->BackColor = Color::Black;
      gridContainer->Controls->Add(hline);
    }

    // Initialize grid cells with notes area below
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        // Create main cell TextBox (45 pixels tall)
        grid[i, j] = gcnew TextBox();
        grid[i, j]->Size = System::Drawing::Size(60, 45);
        grid[i, j]->Location = System::Drawing::Point(j * cellWidth, i * cellHeight);
        grid[i, j]->MaxLength = 1;
        grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", gridFontSize);
        grid[i, j]->TextAlign = HorizontalAlignment::Center;
        grid[i, j]->Tag = gcnew array<int>{i, j};
        grid[i, j]->TextChanged +=
            gcnew EventHandler(this, &MainForm::Cell_TextChanged);
        grid[i, j]->KeyDown +=
            gcnew KeyEventHandler(this, &MainForm::Cell_KeyDown);
        grid[i, j]->KeyPress +=
            gcnew KeyPressEventHandler(this, &MainForm::Cell_KeyPress);
        grid[i, j]->BackColor = Color::White;
        grid[i, j]->BorderStyle = BorderStyle::None;
        grid[i, j]->Cursor = Cursors::IBeam;
        grid[i, j]->GotFocus += gcnew EventHandler(this, &MainForm::Cell_GotFocus);
        grid[i, j]->LostFocus += gcnew EventHandler(this, &MainForm::Cell_LostFocus);
        gridContainer->Controls->Add(grid[i, j]);
        grid[i, j]->MouseWheel += gcnew MouseEventHandler(this, &MainForm::Cell_MouseWheel);
        grid[i, j]->MouseDown += gcnew MouseEventHandler(this, &MainForm::Cell_MouseDown);

        // Create notes TextBox below the cell (25 pixels tall) - now editable!
        notes[i, j] = gcnew TextBox();
        notes[i, j]->Size = System::Drawing::Size(60, 25);
        notes[i, j]->Location = System::Drawing::Point(j * cellWidth, i * cellHeight + 45);
        notes[i, j]->Font = gcnew System::Drawing::Font(L"Arial", 7);
        notes[i, j]->Multiline = true;
        notes[i, j]->WordWrap = true;
        notes[i, j]->BackColor = Color::WhiteSmoke;
        notes[i, j]->BorderStyle = BorderStyle::FixedSingle;
        notes[i, j]->Tag = gcnew array<int>{i, j};
        notes[i, j]->ScrollBars = ScrollBars::None;
        notes[i, j]->AcceptsTab = false;
        notes[i, j]->Visible = false;  // Hidden by default
        gridContainer->Controls->Add(notes[i, j]);
      }
    }

    Label^ debugLabel = gcnew Label();
    debugLabel->Text = "Debug Output";
    debugLabel->Location = Point(600, gridTop);
    debugLabel->AutoSize = true;
    this->Controls->Add(debugLabel);

    // Initialize debug box
    debugBox = gcnew TextBox();
    debugBox->Multiline = true;
    debugBox->ScrollBars = ScrollBars::Vertical;
    debugBox->ReadOnly = true;
    debugBox->Location = Point(600, gridTop+20);  // Position next to grid
    debugBox->Size = System::Drawing::Size(150, 600);  // Adjusted height to match new grid
    debugBox->Font = gcnew System::Drawing::Font(L"Consolas", 9);
    this->Controls->Add(debugBox);

    // Create a timer for updating debug messages
    Timer^ debugTimer = gcnew Timer();
    debugTimer->Interval = 100; // Check every 100ms
    debugTimer->Tick += gcnew EventHandler(this, &MainForm::UpdateDebugBox);
    debugTimer->Start();

    // Handle form resize to expand grid
    this->Resize += gcnew EventHandler(this, &MainForm::Form_Resize);
    // Pause timer when window loses focus
    this->Deactivate += gcnew EventHandler(this, &MainForm::Form_Deactivate);
    this->Activate += gcnew EventHandler(this, &MainForm::Form_Activate);

    // Initialize game timer
    elapsedSeconds = 0;
    gameTimer = gcnew Timer();
    gameTimer->Interval = 1000;
    gameTimer->Tick += gcnew EventHandler(this, &MainForm::GameTimer_Tick);
    gameTimer->Start();
  }

  void GameTimer_Tick(Object^ sender, EventArgs^ e) {
    elapsedSeconds++;
    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;
    timerLabel->Text = String::Format("{0:D2}:{1:D2}", minutes, seconds);
  }

  void ResetTimer() {
    elapsedSeconds = 0;
    timerLabel->Text = "00:00";
  }

  void UpdateDebugBox(Object^ sender, EventArgs^ e) {
      char* msg;
      while ((msg = sudoku->NativeSudoku->get_next_debug_message()) != nullptr) {
          std::string unixMsg(msg);
          std::string dosMsg = unix2dos(unixMsg);
          String^ managedMsg = gcnew String(dosMsg.c_str());
          debugBox->AppendText(managedMsg);
          debugBox->SelectionStart = debugBox->Text->Length;
          debugBox->ScrollToCaret();
      }
  }

  void Form_Resize(Object^ sender, EventArgs^ e) {
    int gridTop = menuStrip->Height + toolStrip->Height + instructionsBox->Height + 25;
    int availableWidth = this->ClientSize.Width - 100;  // 50px margin left, 50px for debug box
    int availableHeight = this->ClientSize.Height - gridTop - statusStrip->Height - 20;
    
    // Grid proportions: 60px per cell × 9 = 540 wide
    //                   70px per cell × 9 = 630 tall (includes 25px notes area)
    // Aspect ratio = 540:630 = 6:7
    
    // Calculate grid size while maintaining aspect ratio
    int gridWidth = availableWidth;
    int gridHeight = (gridWidth * 7) / 6;  // Maintain 6:7 aspect ratio
    
    // If height is too large, scale down based on height instead
    if (gridHeight > availableHeight) {
      gridHeight = availableHeight;
      gridWidth = (gridHeight * 6) / 7;  // Maintain 6:7 aspect ratio
    }
    
    // Minimum sizes
    gridWidth = System::Math::Max(gridWidth, 240);   // At least 60px × 4 cells
    gridHeight = System::Math::Max(gridHeight, 280);  // At least 70px × 4 cells
    
    gridContainer->Size = System::Drawing::Size(gridWidth, gridHeight);
    gridContainer->Location = Point(50, gridTop);
    
    // Calculate cell sizes (width and height can be different)
    int cellWidth = gridWidth / 9;
    int cellHeight = gridHeight / 9;
    int mainCellHeight = (cellHeight * 45) / 70;  // 45 pixels for main cell out of 70 total
    int notesCellHeight = cellHeight - mainCellHeight;  // Remaining for notes
    
    // Update all cells and notes boxes
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        // Update main cell
        grid[i, j]->Size = System::Drawing::Size(cellWidth, mainCellHeight);
        grid[i, j]->Location = System::Drawing::Point(j * cellWidth, i * cellHeight);
        
        // Scale font based on cell size
        float fontSize = System::Math::Max(8.0f, (float)mainCellHeight * 0.5f);
        grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", fontSize);
        
        // Update notes box
        notes[i, j]->Size = System::Drawing::Size(cellWidth, notesCellHeight);
        notes[i, j]->Location = System::Drawing::Point(j * cellWidth, i * cellHeight + mainCellHeight);
        
        // Scale notes font based on notes area size
        float notesFontSize = System::Math::Max(6.0f, (float)notesCellHeight * 0.4f);
        notes[i, j]->Font = gcnew System::Drawing::Font(L"Arial", notesFontSize);
      }
    }
    
    // Update grid lines
    gridContainer->Controls->Clear();
    for (int i = 0; i <= 9; i++) {
      int thickness = (i % 3 == 0) ? 3 : 1;
      int offset = (i % 3 == 0) ? 1 : 0;
      
      // Vertical lines
      Panel ^ vline = gcnew Panel();
      vline->BorderStyle = BorderStyle::None;
      vline->Location = Point(i * cellWidth - offset, 0);
      vline->Size = System::Drawing::Size(thickness, gridHeight);
      vline->BackColor = Color::Black;
      gridContainer->Controls->Add(vline);

      // Horizontal lines
      Panel ^ hline = gcnew Panel();
      hline->BorderStyle = BorderStyle::None;
      hline->Location = Point(0, i * cellHeight - offset);
      hline->Size = System::Drawing::Size(gridWidth, thickness);
      hline->BackColor = Color::Black;
      gridContainer->Controls->Add(hline);
    }
    
    // Re-add all cells and notes on top of grid lines
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        gridContainer->Controls->Add(grid[i, j]);
        gridContainer->Controls->Add(notes[i, j]);
      }
    }
    
    // Update debug box position and size
    debugBox->Location = Point(gridContainer->Right + 20, gridTop);
    debugBox->Size = System::Drawing::Size(availableWidth - gridWidth - 70, gridHeight);
  }

private: void SafeSetClipboard(DataObject^ data) {
    try {
        Clipboard::SetDataObject(data, true, 3, 100); // Retry up to 3 times with 100ms delay
    }
    catch (Exception^ ex) {
        UpdateStatus("Failed to set clipboard: " + ex->Message);
    }
}


  void ClearDebugBox() {
      debugBox->Clear();
  }

  void UpdateGrid() {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        int value = sudoku->GetValue(i, j);
        grid[i, j]->Text = (value >= 0) ? (value + 1).ToString() : "";
        
        // Check if this cell is immutable (from generated puzzle)
        if (sudoku->IsCellImmutable(i, j)) {
          // Original clues: white background, salmon bold text
          grid[i, j]->BackColor = Color::White;
          grid[i, j]->ForeColor = Color::Salmon;
          grid[i, j]->ReadOnly = true;
          grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Bold);
        } else if (sudoku->IsCellQuasiImmutable(i, j)) {
          // Earned locks: light green background, dark green text
          grid[i, j]->BackColor = Color::LightGreen;
          grid[i, j]->ForeColor = Color::DarkGreen;
          grid[i, j]->ReadOnly = true;
          grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Bold);
        } else {
          // Regular cells
          grid[i, j]->BackColor = Color::White;
          grid[i, j]->ForeColor = Color::Black;
          grid[i, j]->ReadOnly = false;
          grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Regular);
        }
        
        ClearNotes(i, j);  // Clear notes when updating grid
      }
    }
    ValidateAndHighlight();
    CheckForWin();
    // Restore block cursor on currently focused cell
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 9; j++)
        if (grid[i, j]->Focused) grid[i, j]->SelectAll();
  }

  String^ GetBoardAsText() {
        String^ result = "<table border='1' style='border-collapse: collapse; border: 2px solid black;'>";
    
        // Generate the HTML table
        for (int i = 0; i < 9; i++) {
            result += "<tr style='height: 30px;'>";
            for (int j = 0; j < 9; j++) {
                // Add cell with appropriate borders
                String^ borderStyle = "border: 1px solid #ccc;";
                if (i % 3 == 0) borderStyle += "border-top: 2px solid black;";
                if (i == 8) borderStyle += "border-bottom: 2px solid black;";
                if (j % 3 == 0) borderStyle += "border-left: 2px solid black;";
                if (j == 8) borderStyle += "border-right: 2px solid black;";

                result += "<td style='width: 30px; text-align: center; " + borderStyle + "'>";
            
                // Get cell value
                String^ value = grid[i, j]->Text;
                result += String::IsNullOrEmpty(value) ? "&nbsp;" : value;
            
                result += "</td>";
            }
            result += "</tr>";
        }
        result += "</table>";
        return result;
    }

void CopyBoard_Click(Object^ sender, EventArgs^ e) {
    try {
        // Create DataObject to hold formats
        DataObject^ dataObj = gcnew DataObject();

        // Create HTML content
        String^ htmlContent = "<div style='font-family: Arial, sans-serif;'>";
        htmlContent += "<h2 style='text-align: center; color: #333;'>Sudoku Puzzle</h2></div>";
        htmlContent += "<table style='border-collapse: collapse; border: 2px solid black; margin: 0 auto;'>";
        htmlContent += "<table style='border-collapse: collapse; border: 2px solid black;'>";
        for (int i = 0; i < 9; i++) {
            htmlContent += "<tr>";
            for (int j = 0; j < 9; j++) {
                String^ style = "width: 30px; height: 30px; text-align: center; ";
                
                // Add border styles
                if (j % 3 == 0) style += "border-left: 2px solid black; ";
                else style += "border-left: 1px solid #ccc; ";
                
                if (j == 8) style += "border-right: 2px solid black; ";
                
                if (i % 3 == 0) style += "border-top: 2px solid black; ";
                else style += "border-top: 1px solid #ccc; ";
                
                if (i == 8) style += "border-bottom: 2px solid black; ";
                else style += "border-bottom: 1px solid #ccc; ";
                
                htmlContent += "<td style='" + style + "'>";
                String^ value = grid[i, j]->Text;
                htmlContent += String::IsNullOrEmpty(value) ? "&nbsp;" : value;
                htmlContent += "</td>";
            }
            htmlContent += "</tr>";
        }
        htmlContent += "</table>";

        // Create the HTML clipboard format header
        String^ htmlHeader = "Version:0.9\r\n" +
            "StartHTML:<<<<<1>>>>>\r\n" +
            "EndHTML:<<<<<2>>>>>\r\n" +
            "StartFragment:<<<<<3>>>>>\r\n" +
            "EndFragment:<<<<<4>>>>>\r\n";

        String^ htmlStart = "<html><body><!--StartFragment-->";
        String^ htmlEnd = "<!--EndFragment--></body></html>";
        
        String^ fullHtml = htmlStart + htmlContent + htmlEnd;
        
        // Calculate positions
        int startHtml = htmlHeader->Length;
        int startFragment = startHtml + htmlStart->Length;
        int endFragment = startFragment + htmlContent->Length;
        int endHtml = endFragment + htmlEnd->Length;
        
        // Replace position markers
        htmlHeader = htmlHeader->Replace("<<<<<1>>>>>", startHtml.ToString("D8"));
        htmlHeader = htmlHeader->Replace("<<<<<2>>>>>", endHtml.ToString("D8"));
        htmlHeader = htmlHeader->Replace("<<<<<3>>>>>", startFragment.ToString("D8"));
        htmlHeader = htmlHeader->Replace("<<<<<4>>>>>", endFragment.ToString("D8"));
        
        // Combine everything
        String^ clipboardHtml = htmlHeader + fullHtml;
        
        // Set HTML format
        dataObj->SetData(DataFormats::Html, clipboardHtml);
        
        // Add plain text as fallback
        String^ plainText = "Sudoku Puzzle\r\n";
        for (int i = 0; i < 9; i++) {
            if (i % 3 == 0) {
                plainText += "+----+----+----+----+----+----+\r\n";
            }
            for (int j = 0; j < 9; j++) {
                if (j % 3 == 0) plainText += "|";
                String^ value = grid[i, j]->Text;
                plainText += String::IsNullOrEmpty(value) ? " . " : " " + value + " ";
            }
            plainText += "|\r\n";
        }
        plainText +=         "+----+----+----+----+----+----+\r\n";
        dataObj->SetData(DataFormats::Text, plainText);

        // Use BeginInvoke to marshal the clipboard operation to the UI thread
        this->BeginInvoke(gcnew Action<DataObject^>(this, &MainForm::SafeSetClipboard), dataObj);
        
        UpdateStatus("Board copied to clipboard");
    }
    catch (Exception^ ex) {
        UpdateStatus("Failed to copy board to clipboard: " + ex->Message);
        System::Diagnostics::Debug::WriteLine(ex->ToString());
    }
}


  void Cell_GotFocus(Object^ sender, EventArgs^ e) {
    TextBox^ tb = safe_cast<TextBox^>(sender);
    array<int>^ pos = safe_cast<array<int>^>(tb->Tag);
    int val = sudoku->GetValue(pos[0], pos[1]);
    highlightValue = val; // -1 if empty, highlights matching digits
    ValidateAndHighlight();
    tb->SelectAll();
  }

  void Cell_LostFocus(Object^ sender, EventArgs^ e) {
    TextBox^ tb = safe_cast<TextBox^>(sender);
    highlightValue = -1;
    ValidateAndHighlight();
    tb->SelectionLength = 0;
  }

  void Cell_MouseWheel(Object^ sender, MouseEventArgs^ e) {
      TextBox^ textBox = safe_cast<TextBox^>(sender);
      array<int>^ position = safe_cast<array<int>^>(textBox->Tag);
      int row = position[0];
      int col = position[1];
    
      // Get current value
      int currentValue = -1;  // -1 represents blank
      if (!String::IsNullOrEmpty(textBox->Text)) {
          Int32::TryParse(textBox->Text, currentValue);
          currentValue--;  // Convert to 0-8 range
      }  
    
      // Calculate new value based on wheel direction
      if (e->Delta > 0) {  // Mouse wheel up
          currentValue = (currentValue + 1) % 10;  // 10 states: -1 to 8
      } else {  // Mouse wheel down
          currentValue = (currentValue - 1);
          if (currentValue < -1) currentValue = 8;
      }
    
      // Update the cell
      if (currentValue == -1) {
          sudoku->ClearValue(row, col);
          textBox->Text = "";
      } else {
          sudoku->Clean();
          sudoku->SetValue(row, col, currentValue);
          textBox->Text = (currentValue + 1).ToString();
      }
  }
  
  void Cell_MouseDown(Object^ sender, MouseEventArgs^ e) {
      if (e->Button == System::Windows::Forms::MouseButtons::Middle) {
          TextBox^ textBox = safe_cast<TextBox^>(sender);
          array<int>^ position = safe_cast<array<int>^>(textBox->Tag);
          int row = position[0];
          int col = position[1];
        
          // Clear the cell
          sudoku->ClearValue(row, col);
          textBox->Text = "";
      }
  }

  // Method to clear notes from a cell
  void ClearNotes(int row, int col) {
      notes[row, col]->Text = "";
  }

  // Method to add a candidate number to notes (useful for programmatic usage)
  void AddToNotes(int row, int col, String^ number) {
      String^ currentNotes = notes[row, col]->Text;
      if (!currentNotes->Contains(number)) {
          if (currentNotes->Length > 0) {
              notes[row, col]->Text = currentNotes + " " + number;
          } else {
              notes[row, col]->Text = number;
          }
      }
  }

  delegate void SolverDelegate();

  // Helper class to capture context for background thread
  ref class SolverRunner {
  public:
    MainForm^ form;
    SolverDelegate^ work;
    String^ msg;
    SolverRunner(MainForm^ f, SolverDelegate^ w, String^ m) : form(f), work(w), msg(m) {}
    void Run() {
      work->Invoke();
      array<Object^>^ args = gcnew array<Object^>{msg};
      form->BeginInvoke(gcnew System::Action<String^>(form, &MainForm::SolverFinished), args);
    }
  };

  void RunSolverAsync(SolverDelegate^ solverWork, String^ completionMsg) {
    if (isSolving) {
      UpdateStatus("Solver is already running...");
      return;
    }
    if (!sudoku->IsValidSolution()) {
      UpdateStatus("Current board is invalid");
      return;
    }
    isSolving = true;
    UpdateStatus("Solving...");

    SolverRunner^ runner = gcnew SolverRunner(this, solverWork, completionMsg);
    System::Threading::Thread^ t = gcnew System::Threading::Thread(
      gcnew System::Threading::ThreadStart(runner, &SolverRunner::Run)
    );
    t->IsBackground = true;
    t->Start();
  }

  void SolverFinished(String^ msg) {
    isSolving = false;
    ClearDebugBox();
    UpdateGrid();
    UpdateStatus(msg);
  }

  // Sound helpers - all played on background thread so they don't block UI
  ref class SoundPlayer {
  public:
    int freq, duration;
    SoundPlayer(int f, int d) : freq(f), duration(d) {}
    void Play() { ::Beep(freq, duration); }
  };

  ref class SoundSequence {
  public:
    array<int>^ freqs;
    array<int>^ durations;
    SoundSequence(array<int>^ f, array<int>^ d) : freqs(f), durations(d) {}
    void Play() {
      for (int i = 0; i < freqs->Length; i++) {
        if (freqs[i] == 0)
          System::Threading::Thread::Sleep(durations[i]);
        else
          ::Beep(freqs[i], durations[i]);
      }
    }
  };

  void PlaySoundAsync(int freq, int duration) {
    SoundPlayer^ sp = gcnew SoundPlayer(freq, duration);
    System::Threading::Thread^ t = gcnew System::Threading::Thread(
      gcnew System::Threading::ThreadStart(sp, &SoundPlayer::Play));
    t->IsBackground = true;
    t->Start();
  }

  void PlaySequenceAsync(array<int>^ freqs, array<int>^ durations) {
    SoundSequence^ ss = gcnew SoundSequence(freqs, durations);
    System::Threading::Thread^ t = gcnew System::Threading::Thread(
      gcnew System::Threading::ThreadStart(ss, &SoundSequence::Play));
    t->IsBackground = true;
    t->Start();
  }

  // New game started - cheerful ascending arpeggio
  void PlayNewGameSound() {
    PlaySequenceAsync(
      gcnew array<int>  {392, 523, 659, 784, 1047},
      gcnew array<int>  { 80,  80,  80,  80,  200});
  }

  // Wrong digit entered
  void PlayWrongSound() {
    PlaySoundAsync(200, 150);
  }

  // Correct digit entered (not yet quasi-immutable)
  void PlayCorrectSound() {
    PlaySoundAsync(880, 80);
  }

  // Cell becomes quasi-immutable (streak of 5 correct)
  void PlayQuasiImmutableSound() {
    PlaySequenceAsync(
      gcnew array<int>  {523, 659, 784, 1047},
      gcnew array<int>  { 80,  80,  80,  160});
  }

  // Puzzle solved - victory fanfare
  void PlayWinSound() {
    PlaySequenceAsync(
      gcnew array<int>  {523, 523,   0, 523,   0, 415, 523,   0, 784,   0, 392,   0, 523},
      gcnew array<int>  {100, 100,  50, 100,  50, 100, 100, 100, 400, 200, 400, 200, 500});
  }

  // Returns the correct value (0-8) for a cell by solving a copy, or -1 if unsolvable
  int GetCorrectValue(int row, int col) {
    Sudoku* copy = new Sudoku();
    memcpy(copy->board, sudoku->NativeSudoku->board, sizeof(copy->board));
    copy->Solve();
    int val = copy->GetValue(row, col);
    delete copy;
    return val;
  }

  void HintCell() {
    // Solve a copy to find the answer, then place one random unknown cell
    Sudoku* copy = new Sudoku();
    memcpy(copy->board, sudoku->NativeSudoku->board, sizeof(copy->board));
    copy->Solve();

    // Collect all empty cells
    System::Collections::Generic::List<array<int>^>^ emptyCells =
        gcnew System::Collections::Generic::List<array<int>^>();
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 9; j++)
        if (sudoku->GetValue(i, j) == -1)
          emptyCells->Add(gcnew array<int>{i, j});

    if (emptyCells->Count == 0) {
      delete copy;
      UpdateStatus("No empty cells to fill");
      return;
    }

    // Pick a random empty cell
    Random^ rng = gcnew Random();
    array<int>^ cell = emptyCells[rng->Next(emptyCells->Count)];
    int row = cell[0], col = cell[1];
    int solvedVal = copy->GetValue(row, col);
    delete copy;

    if (solvedVal == -1) {
      UpdateStatus("Cheat: Could not solve current board");
      return;
    }

    sudoku->SaveBoardState();
    sudoku->SetValue(row, col, solvedVal);
    UpdateGrid();
    UpdateUndoButtonState();
    UpdateStatus("Cheat: placed " + (solvedVal + 1).ToString() +
                 " at row " + (row + 1).ToString() + ", col " + (col + 1).ToString());
  }

  void CheckForWin() {
    if (puzzleSolved || currentDifficulty == "") return;

    // Check all 81 cells are filled
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 9; j++)
        if (sudoku->GetValue(i, j) == -1) return;

    // Check no conflicts (no red cells) - same logic as ValidateAndHighlight
    for (int row = 0; row < 9; row++) {
      for (int col = 0; col < 9; col++) {
        int value = sudoku->GetValue(row, col);
        // Check row
        for (int c = 0; c < 9; c++)
          if (c != col && sudoku->GetValue(row, c) == value) return;
        // Check column
        for (int r = 0; r < 9; r++)
          if (r != row && sudoku->GetValue(r, col) == value) return;
        // Check 3x3 box
        int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
        for (int r = boxRow; r < boxRow + 3; r++)
          for (int c = boxCol; c < boxCol + 3; c++)
            if ((r != row || c != col) && sudoku->GetValue(r, c) == value) return;
      }
    }

    puzzleSolved = true;
    gameTimer->Stop();

    // Lock all user-entered cells as quasi-immutable (green)
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 9; j++)
        if (!sudoku->IsCellImmutable(i, j) && !sudoku->IsCellQuasiImmutable(i, j))
          sudoku->SetQuasiImmutable(i, j);
    UpdateGrid();

    PlayWinSound();

    int mins = elapsedSeconds / 60;
    int secs = elapsedSeconds % 60;
    String^ timeStr = String::Format("{0:D2}:{1:D2}", mins, secs);
    String^ diff = currentDifficulty;

    // Convert to std string for highscores
    std::string diffStd = msclr::interop::marshal_as<std::string>(diff);
    bool isHigh = highscores->isHighScore(elapsedSeconds, diffStd);

    String^ msg = "Congratulations! You solved the " + diff + " puzzle in " + timeStr + "!";
    if (isHigh) msg += "\r\nNew high score! Enter your name:";

    if (isHigh) {
      // Simple name input dialog
      Form^ inputDlg = gcnew Form();
      inputDlg->Text = "New High Score!";
      inputDlg->Size = System::Drawing::Size(340, 160);
      inputDlg->StartPosition = FormStartPosition::CenterParent;
      inputDlg->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
      inputDlg->MaximizeBox = false;

      Label^ lbl = gcnew Label();
      lbl->Text = msg + "\r\nEnter your name:";
      lbl->Location = Point(10, 10);
      lbl->Size = System::Drawing::Size(310, 60);
      inputDlg->Controls->Add(lbl);

      TextBox^ nameTxt = gcnew TextBox();
      nameTxt->Text = "Player";
      nameTxt->Location = Point(10, 75);
      nameTxt->Size = System::Drawing::Size(200, 25);
      inputDlg->Controls->Add(nameTxt);

      Button^ okBtn = gcnew Button();
      okBtn->Text = "OK";
      okBtn->Location = Point(220, 75);
      okBtn->DialogResult = System::Windows::Forms::DialogResult::OK;
      inputDlg->Controls->Add(okBtn);
      inputDlg->AcceptButton = okBtn;

      String^ name = "Anonymous";
      if (inputDlg->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
        name = nameTxt->Text->Trim();
        if (name == "") name = "Anonymous";
      }

      Score score;
      score.name = msclr::interop::marshal_as<std::string>(name);
      score.time = elapsedSeconds;
      score.difficulty = diffStd;
      highscores->addScore(score);
      ShowHighscoresDialog(diff);
    } else {
      MessageBox::Show(this, msg, "Puzzle Solved!", MessageBoxButtons::OK, MessageBoxIcon::Information);
    }

    UpdateStatus("Puzzle solved in " + timeStr + "!");
  }

  void ShowHighscoresDialog(String^ highlightDiff) {
    Form^ dlg = gcnew Form();
    dlg->Text = "High Scores";
    dlg->Size = System::Drawing::Size(420, 480);
    dlg->StartPosition = FormStartPosition::CenterParent;
    dlg->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
    dlg->MaximizeBox = false;

    TabControl^ tabs = gcnew TabControl();
    tabs->Dock = DockStyle::Fill;

    array<String^>^ difficulties = {"easy","medium","hard","master","expert"};
    for each (String^ d in difficulties) {
      TabPage^ page = gcnew TabPage(d->Substring(0,1)->ToUpper() + d->Substring(1));

      ListView^ lv = gcnew ListView();
      lv->Dock = DockStyle::Fill;
      lv->View = View::Details;
      lv->FullRowSelect = true;
      lv->GridLines = true;
      lv->Columns->Add("Rank", 50);
      lv->Columns->Add("Name", 180);
      lv->Columns->Add("Time", 100);

      std::string dStd = msclr::interop::marshal_as<std::string>(d);
      auto scores = highscores->getScoresByDifficulty(dStd);
      int rank = 1;
      for (const auto& s : scores) {
        int m = s.time / 60, sec = s.time % 60;
        String^ t = String::Format("{0:D2}:{1:D2}", m, sec);
        String^ n = gcnew String(s.name.c_str());
        ListViewItem^ item = gcnew ListViewItem(rank.ToString());
        item->SubItems->Add(n);
        item->SubItems->Add(t);
        if (d == highlightDiff && rank == 1)
          item->BackColor = Color::LightGoldenrodYellow;
        lv->Items->Add(item);
        rank++;
      }
      if (scores.empty()) {
        ListViewItem^ item = gcnew ListViewItem("-");
        item->SubItems->Add("No scores yet");
        item->SubItems->Add("-");
        lv->Items->Add(item);
      }

      page->Controls->Add(lv);
      tabs->TabPages->Add(page);

      // Select the tab matching highlight difficulty
      if (d == highlightDiff)
        tabs->SelectedTab = page;
    }

    dlg->Controls->Add(tabs);

    Button^ closeBtn = gcnew Button();
    closeBtn->Text = "Close";
    closeBtn->DialogResult = System::Windows::Forms::DialogResult::OK;
    closeBtn->Dock = DockStyle::Bottom;
    dlg->Controls->Add(closeBtn);

    dlg->ShowDialog(this);
  }

  void ViewHighscores_Click(Object^ sender, EventArgs^ e) {
    ShowHighscoresDialog("");
  }

  void Form_Deactivate(Object^ sender, EventArgs^ e) {
    if (!timerPaused) {
      timerPaused = true;
      gameTimer->Stop();
      UpdateStatus("Game paused");
    }
  }

  void Form_Activate(Object^ sender, EventArgs^ e) {
    if (timerPaused && !puzzleSolved) {
      timerPaused = false;
      gameTimer->Start();
      UpdateStatus("Game resumed");
    }
  }

  void FontSize_Click(Object^ sender, EventArgs^ e) {
    ToolStripMenuItem^ item = safe_cast<ToolStripMenuItem^>(sender);
    gridFontSize = safe_cast<float>(item->Tag);
    // Apply to all grid cells
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 9; j++)
        grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", gridFontSize);
    ValidateAndHighlight();
    UpdateStatus("Font size changed");
  }

  void ColorblindMode_Click(Object^ sender, EventArgs^ e) {
    colorblindMode = !colorblindMode;
    ToolStripMenuItem^ item = safe_cast<ToolStripMenuItem^>(sender);
    item->Checked = colorblindMode;
    // In colorblind mode: immutable = bold+underline, quasi = bold+italic, normal = regular
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        if (sudoku->IsCellImmutable(i, j) || sudoku->IsCellQuasiImmutable(i, j)) {
          // Font style set by ValidateAndHighlight
        } else {
          grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", gridFontSize,
            System::Drawing::FontStyle::Regular);
        }
      }
    }
    ValidateAndHighlight();
    UpdateStatus(colorblindMode ? "Colorblind mode on" : "Colorblind mode off");
  }

  void UpdateStatus(String ^ message) {
    statusLabel->Text = message;
    statusStrip->Refresh();
    debugBox->AppendText(message + "\r\n");
    debugBox->SelectionStart = debugBox->Text->Length;
    debugBox->ScrollToCaret();

    /*sudoku->print_debug(message);*/

  }

  void ValidateAndHighlight() {
    // Reset all cells to their base color/style
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        if (sudoku->IsCellImmutable(i, j)) {
          if (colorblindMode) {
            grid[i, j]->BackColor = Color::White;
            grid[i, j]->ForeColor = Color::Black;
            grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", gridFontSize,
              System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Underline);
          } else {
            grid[i, j]->BackColor = Color::White;
            grid[i, j]->ForeColor = Color::Salmon;
          }
        } else if (sudoku->IsCellQuasiImmutable(i, j)) {
          if (colorblindMode) {
            grid[i, j]->BackColor = Color::White;
            grid[i, j]->ForeColor = Color::Black;
            grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", gridFontSize,
              System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic);
          } else {
            grid[i, j]->BackColor = Color::LightGreen;
            grid[i, j]->ForeColor = Color::DarkGreen;
          }
        } else {
          grid[i, j]->BackColor = Color::White;
          grid[i, j]->ForeColor = Color::Black;
        }
      }
    }

    // Same-number highlight
    if (highlightValue >= 0) {
      for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
          if (sudoku->GetValue(i, j) == highlightValue &&
              grid[i, j]->BackColor != Color::Red)
            grid[i, j]->BackColor = Color::LightYellow;
    }

    // Conflict detection
    for (int row = 0; row < 9; row++) {
      for (int col = 0; col < 9; col++) {
        int value = sudoku->GetValue(row, col);
        if (value == -1) continue;

        bool hasConflict = false;
        for (int c = 0; c < 9; c++)
          if (c != col && sudoku->GetValue(row, c) == value) { hasConflict = true; break; }
        if (!hasConflict)
          for (int r = 0; r < 9; r++)
            if (r != row && sudoku->GetValue(r, col) == value) { hasConflict = true; break; }
        if (!hasConflict) {
          int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
          for (int r = boxRow; r < boxRow + 3 && !hasConflict; r++)
            for (int c = boxCol; c < boxCol + 3 && !hasConflict; c++)
              if ((r != row || c != col) && sudoku->GetValue(r, c) == value)
                hasConflict = true;
        }
        if (hasConflict) {
          grid[row, col]->BackColor = Color::Red;
          grid[row, col]->ForeColor = Color::White;
        }
      }
    }
  }

  void Cell_TextChanged(Object ^ sender, EventArgs ^ e) {
    TextBox ^ textBox = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(textBox->Tag);
    int col = position[0];
    int row = position[1];

    // Check if this cell is locked
    if (sudoku->IsCellLocked(col, row)) {
      return;
    }

    // For non-immutable cells, allow normal editing
    if (textBox->Text->Length > 0) {
      int value;
      if (Int32::TryParse(textBox->Text, value) && value >= 1 && value <= 9) {
        sudoku->Clean();
        sudoku->SetValue(col, row, value - 1);
        textBox->BackColor = Color::White;  // User-entered cells are white
        textBox->ForeColor = Color::Black;
      } else {
        textBox->Text = "";
      }
    } else {
      // Clear the cell when text is empty
      sudoku->ClearValue(col, row);
      textBox->BackColor = Color::White;
      textBox->ForeColor = Color::Black;
    }
    
    // Validate and highlight any conflicts
    ValidateAndHighlight();
  }

  void Cell_KeyPress(Object ^ sender, KeyPressEventArgs ^ e) {
    TextBox ^ textBox = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(textBox->Tag);
    int row = position[0];
    int col = position[1];
    
    // Block any number input (0-9) for locked cells
    if (sudoku->IsCellLocked(row, col)) {
      if (e->KeyChar >= '0' && e->KeyChar <= '9') {
        e->Handled = true;
        return;
      }
    }
    
    // Suppress the beep for all keys - let KeyDown handle everything
    e->Handled = true;
  }

  void Cell_KeyDown(Object ^ sender, KeyEventArgs ^ e) {
    TextBox ^ currentCell = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(currentCell->Tag);
    int row = position[0];
    int col = position[1];

    // Handle Ctrl+Z for undo
    if (e->Control && e->KeyCode == Keys::Z) {
      if (sudoku->CanUndo()) {
        sudoku->Undo();
        UpdateGrid();
        UpdateUndoButtonState();
        UpdateStatus("Undo completed - " + sudoku->GetUndoCount() + " states remaining");
      } else {
        UpdateStatus("Nothing to undo");
      }
      e->Handled = true;
      return;
    }

    switch (e->KeyCode) {
      case Keys::Left:
        if (col > 0) {
          grid[row, col - 1]->Focus();
          e->Handled = true;
        } else {
          grid[row, 8]->Focus();
          e->Handled = true;
        }
        break;
      case Keys::Right:
        if (col < 8) {
          grid[row, col + 1]->Focus();
          e->Handled = true;
        } else {
          grid[row, 0]->Focus();
          e->Handled = true;
        }
        break;
      case Keys::Up:
        if (row > 0) {
          grid[row - 1, col]->Focus();
          e->Handled = true;
        } else {
          grid[8, col]->Focus();
          e->Handled = true;
        }

        break;
      case Keys::Down:
        if (row < 8) {
          grid[row + 1, col]->Focus();
          e->Handled = true;
        } else {
          grid[0, col]->Focus();
          e->Handled = true;
        }
        break;
      case Keys::F1:
        if (!e->Shift) {
          GenerateEasy_Click(nullptr, nullptr);
        } else {
          GenerateMaster_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;

      case Keys::F2:
        if (!e->Shift) {
          GenerateMedium_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;

      case Keys::F3:
        if (!e->Shift) {
          GenerateHard_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;

      case Keys::F4:
        if (!e->Shift) {
          GenerateExpert_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;

      // Number input (1-9)
      case Keys::D1:
      case Keys::D2:
      case Keys::D3:
      case Keys::D4:
      case Keys::D5:
      case Keys::D6:
      case Keys::D7:
      case Keys::D8:
      case Keys::D9:
        if (sudoku->IsCellLocked(row, col)) {
          UpdateStatus("That cell is locked and cannot be changed");
          e->Handled = true;
          break;
        }
        {
          int enteredVal = (int)e->KeyCode - (int)Keys::D1;
          int correctVal = GetCorrectValue(row, col);
          sudoku->SaveBoardState();
          sudoku->Clean();
          sudoku->SetValue(row, col, enteredVal);
          // Auto-clear pencil marks in same row, col, and box
          for (int c = 0; c < 9; c++) ClearNotes(row, c);
          for (int r = 0; r < 9; r++) ClearNotes(r, col);
          int boxR = (row / 3) * 3, boxC = (col / 3) * 3;
          for (int r = boxR; r < boxR + 3; r++)
            for (int c = boxC; c < boxC + 3; c++)
              ClearNotes(r, c);
          UpdateGrid();
          UpdateUndoButtonState();
          // Track sliding window for quasi-immutable locking
          if (correctVal != -1 && enteredVal == correctVal) {
            correctQueue->Enqueue(gcnew array<int>{row, col});
            if (correctQueue->Count >= 5) {
              array<int>^ toLock = correctQueue->Dequeue();
              sudoku->SetQuasiImmutable(toLock[0], toLock[1]);
              UpdateGrid();
              PlayQuasiImmutableSound();
              UpdateStatus("Correct! Cell locked.");
            } else {
              PlayCorrectSound();
            }
          } else {
            PlayWrongSound();
            correctQueue->Clear();
          }
        }
        e->Handled = true;
        break;

      // Clear cell with 0
      case Keys::D0:
        if (sudoku->IsCellLocked(row, col)) {
          UpdateStatus("That cell is locked and cannot be changed");
          e->Handled = true;
          break;
        }
        correctQueue->Clear();
        sudoku->SaveBoardState();
        sudoku->ClearValue(row, col);
        UpdateGrid();
        UpdateUndoButtonState();
        e->Handled = true;
        break;

      // Solving techniques
      case Keys::S:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::StdElim), "Standard elimination completed");
        e->Handled = true;
        break;
      case Keys::L:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::LinElim), "Line elimination completed");
        e->Handled = true;
        break;
      case Keys::H:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindHiddenPairs), "Hidden pairs completed");
        e->Handled = true;
        break;
      case Keys::P:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindPointingPairs), "Pointing pairs completed");
        e->Handled = true;
        break;
      case Keys::N:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindHiddenSingles), "Hidden singles completed");
        e->Handled = true;
        break;
      case Keys::K:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindNakedSets), "Naked sets completed");
        e->Handled = true;
        break;
      case Keys::X:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindXWing), "X-Wing technique completed");
        e->Handled = true;
        break;
      case Keys::F:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindSwordFish), "Swordfish technique completed");
        e->Handled = true;
        break;
      case Keys::Y:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindXYWing), "XY-Wing technique completed");
        e->Handled = true;
        break;
      case Keys::OemSemicolon:
        if (!e->Shift) {
          RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindXYZWing), "XYZ-Wing technique completed");
          e->Handled = true;
        }
        break;
      case Keys::A:
        RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::Solve), "Full solve completed");
        e->Handled = true;
        break;
      case Keys::Z:
        ClearDebugBox();
        sudoku->NewGame();
        UpdateGrid();
        UpdateStatus("New game started");
        e->Handled = true;
        break;
      case Keys::T:
        // Toggle notes
        notesVisible = !notesVisible;
        for (int i = 0; i < 9; i++) {
          for (int j = 0; j < 9; j++) {
            notes[i, j]->Visible = notesVisible;
          }
        }
        UpdateStatus(notesVisible ? "Notes shown" : "Notes hidden");
        e->Handled = true;
        break;
      case Keys::F5:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_1.txt")) {
            ClearDebugBox();
            elapsedSeconds = sudoku->savedElapsedSeconds;
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_1.txt");
          } else {
            UpdateStatus("Failed to load sudoku_1.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_1.txt", elapsedSeconds);
          UpdateStatus("Game saved to sudoku_1.txt");
        }
        e->Handled = true;
        break;
      case Keys::F6:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_2.txt")) {
            ClearDebugBox();
            elapsedSeconds = sudoku->savedElapsedSeconds;
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_2.txt");
          } else {
            UpdateStatus("Failed to load sudoku_2.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_2.txt", elapsedSeconds);
          UpdateStatus("Game saved to sudoku_2.txt");
        }
        e->Handled = true;
        break;
      case Keys::F7:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_3.txt")) {
            ClearDebugBox();
            elapsedSeconds = sudoku->savedElapsedSeconds;
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_3.txt");
          } else {
            UpdateStatus("Failed to load sudoku_3.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_3.txt", elapsedSeconds);
          UpdateStatus("Game saved to sudoku_3.txt");
        }
        e->Handled = true;
        break;
      case Keys::F8:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_4.txt")) {
            ClearDebugBox();
            elapsedSeconds = sudoku->savedElapsedSeconds;
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_4.txt");
          } else {
            UpdateStatus("Failed to load sudoku_4.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_4.txt", elapsedSeconds);
          UpdateStatus("Game saved to sudoku_4.txt");
        }
        e->Handled = true;
        break;
      case Keys::F9:
        if (!e->Shift) {
          sudoku->ExportToExcelXML("puzzle1.xml");
        }
        e->Handled = true;
        break;
      case Keys::F10:
        if (!e->Shift) {
          sudoku->ExportToExcelXML("puzzle2.xml");
        }
        e->Handled = true;
        break;
      case Keys::F11:
        if (e->Shift) {
          RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::Solve), "Full solve completed");
        } else {
          sudoku->ExportToExcelXML("puzzle3.xml");
        }
        e->Handled = true;
        break;
      case Keys::F12:
        if (e->Shift) {
          HintCell();
        } else {
          sudoku->ExportToExcelXML("puzzle4.xml");
        }
        e->Handled = true;
        break;


    }
  }

  // Menu event handlers
  void NewGame_Click(Object ^ sender, EventArgs ^ e) {
    sudoku->NewGame();
    sudoku->ClearUndoHistory();
    UpdateGrid();
    UpdateStatus("New game started");
  }

  void ToggleNotes_Click(Object ^ sender, EventArgs ^ e) {
    notesVisible = !notesVisible;
    
    // Toggle visibility of all notes boxes
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        notes[i, j]->Visible = notesVisible;
      }
    }
    
    // Update status message
    if (notesVisible) {
      UpdateStatus("Notes shown");
    } else {
      UpdateStatus("Notes hidden");
    }
  }

  void Load_Click(Object ^ sender, EventArgs ^ e) {
    sudoku->LoadFromFile("sudoku_1.txt");
    UpdateStatus("Game loaded successfully (sudoku_1.txt)");
    UpdateGrid();
  }

  void Save_Click(Object ^ sender, EventArgs ^ e) {
    sudoku->SaveToFile("sudoku_1.txt", elapsedSeconds);
    UpdateStatus("Game saved successfully (sudoku_1.txt)");
  }

  void Exit_Click(Object ^ sender, EventArgs ^ e) {
     Application::Exit(); 
  }

  void About_Click(Object ^ sender, EventArgs ^ e) {
    String^ aboutText = 
      "Sudoku Game\r\n\r\n" +
      "(C) 2026 Jason Brian Hall\r\n" +
      "MIT License - https://opensource.org/licenses/MIT\r\n\r\n" +
      "GitHub: https://github.com/jasonbrianhall/sudoku_solver\r\n\r\n" +
      "A feature-rich Sudoku game with puzzle generation across five difficulty levels, " +
      "real-time conflict detection, and a progressive locking system that rewards correct answers.\r\n\r\n" +
      "Features:\r\n" +
      "- Generate puzzles at 5 difficulty levels (Easy to Expert)\r\n" +
      "- Real-time conflict detection and highlighting\r\n" +
      "- Progressive cell locking - solve 5 correct in a row to earn green locks\r\n" +
      "- Pencil mark notes for candidates\r\n" +
      "- Game timer with save/load (timer persists across saves)\r\n" +
      "- High score tracking per difficulty\r\n" +
      "- Sound feedback for correct, incorrect, locks, and wins\r\n" +
      "- Save/load games across 4 slots\r\n";

    MessageBox::Show(this, aboutText, "About Sudoku Game", MessageBoxButtons::OK, MessageBoxIcon::Information);
  }

  void SupportAuthor_Click(Object ^ sender, EventArgs ^ e) {
    String^ supportText = 
      "If you enjoy Sudoku Game, please consider supporting the author!\r\n\r\n" +
      "Visit: https://buymeacoffee.com/jasonbrianhall\r\n\r\n" +
      "Your support helps fund development and keeps this project active.";

    MessageBox::Show(this, supportText, "Support the Author", MessageBoxButtons::OK, MessageBoxIcon::Information);
  }

  // Solving technique handlers
  void Solve_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::Solve), "Full solve completed");
  }

  void StdElim_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::StdElim), "Standard elimination completed");
  }

  void LineElim_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::LinElim), "Line elimination completed");
  }

  void HiddenSingles_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindHiddenSingles), "Hidden singles technique completed");
  }

  void HiddenPairs_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindHiddenPairs), "Hidden pairs technique completed");
  }

  void PointingPairs_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindPointingPairs), "Pointing pairs technique completed");
  }

  void NakedSets_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindNakedSets), "Naked sets technique completed");
  }

  void XWing_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindXWing), "X-Wing technique completed");
  }

  void Swordfish_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindSwordFish), "Swordfish technique completed");
  }

  void XYWing_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindXYWing), "XY-Wing technique completed");
  }

  void XYZWing_Click(Object ^ sender, EventArgs ^ e) {
    RunSolverAsync(gcnew SolverDelegate(sudoku, &SudokuWrapper::FindXYZWing), "XYZ-Wing technique completed");
  }

  void SaveSlot_Click(Object^ sender, EventArgs^ e) {
    ToolStripMenuItem^ menuItem = safe_cast<ToolStripMenuItem^>(sender);
    int slot = safe_cast<int>(menuItem->Tag);
    String^ filename = "sudoku_slot_" + slot + ".txt";
    sudoku->SaveToFile(filename, elapsedSeconds);
    UpdateStatus("Game saved to " + filename);
  }

  void LoadSlot_Click(Object^ sender, EventArgs^ e) {
    ToolStripMenuItem^ menuItem = safe_cast<ToolStripMenuItem^>(sender);
    int slot = safe_cast<int>(menuItem->Tag);
    String^ filename = "sudoku_slot_" + slot + ".txt";
    if (sudoku->LoadFromFile(filename)) {
      elapsedSeconds = sudoku->savedElapsedSeconds;
      UpdateGrid();
      UpdateStatus("Game loaded from " + filename);
    } else {
      UpdateStatus("Failed to load " + filename);
    }
  }

  void Undo_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->Undo()) {
      UpdateGrid();
      UpdateUndoButtonState();
      UpdateStatus("Undo completed - " + sudoku->GetUndoCount() + " states remaining");
    } else {
      UpdateStatus("Nothing to undo");
    }
  }

  void ClearBoard_Click(Object ^ sender, EventArgs ^ e) {
    System::Windows::Forms::DialogResult result = MessageBox::Show(
      this,
      "Clear all non-immutable cells? This action can be undone with Ctrl+Z.",
      "Clear Board",
      MessageBoxButtons::YesNo,
      MessageBoxIcon::Question
    );
    
    if (result == System::Windows::Forms::DialogResult::Yes) {
      sudoku->SaveBoardState();
      sudoku->ClearBoardExceptImmutable();
      UpdateGrid();
      UpdateUndoButtonState();
      UpdateStatus("Board cleared - immutable cells preserved");
    }
  }

  void UpdateUndoButtonState() {
    if (undoBtn != nullptr) {
      undoBtn->Enabled = sudoku->CanUndo();
      if (sudoku->CanUndo()) {
        undoCountLabel->Text = "Undo (" + sudoku->GetUndoCount() + ")";
      } else {
        undoCountLabel->Text = "Undo (0)";
      }
    }
  }



 public:
  MainForm() {
    sudoku = gcnew SudokuWrapper();
    highscores = new Highscores();
    currentDifficulty = "";
    puzzleSolved = false;
    isSolving = false;
    timerPaused = false;
    highlightValue = -1;
    colorblindMode = false;
    gridFontSize = 20.0f;
    correctQueue = gcnew System::Collections::Generic::Queue<array<int>^>();
    InitializeComponent();

    // Set the form icon
    try {
      System::Drawing::Icon ^ icon = gcnew System::Drawing::Icon(
          System::Reflection::Assembly::GetExecutingAssembly()
              ->GetManifestResourceStream("app.ico"));
      this->Icon = icon;
    } catch (Exception ^ ex) {
      System::Diagnostics::Debug::WriteLine("Failed to load icon: " +
                                            ex->Message);
    }

    // Auto-generate easy puzzle on startup (as if F1 was pressed)
    GenerateEasy_Click(nullptr, nullptr);
  }
};
}  // namespace SudokuGame


[STAThread]
int main(array<String ^> ^ args) {
  Application::EnableVisualStyles();
  Application::SetCompatibleTextRenderingDefault(false);
  Application::Run(gcnew SudokuGame::MainForm());
  return 0;
}
