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
public
ref class SudokuWrapper {
 private:
  Sudoku* nativeSudoku;

 public:
  SudokuWrapper() { nativeSudoku = new Sudoku(); }

  ~SudokuWrapper() {
    if (nativeSudoku) {
      delete nativeSudoku;
      nativeSudoku = nullptr;
    }
  }
  property Sudoku* NativeSudoku {
    Sudoku* get() { return nativeSudoku; }
  }
  // Core game functions
  void SetValue(int x, int y, int value) {
    nativeSudoku->SetValue(x, y, value);
  }

  int GetValue(int x, int y) { return nativeSudoku->GetValue(x, y); }
  void NewGame() { nativeSudoku->NewGame(); }
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
};

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

public
ref class MainForm : public System::Windows::Forms::Form {
 private:
  SudokuWrapper ^ sudoku;
  array<TextBox ^, 2> ^ grid;
  MenuStrip ^ menuStrip;
  ToolStrip ^ toolStrip;
  StatusStrip ^ statusStrip;
  ToolStripStatusLabel ^ statusLabel;
  TextBox^ instructionsBox;
  TextBox^ debugBox;
  Panel^ gridContainer;
  bool notesMode = false;

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
      UpdateGrid();
      UpdateStatus("Generated new easy puzzle");
    } else {
      UpdateStatus("Failed to generate easy puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
    sudoku->Clean();
  }

  void GenerateMedium_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("medium")) {
      UpdateGrid();
      UpdateStatus("Generated new medium puzzle");
    } else {
      UpdateStatus("Failed to generate medium puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
    sudoku->Clean();
  }

  void GenerateHard_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("hard")) {
      UpdateGrid();
      UpdateStatus("Generated new hard puzzle");
    } else {
      UpdateStatus("Failed to generate hard puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
    sudoku->Clean();
  }

  void GenerateExpert_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("expert")) {
      UpdateGrid();
      UpdateStatus("Generated new expert puzzle");
    } else {
      UpdateStatus("Failed to generate expert puzzle");
      sudoku->NewGame();
      UpdateGrid();
    }
    sudoku->Clean();
  }

  void GenerateMaster_Click(Object ^ sender, EventArgs ^ e) {
    PuzzleGenerator generator(*sudoku->NativeSudoku);
    if (generator.generatePuzzle("extreme")) {
      UpdateGrid();
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
    
    // Game section
    ToolStripButton^ newGameBtn = gcnew ToolStripButton(
        "New Game (Z)", nullptr,
        gcnew EventHandler(this, &MainForm::NewGame_Click));
    newGameBtn->AutoSize = false;
    newGameBtn->Size = System::Drawing::Size(100, 25);
    toolStrip->Items->Add(newGameBtn);
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
        L"  - Use the mouse cursor to move around the board (scroll button will change the value and middle mouse will clear)\r\n"
        L"  - Use the keypad to enter numbers (0 to clear the current cell).\r\n"
        L"  - Press 'A' to solve the puzzle.\r\n"
        L"  - Press F1-F4 or Shift+F1 to generate increasingly difficult random puzzles.\r\n"
        L"  - Press F5-F8 to save, and Shift+F5-F8 to load.\r\n"
        L"  - Press F9-F12 to save as XML Spreadsheet in the format puzzle1.xml, puzzle2.xml, etc.\r\n"
        L"  - Red highlighted cells indicate conflicts (duplicates in row, column, or box).";

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

    // Initialize grid
    grid = gcnew array<TextBox ^, 2>(9, 9);
    int gridTop = menuStrip->Height + toolStrip->Height + instructionsBox->Height + 25;

    // Create a container panel for the Sudoku grid with white background
    gridContainer = gcnew Panel();
    gridContainer->Location = Point(50, gridTop);
    gridContainer->Size = System::Drawing::Size(405, 405);
    gridContainer->BackColor = Color::White;
    this->Controls->Add(gridContainer);

    // Draw bold grid lines for 3x3 boxes FIRST (underneath cells)
    for (int i = 0; i <= 9; i++) {
      int thickness = (i % 3 == 0) ? 3 : 1;
      int offset = (i % 3 == 0) ? 1 : 0;
      
      Panel ^ vline = gcnew Panel();
      vline->BorderStyle = BorderStyle::None;
      vline->Location = Point(i * 45 - offset, 0);
      vline->Size = System::Drawing::Size(thickness, 405);
      vline->BackColor = Color::Black;
      gridContainer->Controls->Add(vline);

      Panel ^ hline = gcnew Panel();
      hline->BorderStyle = BorderStyle::None;
      hline->Location = Point(0, i * 45 - offset);
      hline->Size = System::Drawing::Size(405, thickness);
      hline->BackColor = Color::Black;
      gridContainer->Controls->Add(hline);
    }

    // Initialize grid cells as proper cells (45x45 pixels each, no gaps)
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        grid[i, j] = gcnew TextBox();
        grid[i, j]->Size = System::Drawing::Size(45, 45);  // Full cell size
        grid[i, j]->Location = System::Drawing::Point(j * 45, i * 45);  // Perfectly positioned
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
      }
    }

    Label^ debugLabel = gcnew Label();
    debugLabel->Text = "Debug Output";
    debugLabel->Location = Point(500, gridTop);
    debugLabel->AutoSize = true;
    this->Controls->Add(debugLabel);

    // Initialize debug box
    debugBox = gcnew TextBox();
    debugBox->Multiline = true;
    debugBox->ScrollBars = ScrollBars::Vertical;
    debugBox->ReadOnly = true;
    debugBox->Location = Point(500, gridTop+20);  // Position next to grid
    debugBox->Size = System::Drawing::Size(250, 385);  // Same height as grid
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

  void Form_Resize(Object^ sender, EventArgs^ e) {
    int gridTop = menuStrip->Height + toolStrip->Height + instructionsBox->Height + 25;
    int availableWidth = this->ClientSize.Width - 100;  // 50px margin left, 50px for debug
    int availableHeight = this->ClientSize.Height - gridTop - statusStrip->Height - 60;  // 60px padding at bottom
    
    // Keep grid square and fit within available space - cap at 450px max
    int gridSize = System::Math::Min(availableWidth, availableHeight);
    gridSize = System::Math::Min(gridSize, 450);  // Maximum size cap
    gridSize = System::Math::Max(gridSize, 180);  // Minimum size
    
    gridContainer->Size = System::Drawing::Size(gridSize, gridSize);
    gridContainer->Location = Point(50, gridTop);
    
    // Calculate cell size based on grid size
    int cellSize = gridSize / 9;
    
    // Calculate font size based on cell size (proportional)
    float fontSize = System::Math::Max(8.0f, (float)cellSize * 0.6f);
    
    // Update all cells and grid lines
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        grid[i, j]->Size = System::Drawing::Size(cellSize, cellSize);
        grid[i, j]->Location = System::Drawing::Point(j * cellSize, i * cellSize);
        grid[i, j]->Font = gcnew System::Drawing::Font(L"Arial", fontSize);
      }
    }
    
    // Update grid lines
    gridContainer->Controls->Clear();
    for (int i = 0; i <= 9; i++) {
      int thickness = (i % 3 == 0) ? 3 : 1;
      int offset = (i % 3 == 0) ? 1 : 0;
      
      Panel ^ vline = gcnew Panel();
      vline->BorderStyle = BorderStyle::None;
      vline->Location = Point(i * cellSize - offset, 0);
      vline->Size = System::Drawing::Size(thickness, gridSize);
      vline->BackColor = Color::Black;
      gridContainer->Controls->Add(vline);

      Panel ^ hline = gcnew Panel();
      hline->BorderStyle = BorderStyle::None;
      hline->Location = Point(0, i * cellSize - offset);
      hline->Size = System::Drawing::Size(gridSize, thickness);
      hline->BackColor = Color::Black;
      gridContainer->Controls->Add(hline);
    }
    
    // Re-add cells on top of grid lines
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        gridContainer->Controls->Add(grid[i, j]);
      }
    }
    
    // Adjust debug box
    debugBox->Location = Point(50 + gridSize + 20, gridTop);
    debugBox->Size = System::Drawing::Size(availableWidth - gridSize - 20, gridSize);
  }

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
      ValidateAndHighlight();
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
          ValidateAndHighlight();
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

  void ToggleNotesMode_Click(Object ^ sender, EventArgs ^ e) {
    // Notes mode removed
  }

  void DisplayNotes(int row, int col) {
    // Notes mode removed
  }

  void ValidateAndHighlight() {
    // Clear all previous highlights
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        grid[i, j]->BackColor = Color::White;
        grid[i, j]->ForeColor = Color::Black;
      }
    }

    // Check each cell for conflicts
    for (int row = 0; row < 9; row++) {
      for (int col = 0; col < 9; col++) {
        int value = sudoku->GetValue(row, col);
        if (value == -1) continue;  // Skip empty cells

        // Check row for duplicates
        for (int c = 0; c < 9; c++) {
          if (c != col && sudoku->GetValue(row, c) == value) {
            // Highlight both cells as conflicts
            grid[row, col]->BackColor = Color::Red;
            grid[row, col]->ForeColor = Color::White;
            grid[row, c]->BackColor = Color::Red;
            grid[row, c]->ForeColor = Color::White;
          }
        }

        // Check column for duplicates
        for (int r = 0; r < 9; r++) {
          if (r != row && sudoku->GetValue(r, col) == value) {
            // Highlight both cells as conflicts
            grid[row, col]->BackColor = Color::Red;
            grid[row, col]->ForeColor = Color::White;
            grid[r, col]->BackColor = Color::Red;
            grid[r, col]->ForeColor = Color::White;
          }
        }

        // Check 3x3 box for duplicates
        int boxRow = (row / 3) * 3;
        int boxCol = (col / 3) * 3;
        for (int r = boxRow; r < boxRow + 3; r++) {
          for (int c = boxCol; c < boxCol + 3; c++) {
            if ((r != row || c != col) && sudoku->GetValue(r, c) == value) {
              // Highlight both cells as conflicts
              grid[row, col]->BackColor = Color::Red;
              grid[row, col]->ForeColor = Color::White;
              grid[r, c]->BackColor = Color::Red;
              grid[r, c]->ForeColor = Color::White;
            }
          }
        }
      }
    }
  }

  void Cell_TextChanged(Object ^ sender, EventArgs ^ e) {
    TextBox ^ textBox = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(textBox->Tag);
    int row = position[0];
    int col = position[1];

    if (String::IsNullOrEmpty(textBox->Text)) {
      sudoku->ClearValue(row, col);
    } else {
      int value;
      if (Int32::TryParse(textBox->Text, value) && value >= 1 && value <= 9) {
        sudoku->SetValue(row, col, value - 1);
      } else {
        textBox->Text = "";
      }
    }
    ValidateAndHighlight();
  }

  void Cell_KeyPress(Object ^ sender, KeyPressEventArgs ^ e) {
    // Suppress the beep for all keys - let KeyDown handle everything
    e->Handled = true;
  }

  void Cell_KeyDown(Object ^ sender, KeyEventArgs ^ e) {
    TextBox ^ textBox = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(textBox->Tag);
    int row = position[0];
    int col = position[1];

    // Handle number keys 0-9
    if (e->KeyCode >= Keys::D0 && e->KeyCode <= Keys::D9) {
      int digit = e->KeyCode - Keys::D0;  // Convert to 0-9
      if (digit == 0) {
        // 0 clears the cell
        sudoku->ClearValue(row, col);
        textBox->Text = "";
      } else {
        // 1-9 sets the value
        sudoku->SetValue(row, col, digit - 1);
        textBox->Text = digit.ToString();
      }
      ValidateAndHighlight();
      e->Handled = true;
      return;
    }

    // Handle numeric keypad 0-9
    if (e->KeyCode >= Keys::NumPad0 && e->KeyCode <= Keys::NumPad9) {
      int digit = e->KeyCode - Keys::NumPad0;  // Convert to 0-9
      if (digit == 0) {
        // 0 clears the cell
        sudoku->ClearValue(row, col);
        textBox->Text = "";
      } else {
        // 1-9 sets the value
        sudoku->SetValue(row, col, digit - 1);
        textBox->Text = digit.ToString();
      }
      ValidateAndHighlight();
      e->Handled = true;
      return;
    }

    // Handle backspace to clear
    if (e->KeyCode == Keys::Back) {
      sudoku->ClearValue(row, col);
      textBox->Text = "";
      ValidateAndHighlight();
      e->Handled = true;
      return;
    }

    // Handle arrow keys for navigation
    switch (e->KeyCode) {
      case Keys::Up:
        if (row > 0)
          grid[row - 1, col]->Focus();
        else
          grid[8, col]->Focus();
        e->Handled = true;
        break;
      case Keys::Down:
        if (row < 8)
          grid[row + 1, col]->Focus();
        else
          grid[0, col]->Focus();
        e->Handled = true;
        break;
      case Keys::Left:
        if (col > 0)
          grid[row, col - 1]->Focus();
        else
          grid[row, 8]->Focus();
        e->Handled = true;
        break;
      case Keys::Right:
        if (col < 8)
          grid[row, col + 1]->Focus();
        else
          grid[row, 0]->Focus();
        e->Handled = true;
        break;
      case Keys::Z:
        if (e->Control) {
          NewGame_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::A:
        Solve_Click(nullptr, nullptr);
        e->Handled = true;
        break;
      case Keys::S:
        if (!e->Shift) {
          StdElim_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::L:
        if (!e->Shift) {
          LineElim_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::N:
        if (!e->Shift) {
          HiddenSingles_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::H:
        if (!e->Shift) {
          HiddenPairs_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::P:
        if (!e->Shift) {
          PointingPairs_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::K:
        if (!e->Shift) {
          NakedSets_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::X:
        if (!e->Shift) {
          XWing_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::F:
        if (!e->Shift) {
          Swordfish_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::Y:
        if (!e->Shift) {
          XYWing_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::OemSemicolon:  // ;
        if (!e->Shift) {
          XYZWing_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::F1:
        if (!e->Shift) {
          GenerateEasy_Click(nullptr, nullptr);
        } else {
          GenerateExpert_Click(nullptr, nullptr);
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
          GenerateMaster_Click(nullptr, nullptr);
        }
        e->Handled = true;
        break;
      case Keys::F5:
        if (!e->Shift) {
          sudoku->SaveToFile("sudoku_1.txt");
          UpdateStatus("Game saved to sudoku_1.txt");
        } else {
          if (sudoku->LoadFromFile("sudoku_1.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_1.txt");
          } else {
            UpdateStatus("Failed to load sudoku_1.txt");
          }
        }
        e->Handled = true;
        break;
      case Keys::F6:
        if (!e->Shift) {
          sudoku->SaveToFile("sudoku_2.txt");
          UpdateStatus("Game saved to sudoku_2.txt");
        } else {
          if (sudoku->LoadFromFile("sudoku_2.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_2.txt");
          } else {
            UpdateStatus("Failed to load sudoku_2.txt");
          }
        }
        e->Handled = true;
        break;
      case Keys::F7:
        if (!e->Shift) {
          sudoku->SaveToFile("sudoku_3.txt");
          UpdateStatus("Game saved to sudoku_3.txt");
        } else {
          if (sudoku->LoadFromFile("sudoku_3.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_3.txt");
          } else {
            UpdateStatus("Failed to load sudoku_3.txt");
          }
        }
        e->Handled = true;
        break;
      case Keys::F8:
        if (!e->Shift) {
          sudoku->SaveToFile("sudoku_4.txt");
          UpdateStatus("Game saved to sudoku_4.txt");
        } else {
          if (sudoku->LoadFromFile("sudoku_4.txt")) {
            ClearDebugBox();
            UpdateGrid();
            UpdateStatus("Game loaded from sudoku_4.txt");
          } else {
            UpdateStatus("Failed to load sudoku_4.txt");
          }
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
