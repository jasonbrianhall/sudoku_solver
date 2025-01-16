#define _HAS_STD_BYTE 0
#define NOMINMAX
#include "sudoku.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace SudokuGame {
    public ref class SudokuWrapper {
    private:
        Sudoku* nativeSudoku;

    public:
        SudokuWrapper() {
            nativeSudoku = new Sudoku();
        }

        ~SudokuWrapper() {
            if (nativeSudoku) {
                delete nativeSudoku;
                nativeSudoku = nullptr;
            }
        }

        // Core game functions
        void SetValue(int x, int y, int value) { nativeSudoku->SetValue(x, y, value); }
        int GetValue(int x, int y) { return nativeSudoku->GetValue(x, y); }
        void NewGame() { nativeSudoku->NewGame(); }
        void Solve() { nativeSudoku->Solve(); }
        void ClearValue(int x, int y) { nativeSudoku->ClearValue(x, y); }
        void Clean() { nativeSudoku->Clean(); }
        
        // File operations
	bool LoadFromFile(String^ filename) {
	    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
	    return nativeSudoku->LoadFromFile(std::string(wstr.begin(), wstr.end()));
	}

	void SaveToFile(String^ filename) {
	    std::wstring wstr = msclr::interop::marshal_as<std::wstring>(filename);
	    nativeSudoku->SaveToFile(std::string(wstr.begin(), wstr.end()));
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
    };

    public ref class MainForm : public System::Windows::Forms::Form {
    private:
        SudokuWrapper^ sudoku;
        array<TextBox^, 2>^ grid;
        MenuStrip^ menuStrip;
        ToolStrip^ toolStrip;
        StatusStrip^ statusStrip;
        ToolStripStatusLabel^ statusLabel;

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
            fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem("New Game", nullptr, 
                gcnew EventHandler(this, &MainForm::NewGame_Click)));
            fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem("Load", nullptr,
                gcnew EventHandler(this, &MainForm::Load_Click)));
            fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem("Save", nullptr,
                gcnew EventHandler(this, &MainForm::Save_Click)));
            fileMenu->DropDownItems->Add(gcnew ToolStripSeparator());
            fileMenu->DropDownItems->Add(gcnew ToolStripMenuItem("Exit", nullptr,
                gcnew EventHandler(this, &MainForm::Exit_Click)));
            menuStrip->Items->Add(fileMenu);
            this->Controls->Add(menuStrip);

            // Initialize ToolStrip
            toolStrip = gcnew ToolStrip();
            toolStrip->Items->Add(gcnew ToolStripButton("Solve All (A)", nullptr,
                gcnew EventHandler(this, &MainForm::Solve_Click)));
            toolStrip->Items->Add(gcnew ToolStripSeparator());

            // Basic techniques group
            toolStrip->Items->Add(gcnew ToolStripLabel("Basic: "));
            toolStrip->Items->Add(gcnew ToolStripButton("Standard Elim (S)", nullptr,
                gcnew EventHandler(this, &MainForm::StdElim_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Line Elim (L)", nullptr,
                gcnew EventHandler(this, &MainForm::LineElim_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Hidden Singles (N)", nullptr,
                gcnew EventHandler(this, &MainForm::HiddenSingles_Click)));
            toolStrip->Items->Add(gcnew ToolStripSeparator());

            // Advanced techniques group
            toolStrip->Items->Add(gcnew ToolStripLabel("Advanced: "));
            toolStrip->Items->Add(gcnew ToolStripButton("Hidden Pairs (H)", nullptr,
                gcnew EventHandler(this, &MainForm::HiddenPairs_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Pointing Pairs (P)", nullptr,
                gcnew EventHandler(this, &MainForm::PointingPairs_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Naked Sets (K)", nullptr,
                gcnew EventHandler(this, &MainForm::NakedSets_Click)));
            toolStrip->Items->Add(gcnew ToolStripSeparator());

            // Expert techniques group
            toolStrip->Items->Add(gcnew ToolStripLabel("Expert: "));
            toolStrip->Items->Add(gcnew ToolStripButton("X-Wing (X)", nullptr,
                gcnew EventHandler(this, &MainForm::XWing_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Swordfish (F)", nullptr,
                gcnew EventHandler(this, &MainForm::Swordfish_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("XY-Wing (Y)", nullptr,
                gcnew EventHandler(this, &MainForm::XYWing_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("XYZ-Wing (;)", nullptr,
                gcnew EventHandler(this, &MainForm::XYZWing_Click)));

            this->Controls->Add(toolStrip);

            // Initialize grid
            grid = gcnew array<TextBox^, 2>(9, 9);
            int gridTop = menuStrip->Height + toolStrip->Height + 20;

            // Create a container panel for the Sudoku grid
            Panel^ gridContainer = gcnew Panel();
            gridContainer->Location = Point(50, gridTop);
            gridContainer->Size = System::Drawing::Size(405, 405);
            gridContainer->BackColor = Color::Black;
            this->Controls->Add(gridContainer);

            // Initialize grid cells
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    grid[i, j] = gcnew TextBox();
                    grid[i, j]->Size = System::Drawing::Size(40, 40);
                    grid[i, j]->Location = System::Drawing::Point(3 + j * 45, 3 + i * 45);
                    grid[i, j]->MaxLength = 1;
                    grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", 20);
                    grid[i, j]->TextAlign = HorizontalAlignment::Center;
                    grid[i, j]->Tag = gcnew array<int> { i, j };
                    grid[i, j]->TextChanged += gcnew EventHandler(this, &MainForm::Cell_TextChanged);
                    grid[i, j]->KeyDown += gcnew KeyEventHandler(this, &MainForm::Cell_KeyDown);
                    grid[i, j]->BackColor = Color::White;
                    gridContainer->Controls->Add(grid[i, j]);
                }
            }

            // Draw grid lines
            for (int i = 0; i <= 9; i++) {
                Panel^ vline = gcnew Panel();
                vline->BorderStyle = BorderStyle::None;
                vline->Location = Point(i * 45, 0);
                vline->Size = System::Drawing::Size((i % 3 == 0) ? 3 : 1, gridContainer->Height);
                vline->BackColor = (i % 3 == 0) ? Color::Black : Color::LightGray;
                gridContainer->Controls->Add(vline);

                Panel^ hline = gcnew Panel();
                hline->BorderStyle = BorderStyle::None;
                hline->Location = Point(0, i * 45);
                hline->Size = System::Drawing::Size(gridContainer->Width, (i % 3 == 0) ? 3 : 1);
                hline->BackColor = (i % 3 == 0) ? Color::Black : Color::LightGray;
                gridContainer->Controls->Add(hline);
            }
        }

        void UpdateGrid() {
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    int value = sudoku->GetValue(i, j);
                    grid[i, j]->Text = (value >= 0) ? (value + 1).ToString() : "";
                }
            }
        }

        void UpdateStatus(String^ message) {
            statusLabel->Text = message;
            statusStrip->Refresh();
        }

        void Cell_TextChanged(Object^ sender, EventArgs^ e) {
            TextBox^ textBox = safe_cast<TextBox^>(sender);
            array<int>^ position = safe_cast<array<int>^>(textBox->Tag);
            
            if (textBox->Text->Length > 0) {
                int value;
                if (Int32::TryParse(textBox->Text, value) && value >= 1 && value <= 9) {
                    sudoku->Clean();
                    sudoku->SetValue(position[0], position[1], value - 1);
                } else {
                    textBox->Text = "";
                }
            }
        }

        void Cell_KeyDown(Object^ sender, KeyEventArgs^ e) {
            TextBox^ currentCell = safe_cast<TextBox^>(sender);
            array<int>^ position = safe_cast<array<int>^>(currentCell->Tag);
            int row = position[0];
            int col = position[1];

            switch (e->KeyCode) {
                case Keys::Left:
                    if (col > 0) {
                        grid[row, col - 1]->Focus();
                        e->Handled = true;
                    }
                    else {
                        grid[row, 8]->Focus();
                        e->Handled = true;
                    }                                        
                    break;
                case Keys::Right:
                    if (col < 8) {
                        grid[row, col + 1]->Focus();
                        e->Handled = true;
                    }
                    else {
                        grid[row, 0]->Focus();
                        e->Handled = true;
                    }                    
                    break;
                case Keys::Up:
                    if (row > 0) {
                        grid[row - 1, col]->Focus();
                        e->Handled = true;
                    }
                    else {
                        grid[8, col]->Focus();
                        e->Handled = true;
                    }                    
                    
                    break;
                case Keys::Down:
                    if (row < 8) {
                        grid[row + 1, col]->Focus();
                        e->Handled = true;
                    }
                    else {
                        grid[0, col]->Focus();
                        e->Handled = true;
                    }                    
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
                    sudoku->StdElim();
                    UpdateGrid();
                    UpdateStatus("Standard elimination completed");
                    e->Handled = true;
                    break;
                case Keys::L:
                    sudoku->LinElim();
                    UpdateGrid();
                    UpdateStatus("Line elimination completed");
                    e->Handled = true;
                    break;
                case Keys::H:
                    sudoku->FindHiddenPairs();
                    UpdateGrid();
                    UpdateStatus("Hidden pairs completed");
                    e->Handled = true;
                    break;
                case Keys::P:
                    sudoku->FindPointingPairs();
                    UpdateGrid();
                    UpdateStatus("Pointing pairs completed");
                    e->Handled = true;
                    break;
                case Keys::N:
                    sudoku->FindHiddenSingles();
                    UpdateGrid();
                    UpdateStatus("Hidden singles completed");
                    e->Handled = true;
                    break;
                case Keys::K:
                    sudoku->FindNakedSets();
                    UpdateGrid();
                    UpdateStatus("Naked sets completed");
                    e->Handled = true;
                    break;
                case Keys::X:
                    sudoku->FindXWing();
                    UpdateGrid();
                    UpdateStatus("X-Wing technique completed");
                    e->Handled = true;
                    break;
                case Keys::F:
                    sudoku->FindSwordFish();
                    UpdateGrid();
                    UpdateStatus("Swordfish technique completed");
                    e->Handled = true;
                    break;
                case Keys::Y:
                    sudoku->FindXYWing();
                    UpdateGrid();
                    UpdateStatus("XY-Wing technique completed");
                    e->Handled = true;
                    break;
                case Keys::OemSemicolon:  // For XYZ-Wing (;)
                    if (!e->Shift) {
                        sudoku->FindXYZWing();
                        UpdateGrid();
                        UpdateStatus("XYZ-Wing technique completed");
                        e->Handled = true;
                    }
                    break;
                case Keys::A:
                    sudoku->Solve();
                    UpdateGrid();
                    UpdateStatus("Full solve completed");
                    e->Handled = true;
                    break;
                case Keys::Z:
                    sudoku->NewGame();
                    UpdateGrid();
                    UpdateStatus("New game started");
                    e->Handled = true;
                    break;
                case Keys::F5:
                    if (e->Shift) {
                        if (sudoku->LoadFromFile("sudoku_1.txt")) {
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
            }
        }

        // Menu event handlers
        void NewGame_Click(Object^ sender, EventArgs^ e) {
            sudoku->NewGame();
            UpdateGrid();
            UpdateStatus("New game started");
        }

	void Load_Click(Object^ sender, EventArgs^ e) {
	    OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
	    openFileDialog->Filter = "Sudoku files (*.txt)|*.txt|All files (*.*)|*.*";
	    
	    if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		sudoku->LoadFromFile(openFileDialog->FileName);  // Pass String^ directly
		UpdateStatus("Game loaded successfully");
		UpdateGrid();
	    }
	}

	void Save_Click(Object^ sender, EventArgs^ e) {
	    SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
	    saveFileDialog->Filter = "Sudoku files (*.txt)|*.txt|All files (*.*)|*.*";
	    
	    if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		sudoku->SaveToFile(saveFileDialog->FileName);  // Pass String^ directly
		UpdateStatus("Game saved successfully");
	    }
	}

        void Exit_Click(Object^ sender, EventArgs^ e) {
            Application::Exit();
        }

        // Solving technique handlers
        void Solve_Click(Object^ sender, EventArgs^ e) {
            sudoku->Solve();
            UpdateGrid();
            UpdateStatus("Full solve completed");
        }

        void StdElim_Click(Object^ sender, EventArgs^ e) {
            sudoku->StdElim();
            UpdateGrid();
            UpdateStatus("Standard elimination completed");
        }

        void LineElim_Click(Object^ sender, EventArgs^ e) {
            sudoku->LinElim();
            UpdateGrid();
            UpdateStatus("Line elimination completed");
        }

        void HiddenSingles_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindHiddenSingles();
            UpdateGrid();
            UpdateStatus("Hidden singles technique completed");
        }

        void HiddenPairs_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindHiddenPairs();
            UpdateGrid();
            UpdateStatus("Hidden pairs technique completed");
        }

        void PointingPairs_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindPointingPairs();
            UpdateGrid();
            UpdateStatus("Pointing pairs technique completed");
        }

        void NakedSets_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindNakedSets();
            UpdateGrid();
            UpdateStatus("Naked sets technique completed");
        }

        void XWing_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindXWing();
            UpdateGrid();
            UpdateStatus("X-Wing technique completed");
        }

        void Swordfish_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindSwordFish();
            UpdateGrid();
            UpdateStatus("Swordfish technique completed");
        }

        void XYWing_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindXYWing();
            UpdateGrid();
            UpdateStatus("XY-Wing technique completed");
        }

        void XYZWing_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindXYZWing();
            UpdateGrid();
            UpdateStatus("XYZ-Wing technique completed");
        }

    public:
        MainForm() {
            sudoku = gcnew SudokuWrapper();
            InitializeComponent();
        }
    };
}

int main(array<String^>^ args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew SudokuGame::MainForm());
    return 0;
}
