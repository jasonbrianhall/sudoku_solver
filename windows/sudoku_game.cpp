// Sudoku Game - Windows Forms C++/CLI Interface
// Using TextBox grid similar to win_main.cpp

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
using namespace System::Collections::Generic;

namespace SudokuGame {

// Main game window
public ref class SudokuGameWindow : public Form {
private:
    Sudoku* nativeSudoku;
    PuzzleGenerator* puzzleGenerator;
    array<TextBox^, 2>^ grid;
    Label^ statusLabel;
    String^ currentDifficulty;
    
    MenuStrip^ menuStrip;
    ToolStripMenuItem^ fileMenu;
    ToolStripMenuItem^ gameMenu;
    ToolStripMenuItem^ solveMenu;
    ToolStripMenuItem^ helpMenu;

public:
    SudokuGameWindow() {
        nativeSudoku = new Sudoku();
        puzzleGenerator = new PuzzleGenerator(*nativeSudoku);
        currentDifficulty = "medium";
        
        InitializeComponent();
        InitializeMenu();
        CreateGameBoard();
        SetupKeyBindings();
        OnGenerateEasy(nullptr, nullptr);  // Auto-load easy puzzle on startup
    }

private:
    void InitializeComponent() {
        this->Text = "Sudoku Solver - Game Edition";
        this->Size = Drawing::Size(700, 850);
        this->StartPosition = FormStartPosition::CenterScreen;
        this->Icon = nullptr;
        
        statusLabel = gcnew Label();
        statusLabel->Text = "Ready to play!";
        statusLabel->TextAlign = ContentAlignment::MiddleCenter;
        statusLabel->Height = 30;
        statusLabel->ForeColor = Color::Red;
        statusLabel->Font = gcnew Drawing::Font("Arial", 10, FontStyle::Bold);
        this->Controls->Add(statusLabel);
        statusLabel->Dock = DockStyle::Top;
    }
    
    void InitializeMenu() {
        menuStrip = gcnew MenuStrip();
        this->Controls->Add(menuStrip);
        this->MainMenuStrip = menuStrip;
        
        // File Menu
        fileMenu = gcnew ToolStripMenuItem("&File");
        fileMenu->DropDownItems->Add("&New Game", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnNewGame));
        fileMenu->DropDownItems->Add("&Load Game", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnLoadGame));
        fileMenu->DropDownItems->Add("&Save Game", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnSaveGame));
        fileMenu->DropDownItems->Add(gcnew ToolStripSeparator());
        fileMenu->DropDownItems->Add("&Export to Excel", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnExportExcel));
        fileMenu->DropDownItems->Add(gcnew ToolStripSeparator());
        fileMenu->DropDownItems->Add("E&xit", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnExit));
        menuStrip->Items->Add(fileMenu);
        
        // Game Menu
        gameMenu = gcnew ToolStripMenuItem("&Game");
        gameMenu->DropDownItems->Add("&Easy (F1)", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnGenerateEasy));
        gameMenu->DropDownItems->Add("&Medium (F2)", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnGenerateMedium));
        gameMenu->DropDownItems->Add("&Hard (F3)", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnGenerateHard));
        gameMenu->DropDownItems->Add("&Expert (F4)", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnGenerateExpert));
        gameMenu->DropDownItems->Add("E&xtreme (Shift+F1)", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnGenerateExtreme));
        menuStrip->Items->Add(gameMenu);
        
        // Solve Menu
        solveMenu = gcnew ToolStripMenuItem("&Solve");
        solveMenu->DropDownItems->Add("Standard &Elimination", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnStdElim));
        solveMenu->DropDownItems->Add("&Line Elimination", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnLinElim));
        solveMenu->DropDownItems->Add("&Hidden Singles", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnHiddenSingles));
        solveMenu->DropDownItems->Add("Hidden &Pairs", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnHiddenPairs));
        solveMenu->DropDownItems->Add("&Pointing Pairs", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnPointingPairs));
        solveMenu->DropDownItems->Add("&X-Wing", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnXWing));
        solveMenu->DropDownItems->Add("&Swordfish", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnSwordfish));
        solveMenu->DropDownItems->Add("&XY-Wing", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnXYWing));
        solveMenu->DropDownItems->Add("X&YZ-Wing", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnXYZWing));
        solveMenu->DropDownItems->Add(gcnew ToolStripSeparator());
        solveMenu->DropDownItems->Add("Solve &All", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnSolveAll));
        menuStrip->Items->Add(solveMenu);
        
        // Help Menu
        helpMenu = gcnew ToolStripMenuItem("&Help");
        helpMenu->DropDownItems->Add("&About", nullptr, gcnew EventHandler(this, &SudokuGameWindow::OnAbout));
        menuStrip->Items->Add(helpMenu);
    }
    
    void CreateGameBoard() {
        grid = gcnew array<TextBox^, 2>(9, 9);
        int cellSize = 40;
        int startX = 70;
        int startY = 50;
        
        // Create grid of TextBox controls
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                TextBox^ tb = gcnew TextBox();
                tb->Size = Drawing::Size(cellSize, cellSize);
                tb->Location = Drawing::Point(startX + j * cellSize, startY + i * cellSize);
                tb->TextAlign = HorizontalAlignment::Center;
                tb->Font = gcnew Drawing::Font("Arial", 14, FontStyle::Bold);
                tb->MaxLength = 1;
                tb->BorderStyle = BorderStyle::FixedSingle;
                tb->TextChanged += gcnew EventHandler(this, &SudokuGameWindow::OnCellTextChanged);
                
                grid[i, j] = tb;
                this->Controls->Add(tb);
            }
        }
    }
    
    void SetupKeyBindings() {
        this->KeyPreview = true;
        this->KeyDown += gcnew KeyEventHandler(this, &SudokuGameWindow::OnKeyDown);
    }
    
    void UpdateGrid() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                int value = nativeSudoku->GetValue(i, j);
                grid[i, j]->Text = (value >= 0 && value <= 8) ? (value + 1).ToString() : "";
                
                // Check if cell is a clue (immutable)
                // A clue is a cell that was filled during puzzle generation
                // Since we generate puzzles, we mark filled cells as immutable
                if (value >= 0 && value <= 8) {
                    grid[i, j]->BackColor = Color::LightBlue;
                    grid[i, j]->ForeColor = Color::DarkBlue;
                    grid[i, j]->ReadOnly = true;
                    grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Bold);
                } else {
                    grid[i, j]->BackColor = Color::White;
                    grid[i, j]->ForeColor = Color::Black;
                    grid[i, j]->ReadOnly = false;
                    grid[i, j]->Font = gcnew System::Drawing::Font(grid[i, j]->Font, System::Drawing::FontStyle::Regular);
                }
            }
        }
    }
    
    void GeneratePuzzle(String^ difficulty) {
        std::string diff = msclr::interop::marshal_as<std::string>(difficulty);
        
        if (puzzleGenerator->generatePuzzle(diff)) {
            nativeSudoku->Clean();
            currentDifficulty = difficulty;
            UpdateGrid();
            statusLabel->Text = String::Format("Generated {0} puzzle", difficulty);
        } else {
            MessageBox::Show("Failed to generate puzzle", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
    
    void OnCellTextChanged(Object^ sender, EventArgs^ e) {
        TextBox^ tb = safe_cast<TextBox^>(sender);
        
        if (tb->Text->Length > 0) {
            if (!Char::IsDigit(tb->Text[0]) || tb->Text[0] < '1' || tb->Text[0] > '9') {
                tb->Text = "";
                return;
            }
            
            // Find cell position
            int row = -1, col = -1;
            for (int i = 0; i < 9 && row == -1; i++) {
                for (int j = 0; j < 9; j++) {
                    if (grid[i, j] == tb) {
                        row = i;
                        col = j;
                        break;
                    }
                }
            }
            
            if (row >= 0 && col >= 0) {
                int val = Int32::Parse(tb->Text);
                nativeSudoku->SetValue(col, row, val);
                
                if (!nativeSudoku->IsValidSolution()) {
                    statusLabel->Text = "Invalid solution! Please check your entries.";
                    statusLabel->ForeColor = Color::Red;
                } else {
                    statusLabel->Text = "Valid solution!";
                    statusLabel->ForeColor = Color::Green;
                }
            }
        }
    }
    
    void OnKeyDown(Object^ sender, KeyEventArgs^ e) {
        if (e->KeyCode == Keys::F1) {
            OnGenerateEasy(nullptr, nullptr);
            e->Handled = true;
        } else if (e->KeyCode == Keys::F2) {
            OnGenerateMedium(nullptr, nullptr);
            e->Handled = true;
        } else if (e->KeyCode == Keys::F3) {
            OnGenerateHard(nullptr, nullptr);
            e->Handled = true;
        } else if (e->KeyCode == Keys::F4) {
            OnGenerateExpert(nullptr, nullptr);
            e->Handled = true;
        } else if (e->KeyCode == Keys::F1 && e->Shift) {
            OnGenerateExtreme(nullptr, nullptr);
            e->Handled = true;
        }
    }
    
    // Event handlers for menu items
    void OnNewGame(Object^ sender, EventArgs^ e) {
        nativeSudoku->NewGame();
        UpdateGrid();
        statusLabel->Text = "New empty game started";
    }
    
    void OnLoadGame(Object^ sender, EventArgs^ e) {
        OpenFileDialog^ ofd = gcnew OpenFileDialog();
        ofd->Filter = "Sudoku Files (*.sud)|*.sud|All Files (*.*)|*.*";
        if (ofd->ShowDialog() == Windows::Forms::DialogResult::OK) {
            std::string filename = msclr::interop::marshal_as<std::string>(ofd->FileName);
            if (nativeSudoku->LoadFromFile(filename)) {
                UpdateGrid();
                statusLabel->Text = "Game loaded successfully";
            } else {
                MessageBox::Show("Failed to load game", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
    }
    
    void OnSaveGame(Object^ sender, EventArgs^ e) {
        SaveFileDialog^ sfd = gcnew SaveFileDialog();
        sfd->Filter = "Sudoku Files (*.sud)|*.sud";
        sfd->DefaultExt = "sud";
        if (sfd->ShowDialog() == Windows::Forms::DialogResult::OK) {
            std::string filename = msclr::interop::marshal_as<std::string>(sfd->FileName);
            nativeSudoku->SaveToFile(filename);
            statusLabel->Text = "Game saved successfully";
        }
    }
    
    void OnExportExcel(Object^ sender, EventArgs^ e) {
        SaveFileDialog^ sfd = gcnew SaveFileDialog();
        sfd->Filter = "Excel XML Files (*.xml)|*.xml";
        sfd->DefaultExt = "xml";
        if (sfd->ShowDialog() == Windows::Forms::DialogResult::OK) {
            std::string filename = msclr::interop::marshal_as<std::string>(sfd->FileName);
            nativeSudoku->ExportToExcelXML(filename);
            statusLabel->Text = "Exported to Excel";
        }
    }
    
    void OnGenerateEasy(Object^ sender, EventArgs^ e) { GeneratePuzzle("easy"); }
    void OnGenerateMedium(Object^ sender, EventArgs^ e) { GeneratePuzzle("medium"); }
    void OnGenerateHard(Object^ sender, EventArgs^ e) { GeneratePuzzle("hard"); }
    void OnGenerateExpert(Object^ sender, EventArgs^ e) { GeneratePuzzle("expert"); }
    void OnGenerateExtreme(Object^ sender, EventArgs^ e) { GeneratePuzzle("extreme"); }
    
    void OnStdElim(Object^ sender, EventArgs^ e) {
        nativeSudoku->StdElim();
        UpdateGrid();
        statusLabel->Text = "Standard Elimination applied";
    }
    
    void OnLinElim(Object^ sender, EventArgs^ e) {
        nativeSudoku->LinElim();
        UpdateGrid();
        statusLabel->Text = "Line Elimination applied";
    }
    
    void OnHiddenSingles(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindHiddenSingles();
        UpdateGrid();
        statusLabel->Text = "Hidden Singles applied";
    }
    
    void OnHiddenPairs(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindHiddenPairs();
        UpdateGrid();
        statusLabel->Text = "Hidden Pairs applied";
    }
    
    void OnPointingPairs(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindPointingPairs();
        UpdateGrid();
        statusLabel->Text = "Pointing Pairs applied";
    }
    
    void OnXWing(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindXWing();
        UpdateGrid();
        statusLabel->Text = "X-Wing applied";
    }
    
    void OnSwordfish(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindSwordFish();
        UpdateGrid();
        statusLabel->Text = "Swordfish applied";
    }
    
    void OnXYWing(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindXYWing();
        UpdateGrid();
        statusLabel->Text = "XY-Wing applied";
    }
    
    void OnXYZWing(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindXYZWing();
        UpdateGrid();
        statusLabel->Text = "XYZ-Wing applied";
    }
    
    void OnSolveAll(Object^ sender, EventArgs^ e) {
        int result = nativeSudoku->Solve();
        UpdateGrid();
        if (result == 0) {
            statusLabel->Text = "Puzzle solved!";
            statusLabel->ForeColor = Color::Green;
        } else {
            statusLabel->Text = "Could not solve puzzle";
            statusLabel->ForeColor = Color::Red;
        }
    }
    
    void OnExit(Object^ sender, EventArgs^ e) {
        this->Close();
    }
    
    void OnAbout(Object^ sender, EventArgs^ e) {
        MessageBox::Show("Sudoku Solver - Game Edition\r\n\r\nA modern sudoku game with advanced solving techniques.\r\n\r\nBlue cells are immutable clues from puzzle generation.\r\nWhite cells can be edited.", 
                        "About", MessageBoxButtons::OK, MessageBoxIcon::Information);
    }
};

// Application entry point
[STAThread]
void Main() {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew SudokuGameWindow());
}

} // namespace SudokuGame
