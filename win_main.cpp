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
        
        // File operations
        bool LoadFromFile(String^ filename) {
            return nativeSudoku->LoadFromFile(msclr::interop::marshal_as<std::string>(filename));
        }
        void SaveToFile(String^ filename) {
            nativeSudoku->SaveToFile(msclr::interop::marshal_as<std::string>(filename));
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
            toolStrip->Items->Add(gcnew ToolStripButton("Solve All", nullptr,
                gcnew EventHandler(this, &MainForm::Solve_Click)));
            toolStrip->Items->Add(gcnew ToolStripSeparator());

            // Basic techniques group
            toolStrip->Items->Add(gcnew ToolStripLabel("Basic: "));
            toolStrip->Items->Add(gcnew ToolStripButton("Standard Elim", nullptr,
                gcnew EventHandler(this, &MainForm::StdElim_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Line Elim", nullptr,
                gcnew EventHandler(this, &MainForm::LineElim_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Hidden Singles", nullptr,
                gcnew EventHandler(this, &MainForm::HiddenSingles_Click)));
            toolStrip->Items->Add(gcnew ToolStripSeparator());

            // Advanced techniques group
            toolStrip->Items->Add(gcnew ToolStripLabel("Advanced: "));
            toolStrip->Items->Add(gcnew ToolStripButton("Hidden Pairs", nullptr,
                gcnew EventHandler(this, &MainForm::HiddenPairs_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Pointing Pairs", nullptr,
                gcnew EventHandler(this, &MainForm::PointingPairs_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Naked Sets", nullptr,
                gcnew EventHandler(this, &MainForm::NakedSets_Click)));
            toolStrip->Items->Add(gcnew ToolStripSeparator());

            // Expert techniques group
            toolStrip->Items->Add(gcnew ToolStripLabel("Expert: "));
            toolStrip->Items->Add(gcnew ToolStripButton("X-Wing", nullptr,
                gcnew EventHandler(this, &MainForm::XWing_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Swordfish", nullptr,
                gcnew EventHandler(this, &MainForm::Swordfish_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("XY-Wing", nullptr,
                gcnew EventHandler(this, &MainForm::XYWing_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("XYZ-Wing", nullptr,
                gcnew EventHandler(this, &MainForm::XYZWing_Click)));
            toolStrip->Items->Add(gcnew ToolStripButton("Simple Coloring", nullptr,
                gcnew EventHandler(this, &MainForm::SimpleColoring_Click)));

            this->Controls->Add(toolStrip);

            // Initialize grid
            grid = gcnew array<TextBox^, 2>(9, 9);
            int gridTop = menuStrip->Height + toolStrip->Height + 20;
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    grid[i, j] = gcnew TextBox();
                    grid[i, j]->Size = System::Drawing::Size(40, 40);
                    grid[i, j]->Location = System::Drawing::Point(50 + j * 45, gridTop + i * 45);
                    grid[i, j]->MaxLength = 1;
                    grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", 20);
                    grid[i, j]->TextAlign = HorizontalAlignment::Center;
                    grid[i, j]->Tag = gcnew array<int> { i, j };
                    grid[i, j]->TextChanged += gcnew EventHandler(this, &MainForm::Cell_TextChanged);
                    this->Controls->Add(grid[i, j]);
                }
            }

            // Draw grid lines
            for (int i = 0; i <= 9; i++) {
                Panel^ vline = gcnew Panel();
                vline->BorderStyle = BorderStyle::FixedSingle;
                vline->Location = Point(48 + i * 45, gridTop - 2);
                vline->Size = System::Drawing::Size(2, 410);
                vline->BackColor = (i % 3 == 0) ? Color::Black : Color::Gray;
                this->Controls->Add(vline);

                Panel^ hline = gcnew Panel();
                hline->BorderStyle = BorderStyle::FixedSingle;
                hline->Location = Point(48, gridTop - 2 + i * 45);
                hline->Size = System::Drawing::Size(410, 2);
                hline->BackColor = (i % 3 == 0) ? Color::Black : Color::Gray;
                this->Controls->Add(hline);
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
                    sudoku->SetValue(position[0], position[1], value - 1);
                } else {
                    textBox->Text = "";
                }
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
                if (sudoku->LoadFromFile(openFileDialog->FileName)) {
                    UpdateGrid();
                    UpdateStatus("Game loaded successfully");
                } else {
                    UpdateStatus("Failed to load game");
                }
            }
        }

        void Save_Click(Object^ sender, EventArgs^ e) {
            SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
            saveFileDialog->Filter = "Sudoku files (*.txt)|*.txt|All files (*.*)|*.*";
            
            if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                sudoku->SaveToFile(saveFileDialog->FileName);
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

        void SimpleColoring_Click(Object^ sender, EventArgs^ e) {
            sudoku->FindSimpleColoring();
            UpdateGrid();
            UpdateStatus("Simple coloring technique completed");
        }

    public:
        MainForm() {
            sudoku = gcnew SudokuWrapper();
            InitializeComponent();
        }
    };
}

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    Application::Run(gcnew SudokuGame::MainForm());
    return 0;
}
