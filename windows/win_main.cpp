// Microsoft Windows Native Version

#define _HAS_STD_BYTE 0
#include <msclr/marshal_cppstd.h>

#include "resource.h"
#include "sudoku.h"
#include "generatepuzzle.h"

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
  array<array<bool>^>^ immutableCells;  // Track which cells are from generated puzzle

 public:
  SudokuWrapper() { 
    nativeSudoku = new Sudoku(); 
    currentAlgorithm = AlgorithmType::ALG_NONE;
    
    // Initialize immutable cells array
    immutableCells = gcnew array<array<bool>^>(9);
    for (int i = 0; i < 9; i++) {
      immutableCells[i] = gcnew array<bool>(9);
      for (int j = 0; j < 9; j++) {
        immutableCells[i][j] = false;
      }
    }
  }

  ~SudokuWrapper() {
    if (nativeSudoku) {
      delete nativeSudoku;
      nativeSudoku = nullptr;
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
  
  // Reset immutability when starting new game
  void ClearImmutability() {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        immutableCells[i][j] = false;
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
    return nativeSudoku->LoadFromFile(std::string(wstr.begin(), wstr.end()));
  }

  void SaveToFile(String ^ filename) {
    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
    nativeSudoku->SaveToFile(std::string(wstr.begin(), wstr.end()));
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
  void FindSimpleColoring() {
    nativeSudoku->FindSimpleColoring();
  }  // This seems broken right now
  
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

  void GenerateEasy_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("easy")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();  // Mark all filled cells as immutable
      UpdateGrid();
      UpdateStatus("Generated new easy puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate easy puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateMedium_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("medium")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();  // Mark all filled cells as immutable
      UpdateGrid();
      UpdateStatus("Generated new medium puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate medium puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateHard_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("hard")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();  // Mark all filled cells as immutable
      UpdateGrid();
      UpdateStatus("Generated new hard puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate hard puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateExpert_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("expert")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();  // Mark all filled cells as immutable
      UpdateGrid();
      UpdateStatus("Generated new expert puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate expert puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }

  void GenerateMaster_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("extreme")) {
      sudoku->Clean();
      sudoku->MarkPuzzleAsGenerated();  // Mark all filled cells as immutable
      UpdateGrid();
      UpdateStatus("Generated new extreme puzzle - clues are immutable");
    } else {
      UpdateStatus("Failed to generate extreme puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
  }
      UpdateStatus("Generated new master puzzle");
    } else {
      UpdateStatus("Failed to generate master puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
    sudoku->Clean();
  }

  void InitializeComponent() {
    this->Size = System::Drawing::Size(800, 600);
    this->Text = L"Sudoku Solver";
    this->StartPosition = FormStartPosition::CenterScreen;

    // Initialize StatusStrip
    statusStrip = gcnew StatusStrip();
    statusLabel = gcnew ToolStripStatusLabel("Ready");
    statusStrip->Items->Add(statusLabel);
    this->Controls->Add(statusStrip);

    // Initialize MenuStrip
    menuStrip = gcnew MenuStrip();
    ToolStripMenuItem^ fileMenu = gcnew ToolStripMenuItem("File");
    ToolStripMenuItem^ generateBoardMenu = gcnew ToolStripMenuItem("Generate Board");



    fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "New Game (Z)", nullptr, gcnew EventHandler(this, &MainForm::NewGame_Click)));

    // Save Slots
    ToolStripMenuItem^ saveMenu = gcnew ToolStripMenuItem("Save Game");
    int k;
    for (int i = 1; i <= 4; i++) {
        k=i+4;
        ToolStripMenuItem^ saveSlot = gcnew ToolStripMenuItem("Slot " + i + " F(" + k + ")");
        saveSlot->Tag = i; // Store slot number in Tag
        saveSlot->Click += gcnew EventHandler(this, &MainForm::SaveSlot_Click);
        saveMenu->DropDownItems->Add(saveSlot);
    }

    // Load Slots
    ToolStripMenuItem^ loadMenu = gcnew ToolStripMenuItem("Load Game");
    for (int i = 1; i <= 4; i++) {
        k=i+4;
        ToolStripMenuItem^ loadSlot = gcnew ToolStripMenuItem("Slot " + i + " Shift F(" + k + ")");
        loadSlot->Tag = i; // Store slot number in Tag
        loadSlot->Click += gcnew EventHandler(this, &MainForm::LoadSlot_Click);
        loadMenu->DropDownItems->Add(loadSlot);
    }

    // Add Save and Load Menus to File Menu
    fileMenu->DropDownItems->Add(saveMenu);
    fileMenu->DropDownItems->Add(loadMenu);
    fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Quit", nullptr, gcnew EventHandler(this, &MainForm::Exit_Click)));

    // Generate Board Menu Items
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
   generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Save as spreadsheet puzzle1.xml F(9)", nullptr,
        gcnew EventHandler(this, &MainForm::GeneratePuzzle1)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Save as spreadsheet puzzle2.xml F(10)", nullptr,
        gcnew EventHandler(this, &MainForm::GeneratePuzzle2)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Save as spreadsheet puzzle3.xml F(11)", nullptr,
        gcnew EventHandler(this, &MainForm::GeneratePuzzle3)));
    generateBoardMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Save as spreadsheet puzzle4.xml F(12)", nullptr,
        gcnew EventHandler(this, &MainForm::GeneratePuzzle4)));

    // Help Menu
    ToolStripMenuItem^ helpMenu = gcnew ToolStripMenuItem("Help");
    helpMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "About", nullptr,
        gcnew EventHandler(this, &MainForm::About_Click)));
    helpMenu->DropDownItems->Add(gcnew ToolStripMenuItem(
        "Support the Author (Buy Me a Coffee)", nullptr,
        gcnew EventHandler(this, &MainForm::SupportAuthor_Click)));

    // Add Menus to MenuStrip
    menuStrip->Items->Add(fileMenu);
    menuStrip->Items->Add(generateBoardMenu);
    menuStrip->Items->Add(helpMenu);

    // Attach MenuStrip to the Form
    this->MainMenuStrip = menuStrip;
    this->Controls->Add(menuStrip);

    // Initialize ToolStrip
    toolStrip = gcnew ToolStrip();
    ToolStripButton^ newGameBtn = gcnew ToolStripButton(
        "New Game (Z)", nullptr,
        gcnew EventHandler(this, &MainForm::NewGame_Click));
    newGameBtn->AutoSize = false;
    newGameBtn->Size = System::Drawing::Size(100, 25);
    toolStrip->Items->Add(newGameBtn);
    
    // Add Toggle Notes button
    ToolStripButton^ toggleNotesBtn = gcnew ToolStripButton(
        "Toggle Notes (T)", nullptr,
        gcnew EventHandler(this, &MainForm::ToggleNotes_Click));
    toggleNotesBtn->AutoSize = false;
    toggleNotesBtn->Size = System::Drawing::Size(110, 25);
    toolStrip->Items->Add(toggleNotesBtn);
    
    toolStrip->Items->Add(gcnew ToolStripSeparator());
    
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Complete Auto-Solve (A)", nullptr,
        gcnew EventHandler(this, &MainForm::Solve_Click)));
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
        L"  - Press 'A' for auto-solve. Press F1-F4 for new puzzles (easy to expert). Press Shift+F1 for master.\r\n"
        L"  - Press F5-F8 to save, Shift+F5-F8 to load. F9-F12 to export as XML. Arrow keys navigate cells.";

    instructionsBox->Font = gcnew System::Drawing::Font(L"Lucida Console", 9); // Consistent fixed-width font
    this->Controls->Add(instructionsBox);


    // Basic techniques group
    toolStrip->Items->Add(gcnew ToolStripLabel("Basic Algorithms: "));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Standard Elim (S)", nullptr,
        gcnew EventHandler(this, &MainForm::StdElim_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Line Elim (L)", nullptr,
        gcnew EventHandler(this, &MainForm::LineElim_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Hidden Singles (N)", nullptr,
        gcnew EventHandler(this, &MainForm::HiddenSingles_Click)));
    toolStrip->Items->Add(gcnew ToolStripSeparator());

    // Advanced techniques group
    toolStrip->Items->Add(gcnew ToolStripLabel("Advanced Algorithms: "));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Hidden Pairs (H)", nullptr,
        gcnew EventHandler(this, &MainForm::HiddenPairs_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Pointing Pairs (P)", nullptr,
        gcnew EventHandler(this, &MainForm::PointingPairs_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Naked Sets (K)", nullptr,
        gcnew EventHandler(this, &MainForm::NakedSets_Click)));
    toolStrip->Items->Add(gcnew ToolStripSeparator());

    // Expert techniques group
    toolStrip->Items->Add(gcnew ToolStripLabel("Expert Algorithms: "));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "X-Wing (X)", nullptr,
        gcnew EventHandler(this, &MainForm::XWing_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Swordfish (F)", nullptr,
        gcnew EventHandler(this, &MainForm::Swordfish_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "XY-Wing (Y)", nullptr,
        gcnew EventHandler(this, &MainForm::XYWing_Click)));
    toolStrip->Items->Add(gcnew ToolStripButton(
        "XYZ-Wing (;)", nullptr,
        gcnew EventHandler(this, &MainForm::XYZWing_Click)));



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
        grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", 20);
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
          // Display immutable cells in blue
          grid[i, j]->BackColor = Color::LightBlue;
          grid[i, j]->ForeColor = Color::DarkBlue;
          grid[i, j]->ReadOnly = true;
          grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Bold);
        } else {
          // Regular cells are white
          grid[i, j]->BackColor = Color::White;
          grid[i, j]->ForeColor = Color::Black;
          grid[i, j]->ReadOnly = false;
          grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Regular);
        }
        
        ClearNotes(i, j);  // Clear notes when updating grid
      }
    }
    ValidateAndHighlight();
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

  void UpdateStatus(String ^ message) {
    statusLabel->Text = message;
    statusStrip->Refresh();
    debugBox->AppendText(message + "\r\n");
    debugBox->SelectionStart = debugBox->Text->Length;
    debugBox->ScrollToCaret();

    /*sudoku->print_debug(message);*/

  }

  void ValidateAndHighlight() {
    // Clear all previous highlights
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        grid[i, j]->BackColor = Color::White;
        grid[i, j]->ForeColor = Color::Black;
      }
    }

    // ONLY check for direct conflicts (duplicates) - no solver validation
    // This catches immediate rule violations
    for (int row = 0; row < 9; row++) {
      for (int col = 0; col < 9; col++) {
        int value = sudoku->GetValue(row, col);
        if (value == -1) continue;  // Skip empty cells

        bool hasConflict = false;

        // Check row for duplicates
        for (int c = 0; c < 9; c++) {
          if (c != col && sudoku->GetValue(row, c) == value) {
            hasConflict = true;
            break;
          }
        }

        // Check column for duplicates
        if (!hasConflict) {
          for (int r = 0; r < 9; r++) {
            if (r != row && sudoku->GetValue(r, col) == value) {
              hasConflict = true;
              break;
            }
          }
        }

        // Check 3x3 box for duplicates
        if (!hasConflict) {
          int boxRow = (row / 3) * 3;
          int boxCol = (col / 3) * 3;
          for (int r = boxRow; r < boxRow + 3; r++) {
            for (int c = boxCol; c < boxCol + 3; c++) {
              if ((r != row || c != col) && sudoku->GetValue(r, c) == value) {
                hasConflict = true;
                break;
              }
            }
            if (hasConflict) break;
          }
        }

        // If direct conflict found, highlight in red
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

    // Check if this cell is immutable (from generated puzzle)
    if (sudoku->IsCellImmutable(col, row)) {
      // Only allow clearing immutable cells if we're trying to delete
      if (textBox->Text->Length == 0) {
        // Allow clearing
        return;
      } else {
        // Try to parse the input
        int value;
        if (Int32::TryParse(textBox->Text, value) && value >= 1 && value <= 9) {
          // Check if this matches the puzzle's original value
          int originalValue = sudoku->GetValue(col, row);
          
          // If user entered the correct value, allow it
          if (value - 1 == originalValue) {
            // Correct! Keep the value
            sudoku->SetValue(col, row, value - 1);
            // Make textbox blue to indicate immutable
            textBox->BackColor = Color::LightBlue;
            textBox->ForeColor = Color::DarkBlue;
            textBox->ReadOnly = true;  // Prevent further editing
            ValidateAndHighlight();
            return;
          } else {
            // Wrong! Reject the input
            textBox->Text = System::Convert::ToString(originalValue + 1);
            textBox->BackColor = Color::LightBlue;
            textBox->ForeColor = Color::DarkBlue;
            textBox->ReadOnly = true;
            UpdateStatus("That cell is from the puzzle and cannot be changed");
            ValidateAndHighlight();
            return;
          }
        } else {
          // Invalid input for immutable cell
          int originalValue = sudoku->GetValue(col, row);
          textBox->Text = System::Convert::ToString(originalValue + 1);
          textBox->BackColor = Color::LightBlue;
          textBox->ForeColor = Color::DarkBlue;
          textBox->ReadOnly = true;
          ValidateAndHighlight();
          return;
        }
      }
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
    // Suppress the beep for all keys - let KeyDown handle everything
    e->Handled = true;
  }

  void Cell_KeyDown(Object ^ sender, KeyEventArgs ^ e) {
    TextBox ^ currentCell = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(currentCell->Tag);
    int row = position[0];
    int col = position[1];

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
        // Normal input mode - set the cell value
        sudoku->Clean();
        sudoku->SetValue(row, col, ((int)e->KeyCode - (int)Keys::D1));
        UpdateGrid();
        e->Handled = true;
        break;

      // Clear cell with 0
      case Keys::D0:
        sudoku->ClearValue(row, col);
        UpdateGrid();
        e->Handled = true;
        break;

      // Solving techniques
      case Keys::S:
        if (sudoku->IsValidSolution()) {
          sudoku->StdElim();
          ClearDebugBox();
          UpdateGrid();
          UpdateStatus("Standard elimination completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::L:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->LinElim();
          UpdateGrid();
          UpdateStatus("Line elimination completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::H:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindHiddenPairs();
          UpdateGrid();
          UpdateStatus("Hidden pairs completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::P:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindPointingPairs();
          UpdateGrid();
          UpdateStatus("Pointing pairs completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::N:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindHiddenSingles();
          UpdateGrid();
          UpdateStatus("Hidden singles completed");
          e->Handled = true;
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        break;
      case Keys::K:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindNakedSets();
          UpdateGrid();
          UpdateStatus("Naked sets completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::X:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindXWing();
          UpdateGrid();
          UpdateStatus("X-Wing technique completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::F:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindSwordFish();
          UpdateGrid();
          UpdateStatus("Swordfish technique completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::Y:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->FindXYWing();
          UpdateGrid();
          UpdateStatus("XY-Wing technique completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
        e->Handled = true;
        break;
      case Keys::OemSemicolon:  // For XYZ-Wing (;)
        if (!e->Shift) {
          if (sudoku->IsValidSolution()) {
            ClearDebugBox();
            sudoku->FindXYZWing();
            UpdateGrid();
            UpdateStatus("XYZ-Wing technique completed");
          } else {
            UpdateStatus("Current Board is Invalid");
          }
          e->Handled = true;
        }
        break;
      case Keys::A:
        if (sudoku->IsValidSolution()) {
          ClearDebugBox();
          sudoku->Solve();
          UpdateGrid();
          UpdateStatus("Full solve completed");
        } else {
          UpdateStatus("Current Board is Invalid");
        }
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
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_1.txt");
          } else {
            UpdateStatus("Failed to load sudoku_1.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_1.txt");
          UpdateStatus("Game saved to sudoku_1.txt");
        }
        e->Handled = true;
        break;
      case Keys::F6:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_2.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_2.txt");
          } else {
            UpdateStatus("Failed to load sudoku_2.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_2.txt");
          UpdateStatus("Game saved to sudoku_2.txt");
        }
        e->Handled = true;
        break;
      case Keys::F7:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_3.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_3.txt");
          } else {
            UpdateStatus("Failed to load sudoku_3.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_3.txt");
          UpdateStatus("Game saved to sudoku_3.txt");
        }
        e->Handled = true;
        break;
      case Keys::F8:
        if (e->Shift) {
          if (sudoku->LoadFromFile("sudoku_4.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_4.txt");
          } else {
            UpdateStatus("Failed to load sudoku_4.txt");
          }
        } else {
          sudoku->SaveToFile("sudoku_4.txt");
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
        if (!e->Shift) {
          sudoku->ExportToExcelXML("puzzle3.xml");
        }
        e->Handled = true;
        break;
      case Keys::F12:
        if (!e->Shift) {
          sudoku->ExportToExcelXML("puzzle4.xml");
        }
        e->Handled = true;
        break;


    }
  }

  // Menu event handlers
  void NewGame_Click(Object ^ sender, EventArgs ^ e) {
    sudoku->NewGame();
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
    sudoku->SaveToFile("sudoku_1.txt");
    UpdateStatus("Game saved successfully (sudoku_1.txt)");
  }

  void Exit_Click(Object ^ sender, EventArgs ^ e) {
     Application::Exit(); 
  }

  void About_Click(Object ^ sender, EventArgs ^ e) {
    String^ aboutText = 
      "Sudoku Solver\r\n\r\n" +
      "(C) 2025 Jason Brian Hall\r\n" +
      "MIT License - https://opensource.org/licenses/MIT\r\n\r\n" +
      "GitHub: https://github.com/jasonbrianhall/sudoku_solver\r\n\r\n" +
      "A powerful Sudoku puzzle generator and solver with support for multiple difficulty levels " +
      "and advanced solving techniques including X-Wing, Swordfish, XY-Wing, and XYZ-Wing patterns.\r\n\r\n" +
      "Features:\r\n" +
      "- Generate puzzles at 6 difficulty levels\r\n" +
      "- Real-time conflict detection and highlighting\r\n" +
      "- Pencil mark notes for candidates\r\n" +
      "- Multiple solving techniques\r\n" +
      "- Save/load games\r\n" +
      "- Export to Excel XML\r\n";

    MessageBox::Show(this, aboutText, "About Sudoku Solver", MessageBoxButtons::OK, MessageBoxIcon::Information);
  }

  void SupportAuthor_Click(Object ^ sender, EventArgs ^ e) {
    String^ supportText = 
      "If you enjoy this Sudoku Solver, please consider supporting the author!\r\n\r\n" +
      "Visit: https://buymeacoffee.com/jasonbrianhall\r\n\r\n" +
      "Your support helps fund development and keeps this project active.";

    MessageBox::Show(this, supportText, "Support the Author", MessageBoxButtons::OK, MessageBoxIcon::Information);
  }

  // Solving technique handlers
  void Solve_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->Solve();
      UpdateGrid();
      UpdateStatus("Full solve completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void StdElim_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->StdElim();
      UpdateGrid();
      UpdateStatus("Standard elimination completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void LineElim_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->LinElim();
      UpdateGrid();
      UpdateStatus("Line elimination completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void HiddenSingles_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindHiddenSingles();
      UpdateGrid();
      UpdateStatus("Hidden singles technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void HiddenPairs_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindHiddenPairs();
      UpdateGrid();
      UpdateStatus("Hidden pairs technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void PointingPairs_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindPointingPairs();
      UpdateGrid();
      UpdateStatus("Pointing pairs technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void NakedSets_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindNakedSets();
      UpdateGrid();
      UpdateStatus("Naked sets technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void XWing_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindXWing();
      UpdateGrid();
      UpdateStatus("X-Wing technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void Swordfish_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindSwordFish();
      UpdateGrid();
      UpdateStatus("Swordfish technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void XYWing_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindXYWing();
      UpdateGrid();
      UpdateStatus("XY-Wing technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void XYZWing_Click(Object ^ sender, EventArgs ^ e) {
    if (sudoku->IsValidSolution()) {
      sudoku->FindXYZWing();
      UpdateGrid();
      UpdateStatus("XYZ-Wing technique completed");
    } else {
      UpdateStatus("Current board is invalid");
    }
  }

  void SaveSlot_Click(Object^ sender, EventArgs^ e) {
    ToolStripMenuItem^ menuItem = safe_cast<ToolStripMenuItem^>(sender);
    int slot = safe_cast<int>(menuItem->Tag); // Retrieve slot number
    String^ filename = "sudoku_slot_" + slot + ".txt";

    sudoku->SaveToFile(filename);
    UpdateStatus("Game saved to " + filename);
  }

  void LoadSlot_Click(Object^ sender, EventArgs^ e) {
    ToolStripMenuItem^ menuItem = safe_cast<ToolStripMenuItem^>(sender);
    int slot = safe_cast<int>(menuItem->Tag); // Retrieve slot number
    String^ filename = "sudoku_slot_" + slot + ".txt";

    if (sudoku->LoadFromFile(filename)) {
        UpdateGrid();
        UpdateStatus("Game loaded from " + filename);
    } else {
        UpdateStatus("Failed to load " + filename);
    }
  }



 public:
  MainForm() {
    sudoku = gcnew SudokuWrapper();
    InitializeComponent();

    // Set the form icon
    try {
      System::Drawing::Icon ^ icon = gcnew System::Drawing::Icon(
          System::Reflection::Assembly::GetExecutingAssembly()
              ->GetManifestResourceStream("app.ico"));
      this->Icon = icon;
    } catch (Exception ^ ex) {
      // Icon loading failed, will use default icon
      System::Diagnostics::Debug::WriteLine("Failed to load icon: " +
                                            ex->Message);
    }
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
