#define _HAS_STD_BYTE 0
#define NOMINMAX
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

    // Add Menus to MenuStrip
    menuStrip->Items->Add(fileMenu);
    menuStrip->Items->Add(generateBoardMenu);

    // Attach MenuStrip to the Form
    this->MainMenuStrip = menuStrip;
    this->Controls->Add(menuStrip);

    // Initialize ToolStrip
    toolStrip = gcnew ToolStrip();
    toolStrip->Items->Add(gcnew ToolStripButton(
        "Complete Auto-Solve (A)", nullptr,
        gcnew EventHandler(this, &MainForm::Solve_Click)));
    toolStrip->Items->Add(gcnew ToolStripSeparator());

    toolStrip->Items->Add(gcnew ToolStripButton(
        "Copy Board (Word)", nullptr,
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
        L"  - Press F5-F8 to save, and Shift+F5-F8 to load.";
        L"  - Press F9-F12 to save as XML Spreadsheet in the format puzzle1.xml, puzzle2.xml, etc.";

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

    // Create a container panel for the Sudoku grid
    Panel ^ gridContainer = gcnew Panel();
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
        grid[i, j]->Tag = gcnew array<int>{i, j};
        grid[i, j]->TextChanged +=
            gcnew EventHandler(this, &MainForm::Cell_TextChanged);
        grid[i, j]->KeyDown +=
            gcnew KeyEventHandler(this, &MainForm::Cell_KeyDown);
        grid[i, j]->BackColor = Color::White;
        gridContainer->Controls->Add(grid[i, j]);
        grid[i, j]->MouseWheel += gcnew MouseEventHandler(this, &MainForm::Cell_MouseWheel);
        grid[i, j]->MouseDown += gcnew MouseEventHandler(this, &MainForm::Cell_MouseDown);
      }
    }

    // Draw grid lines
    for (int i = 0; i <= 9; i++) {
      Panel ^ vline = gcnew Panel();
      vline->BorderStyle = BorderStyle::None;
      vline->Location = Point(i * 45, 0);
      vline->Size = System::Drawing::Size(i % 3 == 0 ? 3 : 1, 405);
      vline->BackColor = i % 3 == 0 ? Color::Red : Color::LightGray;
      gridContainer->Controls->Add(vline);

      Panel ^ hline = gcnew Panel();
      hline->BorderStyle = BorderStyle::None;
      hline->Location = Point(0, i * 45);
      hline->Size = System::Drawing::Size(405, i % 3 == 0 ? 3 : 1);
      hline->BackColor = i % 3 == 0 ? Color::Red : Color::LightGray;
      gridContainer->Controls->Add(hline);
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
        // Create HTML table representation
        String^ htmlContent = GetBoardAsText();

        // Create DataObject to hold multiple formats
        DataObject^ dataObj = gcnew DataObject();
        
        // Add HTML format
        String^ htmlFormat = "<html><body>" + htmlContent + "</body></html>";
        dataObj->SetData(DataFormats::Html, htmlFormat);
        
        // Also add plain text format as fallback
        String^ plainText = "";
        for (int i = 0; i < 9; i++) {
            // Add horizontal separator
            if (i % 3 == 0 && i != 0) {
                plainText += "+---+---+---+---+---+---+\r\n";
            }
            
            for (int j = 0; j < 9; j++) {
                // Add vertical separator
                if (j % 3 == 0) {
                    plainText += "|";
                }
                
                String^ value = grid[i, j]->Text;
                plainText += String::IsNullOrEmpty(value) ? " . " : " " + value + " ";
            }
            plainText += "|\r\n";
        }
        plainText += "+---+---+---+---+---+---+\r\n";
        
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

  void UpdateStatus(String ^ message) {
    statusLabel->Text = message;
    statusStrip->Refresh();
    debugBox->AppendText(message + "\r\n");
    debugBox->SelectionStart = debugBox->Text->Length;
    debugBox->ScrollToCaret();

    /*sudoku->print_debug(message);*/

  }

  void Cell_TextChanged(Object ^ sender, EventArgs ^ e) {
    TextBox ^ textBox = safe_cast<TextBox ^>(sender);
    array<int> ^ position = safe_cast<array<int> ^>(textBox->Tag);

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
