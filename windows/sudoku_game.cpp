// Sudoku Game - Windows Forms C++/CLI Interface
// Similar to the Python Qt5 version but using .NET Windows Forms

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

// Custom button for sudoku cells
public ref class SudokuButton : public Button {
public:
    int cellX, cellY;
    Nullable<int> cellValue;
    bool isImmutable;
    
    SudokuButton(int x, int y) : cellX(x), cellY(y), cellValue(Nullable<int>()), isImmutable(false) {
        this->AutoSize = false;
        this->MinimumSize = Drawing::Size(50, 50);
        this->Size = Drawing::Size(60, 60);
        this->Font = gcnew Drawing::Font("Arial", 14);
        this->TextAlign = ContentAlignment::MiddleCenter;
        this->FlatStyle = Windows::Forms::FlatStyle::Flat;
        this->BackColor = Drawing::Color::White;
        this->FlatAppearance->BorderColor = Drawing::Color::Black;
        this->FlatAppearance->BorderSize = 1;
    }
    
    void SetValue(Nullable<int> value) {
        cellValue = value;
        if (value.HasValue && value.Value >= 1 && value.Value <= 9) {
            this->Text = value.Value.ToString();  // Display 1-9 directly
            this->ForeColor = Drawing::Color::Black;  // Clue values in black
        } else if (value.HasValue && value.Value >= 0 && value.Value < 1) {
            this->Text = "";
            this->ForeColor = Drawing::Color::Black;
        } else {
            this->Text = "";
            this->ForeColor = Drawing::Color::Black;
        }
    }
    
    Nullable<int> GetValue() {
        return cellValue;
    }
};

// Main game window
public ref class SudokuGameWindow : public Form {
private:
    Sudoku* nativeSudoku;
    PuzzleGenerator* puzzleGenerator;
    array<array<SudokuButton^>^>^ buttons;
    Label^ statusLabel;
    Panel^ boardPanel;
    String^ currentDifficulty;
    array<array<bool>^>^ immutableCells;
    
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
        
        // Initialize immutable cells tracking
        immutableCells = gcnew array<array<bool>^>(9);
        for (int i = 0; i < 9; i++) {
            immutableCells[i] = gcnew array<bool>(9);
        }
        
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
        // Create outer border panel
        Panel^ borderPanel = gcnew Panel();
        borderPanel->Size = Drawing::Size(560, 560);
        borderPanel->Location = Drawing::Point(70, 50);
        borderPanel->BackColor = Drawing::Color::Black;
        borderPanel->BorderStyle = BorderStyle::Fixed3D;
        this->Controls->Add(borderPanel);
        
        boardPanel = gcnew Panel();
        boardPanel->Size = Drawing::Size(540, 540);
        boardPanel->Location = Drawing::Point(10, 10);
        boardPanel->BackColor = Drawing::Color::White;
        borderPanel->Controls->Add(boardPanel);
        
        buttons = gcnew array<array<SudokuButton^>^>(9);
        
        for (int y = 0; y < 9; y++) {
            buttons[y] = gcnew array<SudokuButton^>(9);
            for (int x = 0; x < 9; x++) {
                SudokuButton^ btn = gcnew SudokuButton(x, y);
                btn->Location = Drawing::Point(x * 60, y * 60);
                
                // Borders for 3x3 boxes
                int top = 1, left = 1, right = 1, bottom = 1;
                if (x % 3 == 0) left = 3;
                if ((x + 1) % 3 == 0) right = 3;
                if (y % 3 == 0) top = 3;
                if ((y + 1) % 3 == 0) bottom = 3;
                
                btn->Margin = Windows::Forms::Padding(left, top, right, bottom);
                
                btn->Click += gcnew EventHandler(this, &SudokuGameWindow::OnButtonClick);
                buttons[y][x] = btn;
                boardPanel->Controls->Add(btn);
            }
        }
        
        this->Controls->Add(boardPanel);
    }
    
    void SetupKeyBindings() {
        this->KeyPreview = true;
        this->KeyDown += gcnew KeyEventHandler(this, &SudokuGameWindow::OnKeyDown);
    }
    
    void OnButtonClick(Object^ sender, EventArgs^ e) {
        SudokuButton^ btn = safe_cast<SudokuButton^>(sender);
        if (btn->isImmutable) return;
        
        int value = btn->cellValue.HasValue ? btn->cellValue.Value : 0;
        
        // Check if right-click (clear)
        if (Control::ModifierKeys == Windows::Forms::Keys::Control) {
            nativeSudoku->ClearValue(btn->cellX, btn->cellY);
        } else {
            // Left-click: increment 1-9, wrapping to empty
            value = (value >= 8) ? -1 : value + 1;
            if (value == -1) {
                nativeSudoku->ClearValue(btn->cellX, btn->cellY);
            } else {
                nativeSudoku->SetValue(btn->cellX, btn->cellY, value + 1);
            }
        }
        
        UpdateDisplay();
    }
    
    void OnKeyDown(Object^ sender, KeyEventArgs^ e) {
        // Handle number keys 1-9 and Delete
        if (e->KeyCode >= Keys::D1 && e->KeyCode <= Keys::D9) {
            int num = (int)e->KeyCode - (int)Keys::D1 + 1;
            // Find focused button and set value
            for (int y = 0; y < 9; y++) {
                for (int x = 0; x < 9; x++) {
                    if (buttons[y][x]->Focused && !buttons[y][x]->isImmutable) {
                        nativeSudoku->SetValue(x, y, num);
                        UpdateDisplay();
                        e->Handled = true;
                        return;
                    }
                }
            }
        }
        
        if (e->KeyCode == Keys::Delete || e->KeyCode == Keys::Back) {
            for (int y = 0; y < 9; y++) {
                for (int x = 0; x < 9; x++) {
                    if (buttons[y][x]->Focused && !buttons[y][x]->isImmutable) {
                        nativeSudoku->ClearValue(x, y);
                        UpdateDisplay();
                        e->Handled = true;
                        return;
                    }
                }
            }
        }
        
        switch (e->KeyCode) {
            case Keys::F1:
                if (e->Shift) OnGenerateExtreme(nullptr, nullptr);
                else OnGenerateEasy(nullptr, nullptr);
                e->Handled = true;
                break;
            case Keys::F2:
                OnGenerateMedium(nullptr, nullptr);
                e->Handled = true;
                break;
            case Keys::F3:
                OnGenerateHard(nullptr, nullptr);
                e->Handled = true;
                break;
            case Keys::F4:
                OnGenerateExpert(nullptr, nullptr);
                e->Handled = true;
                break;
        }
    }
    
    void UpdateDisplay() {
        for (int y = 0; y < 9; y++) {
            for (int x = 0; x < 9; x++) {
                int value = nativeSudoku->GetValue(x, y);
                if (value >= 1 && value <= 9) {
                    buttons[y][x]->SetValue(Nullable<int>(value));  // Pass 1-9 directly
                } else {
                    buttons[y][x]->SetValue(Nullable<int>());
                }
            }
        }
        
        if (!nativeSudoku->IsValidSolution()) {
            statusLabel->Text = "Invalid solution! Please check your entries.";
            statusLabel->ForeColor = Color::Red;
        } else {
            statusLabel->Text = "Valid solution!";
            statusLabel->ForeColor = Color::Green;
        }
    }
    
    void GeneratePuzzle(String^ difficulty) {
        std::string diff = msclr::interop::marshal_as<std::string>(difficulty);
        
        if (puzzleGenerator->generatePuzzle(diff)) {
            nativeSudoku->Clean();
            currentDifficulty = difficulty;
            
            // Mark clue cells as immutable
            for (int y = 0; y < 9; y++) {
                for (int x = 0; x < 9; x++) {
                    int value = nativeSudoku->GetValue(x, y);
                    immutableCells[y][x] = (value >= 0 && value <= 8);
                    if (immutableCells[y][x]) {
                        buttons[y][x]->isImmutable = true;
                        buttons[y][x]->BackColor = Color::LightGray;
                        buttons[y][x]->Enabled = false;
                    } else {
                        buttons[y][x]->isImmutable = false;
                        buttons[y][x]->BackColor = Color::White;
                        buttons[y][x]->Enabled = true;
                    }
                }
            }
            
            UpdateDisplay();
            statusLabel->Text = String::Format("Generated {0} puzzle", difficulty);
        } else {
            MessageBox::Show("Failed to generate puzzle", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
    
    // Event handlers for menu items
    void OnNewGame(Object^ sender, EventArgs^ e) {
        nativeSudoku->NewGame();
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                immutableCells[i][j] = false;
                buttons[i][j]->isImmutable = false;
                buttons[i][j]->BackColor = Color::White;
                buttons[i][j]->Enabled = true;
            }
        }
        UpdateDisplay();
        statusLabel->Text = "New empty game started";
    }
    
    void OnLoadGame(Object^ sender, EventArgs^ e) {
        OpenFileDialog^ ofd = gcnew OpenFileDialog();
        ofd->Filter = "Sudoku Files (*.sud)|*.sud|All Files (*.*)|*.*";
        if (ofd->ShowDialog() == Windows::Forms::DialogResult::OK) {
            std::string filename = msclr::interop::marshal_as<std::string>(ofd->FileName);
            if (nativeSudoku->LoadFromFile(filename)) {
                UpdateDisplay();
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
            nativeSudoku->ExportToExcelXML(msclr::interop::marshal_as<std::string>(sfd->FileName));
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
        UpdateDisplay();
        statusLabel->Text = "Standard Elimination applied";
    }
    
    void OnLinElim(Object^ sender, EventArgs^ e) {
        nativeSudoku->LinElim();
        UpdateDisplay();
        statusLabel->Text = "Line Elimination applied";
    }
    
    void OnHiddenSingles(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindHiddenSingles();
        UpdateDisplay();
        statusLabel->Text = "Hidden Singles applied";
    }
    
    void OnHiddenPairs(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindHiddenPairs();
        UpdateDisplay();
        statusLabel->Text = "Hidden Pairs applied";
    }
    
    void OnPointingPairs(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindPointingPairs();
        UpdateDisplay();
        statusLabel->Text = "Pointing Pairs applied";
    }
    
    void OnXWing(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindXWing();
        UpdateDisplay();
        statusLabel->Text = "X-Wing applied";
    }
    
    void OnSwordfish(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindSwordFish();
        UpdateDisplay();
        statusLabel->Text = "Swordfish applied";
    }
    
    void OnXYWing(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindXYWing();
        UpdateDisplay();
        statusLabel->Text = "XY-Wing applied";
    }
    
    void OnXYZWing(Object^ sender, EventArgs^ e) {
        nativeSudoku->FindXYZWing();
        UpdateDisplay();
        statusLabel->Text = "XYZ-Wing applied";
    }
    
    void OnSolveAll(Object^ sender, EventArgs^ e) {
        int result = nativeSudoku->Solve();
        UpdateDisplay();
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
        MessageBox::Show("Sudoku Solver - Game Edition\n\nA modern sudoku game with advanced solving techniques.", 
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
