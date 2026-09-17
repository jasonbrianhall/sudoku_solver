// Sudoku Solver - wxWidgets port of win_main.cpp (originally C++/CLI +
// Windows Forms). Behaviour ported 1:1 where practical; see README.md at
// the project root for a list of the handful of deliberate differences.
#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "sudoku_wrapper.h"
#include "generatepuzzle.h"
#include "grid_widget.h"

namespace {
const wxColour kImmutableBg(173, 216, 230);   // LightBlue
const wxColour kImmutableFg(0, 0, 139);       // DarkBlue
const wxColour kConflictBg(255, 0, 0);
const wxColour kConflictFg(255, 255, 255);
}

class SolverFrame : public wxFrame {
public:
    SolverFrame()
        : wxFrame(nullptr, wxID_ANY, "Sudoku Solver", wxDefaultPosition, wxSize(1000, 700)) {
        BuildMenu();
        BuildToolbar();
        BuildLayout();
        CreateStatusBar();
        SetStatusText("Ready");

        board->onCellKeyDown = [this](int r, int c, wxKeyEvent& e) { OnCellKeyDown(r, c, e); };
        board->onCellMouseWheel = [this](int r, int c, wxMouseEvent& e) { OnCellMouseWheel(r, c, e); };
        board->onCellMouseDown = [this](int r, int c, wxMouseEvent& e) { OnCellMouseDown(r, c, e); };

        debugTimer.Bind(wxEVT_TIMER, &SolverFrame::OnDebugTimer, this);
        debugTimer.Start(16);

        UpdateGrid();

        wxIcon icon;
        if (wxFileExists("app.ico") && icon.LoadFile("app.ico", wxBITMAP_TYPE_ICO))
            SetIcon(icon);
    }

private:
    SudokuWrapper sudoku;
    SudokuGridWidget* board = nullptr;
    wxTextCtrl* instructionsBox = nullptr;
    wxTextCtrl* debugBox = nullptr;
    wxToolBar* toolbar = nullptr;
    wxTimer debugTimer;
    int undoToolId = wxID_ANY;

    // --- Construction ----------------------------------------------------

    void BuildMenu() {
        wxMenuBar* menuBar = new wxMenuBar();

        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(ID_NewGame, "New Game (Z)");

        wxMenu* saveMenu = new wxMenu();
        for (int i = 1; i <= 4; i++)
            saveMenu->Append(ID_SaveSlot1 + (i - 1), wxString::Format("Slot %d  F(%d)", i, i + 4));
        fileMenu->AppendSubMenu(saveMenu, "Save Game");

        wxMenu* loadMenu = new wxMenu();
        for (int i = 1; i <= 4; i++)
            loadMenu->Append(ID_LoadSlot1 + (i - 1), wxString::Format("Slot %d  Shift+F(%d)", i, i + 4));
        fileMenu->AppendSubMenu(loadMenu, "Load Game");

        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT, "Quit");
        menuBar->Append(fileMenu, "&File");

        wxMenu* genMenu = new wxMenu();
        genMenu->Append(ID_GenEasy, "Easy  F(1)");
        genMenu->Append(ID_GenMedium, "Medium  F(2)");
        genMenu->Append(ID_GenHard, "Hard  F(3)");
        genMenu->Append(ID_GenMaster, "Master  F(4)");
        genMenu->Append(ID_GenExpert, "Expert  Shift+F(1)");
        genMenu->AppendSeparator();
        genMenu->Append(ID_ExportXml1, "Save as spreadsheet puzzle1.xml  F(9)");
        genMenu->Append(ID_ExportXml2, "Save as spreadsheet puzzle2.xml  F(10)");
        genMenu->Append(ID_ExportXml3, "Save as spreadsheet puzzle3.xml  F(11)");
        genMenu->Append(ID_ExportXml4, "Save as spreadsheet puzzle4.xml  F(12)");
        menuBar->Append(genMenu, "&Generate Board");

        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT, "About");
        helpMenu->Append(ID_SupportAuthor, "Support the Author (Buy Me a Coffee)");
        menuBar->Append(helpMenu, "&Help");

        SetMenuBar(menuBar);
        Bind(wxEVT_MENU, &SolverFrame::OnMenu, this);
    }

    void BuildToolbar() {
        toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS);
        toolbar->AddTool(ID_NewGame, "New Game (Z)", wxNullBitmap);
        undoToolId = ID_Undo;
        toolbar->AddTool(ID_Undo, "Undo (0)", wxNullBitmap);
        toolbar->AddTool(ID_ClearBoard, "Clear Board", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddTool(ID_ToggleNotes, "Toggle Notes (T)", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddTool(ID_Solve, "Complete Auto-Solve (A)", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddTool(ID_CopyBoard, "Copy Board", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddControl(new wxStaticText(toolbar, wxID_ANY, "Basic: "));
        toolbar->AddTool(ID_StdElim, "Standard Elim (S)", wxNullBitmap);
        toolbar->AddTool(ID_LineElim, "Line Elim (L)", wxNullBitmap);
        toolbar->AddTool(ID_HiddenSingles, "Hidden Singles (N)", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddControl(new wxStaticText(toolbar, wxID_ANY, "Advanced: "));
        toolbar->AddTool(ID_HiddenPairs, "Hidden Pairs (H)", wxNullBitmap);
        toolbar->AddTool(ID_PointingPairs, "Pointing Pairs (P)", wxNullBitmap);
        toolbar->AddTool(ID_NakedSets, "Naked Sets (K)", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddControl(new wxStaticText(toolbar, wxID_ANY, "Expert: "));
        toolbar->AddTool(ID_XWing, "X-Wing (X)", wxNullBitmap);
        toolbar->AddTool(ID_Swordfish, "Swordfish (F)", wxNullBitmap);
        toolbar->AddTool(ID_XYWing, "XY-Wing (Y)", wxNullBitmap);
        toolbar->AddTool(ID_XYZWing, "XYZ-Wing (;)", wxNullBitmap);
        toolbar->Realize();
        Bind(wxEVT_TOOL, &SolverFrame::OnMenu, this);
    }

    void BuildLayout() {
        wxPanel* root = new wxPanel(this);
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        instructionsBox = new wxTextCtrl(root, wxID_ANY,
            "Instructions:\n\n"
            "  - Use keypad (1-9) to enter numbers in cells. Middle mouse click or press 0 to clear.\n"
            "  - Press 'T' or click 'Toggle Notes' to show/hide the notes areas under cells.\n"
            "  - Click in a notes area to add candidate numbers (e.g., '2 5 8').\n"
            "  - Press 'A' for auto-solve. Press F1-F4 for new puzzles (easy to expert). Press Shift+F1 for master.\n"
            "  - Press F5-F8 to save, Shift+F5-F8 to load. F9-F12 to export as XML. Arrow keys navigate cells.",
            wxDefaultPosition, wxSize(-1, 90),
            wxTE_MULTILINE | wxTE_READONLY | wxBORDER_SIMPLE);
        instructionsBox->SetBackgroundColour(wxColour(173, 216, 230));
        mainSizer->Add(instructionsBox, 0, wxEXPAND | wxALL, 5);

        wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);
        board = new SudokuGridWidget(root);
        rowSizer->Add(board, 1, wxEXPAND | wxALL, 5);

        wxBoxSizer* debugCol = new wxBoxSizer(wxVERTICAL);
        debugCol->Add(new wxStaticText(root, wxID_ANY, "Debug Output"), 0, wxLEFT, 2);
        debugBox = new wxTextCtrl(root, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                   wxSize(220, -1), wxTE_MULTILINE | wxTE_READONLY);
        debugBox->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        debugCol->Add(debugBox, 1, wxEXPAND);
        rowSizer->Add(debugCol, 0, wxEXPAND | wxALL, 5);

        mainSizer->Add(rowSizer, 1, wxEXPAND);
        root->SetSizer(mainSizer);
    }

    // --- IDs ---------------------------------------------------------------
    enum {
        ID_NewGame = wxID_HIGHEST + 1, ID_Undo, ID_ClearBoard, ID_ToggleNotes,
        ID_Solve, ID_CopyBoard, ID_StdElim, ID_LineElim, ID_HiddenSingles,
        ID_HiddenPairs, ID_PointingPairs, ID_NakedSets, ID_XWing, ID_Swordfish,
        ID_XYWing, ID_XYZWing, ID_GenEasy, ID_GenMedium, ID_GenHard, ID_GenMaster,
        ID_GenExpert, ID_ExportXml1, ID_ExportXml2, ID_ExportXml3, ID_ExportXml4,
        ID_SaveSlot1, ID_SaveSlot2, ID_SaveSlot3, ID_SaveSlot4,
        ID_LoadSlot1, ID_LoadSlot2, ID_LoadSlot3, ID_LoadSlot4,
        ID_SupportAuthor
    };

    // --- Status / grid rendering --------------------------------------

    void UpdateStatus(const wxString& msg) {
        SetStatusText(msg);
        debugBox->AppendText(msg + "\n");
    }

    void UpdateGrid() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                board->SetCellValue(i, j, sudoku.GetValue(i, j));
                if (sudoku.IsCellImmutable(i, j))
                    board->SetCellStyle(i, j, kImmutableBg, kImmutableFg, true);
                else
                    board->SetCellStyle(i, j, *wxWHITE, *wxBLACK, false);
                board->ClearNotes(i, j);
            }
        }
        ValidateAndHighlight();
        board->ReselectFocusedCell();
    }

    // Unlike UpdateGrid(), this only re-checks conflicts and restores each
    // cell to its correct base colour first (immutable cells stay blue).
    void ValidateAndHighlight() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (sudoku.IsCellImmutable(i, j))
                    board->SetCellStyle(i, j, kImmutableBg, kImmutableFg, true);
                else
                    board->SetCellStyle(i, j, *wxWHITE, *wxBLACK, false);
            }
        }
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                int value = sudoku.GetValue(row, col);
                if (value == -1) continue;
                bool conflict = false;
                for (int c = 0; c < 9 && !conflict; c++)
                    if (c != col && sudoku.GetValue(row, c) == value) conflict = true;
                for (int r = 0; r < 9 && !conflict; r++)
                    if (r != row && sudoku.GetValue(r, col) == value) conflict = true;
                if (!conflict) {
                    int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
                    for (int r = boxRow; r < boxRow + 3 && !conflict; r++)
                        for (int c = boxCol; c < boxCol + 3 && !conflict; c++)
                            if ((r != row || c != col) && sudoku.GetValue(r, c) == value) conflict = true;
                }
                if (conflict)
                    board->SetCellStyle(row, col, kConflictBg, kConflictFg,
                                         sudoku.IsCellImmutable(row, col));
            }
        }
    }

    void UpdateUndoButtonState() {
        toolbar->SetToolShortHelp(ID_Undo, "Undo");
        wxString label = wxString::Format("Undo (%d)", sudoku.GetUndoCount());
        toolbar->FindById(ID_Undo)->SetLabel(label);
        toolbar->Realize();
        toolbar->EnableTool(ID_Undo, sudoku.CanUndo());
    }

    void ClearDebugBox() { debugBox->Clear(); }

    // --- Puzzle generation -------------------------------------------

    void GeneratePuzzle(const std::string& difficulty, const wxString& label) {
        PuzzleGenerator generator(*sudoku.NativeSudoku());
        if (generator.generatePuzzle(difficulty)) {
            sudoku.Clean();
            sudoku.MarkPuzzleAsGenerated();
            sudoku.ClearUndoHistory();
            UpdateGrid();
            UpdateUndoButtonState();
            UpdateStatus("Generated new " + label + " puzzle - clues are immutable");
        } else {
            UpdateStatus("Failed to generate " + label + " puzzle");
            sudoku.NewGame();
            UpdateGrid();
        }
    }

    // --- Algorithm running (runs synchronously - the solver is fast
    // enough on any reasonably-sized board that this doesn't need a
    // background thread the way the timed Game app's does) -------------

    void RunAlgorithm(const wxString& label, std::function<void()> fn) {
        if (!sudoku.IsValidSolution()) {
            UpdateStatus("Current board is invalid");
            return;
        }
        ClearDebugBox();
        fn();
        UpdateGrid();
        UpdateStatus(label + " completed");
    }

    // --- Event handlers --------------------------------------------------

    void OnMenu(wxCommandEvent& evt) {
        int id = evt.GetId();
        switch (id) {
            case ID_NewGame:
                sudoku.NewGame();
                sudoku.ClearUndoHistory();
                UpdateGrid();
                UpdateUndoButtonState();
                UpdateStatus("New game started");
                break;
            case ID_Undo: DoUndo(); break;
            case ID_ClearBoard: DoClearBoard(); break;
            case ID_ToggleNotes: {
                board->SetNotesVisible(!board->GetNotesVisible());
                UpdateStatus(board->GetNotesVisible() ? "Notes shown" : "Notes hidden");
                break;
            }
            case ID_Solve: RunAlgorithm("Full solve", [this]{ sudoku.Solve(); }); break;
            case ID_CopyBoard: DoCopyBoard(); break;
            case ID_StdElim: RunAlgorithm("Standard elimination", [this]{ sudoku.StdElim(); }); break;
            case ID_LineElim: RunAlgorithm("Line elimination", [this]{ sudoku.LinElim(); }); break;
            case ID_HiddenSingles: RunAlgorithm("Hidden singles", [this]{ sudoku.FindHiddenSingles(); }); break;
            case ID_HiddenPairs: RunAlgorithm("Hidden pairs", [this]{ sudoku.FindHiddenPairs(); }); break;
            case ID_PointingPairs: RunAlgorithm("Pointing pairs", [this]{ sudoku.FindPointingPairs(); }); break;
            case ID_NakedSets: RunAlgorithm("Naked sets", [this]{ sudoku.FindNakedSets(); }); break;
            case ID_XWing: RunAlgorithm("X-Wing technique", [this]{ sudoku.FindXWing(); }); break;
            case ID_Swordfish: RunAlgorithm("Swordfish technique", [this]{ sudoku.FindSwordFish(); }); break;
            case ID_XYWing: RunAlgorithm("XY-Wing technique", [this]{ sudoku.FindXYWing(); }); break;
            case ID_XYZWing: RunAlgorithm("XYZ-Wing technique", [this]{ sudoku.FindXYZWing(); }); break;
            case ID_GenEasy: GeneratePuzzle("easy", "easy"); break;
            case ID_GenMedium: GeneratePuzzle("medium", "medium"); break;
            case ID_GenHard: GeneratePuzzle("hard", "hard"); break;
            case ID_GenMaster: GeneratePuzzle("extreme", "extreme"); break;
            case ID_GenExpert: GeneratePuzzle("expert", "expert"); break;
            case ID_ExportXml1: sudoku.ExportToExcelXML("puzzle1.xml"); UpdateStatus("Saved puzzle as puzzle1.xml"); break;
            case ID_ExportXml2: sudoku.ExportToExcelXML("puzzle2.xml"); UpdateStatus("Saved puzzle as puzzle2.xml"); break;
            case ID_ExportXml3: sudoku.ExportToExcelXML("puzzle3.xml"); UpdateStatus("Saved puzzle as puzzle3.xml"); break;
            case ID_ExportXml4: sudoku.ExportToExcelXML("puzzle4.xml"); UpdateStatus("Saved puzzle as puzzle4.xml"); break;
            case wxID_EXIT: Close(true); break;
            case wxID_ABOUT: ShowAbout(); break;
            case ID_SupportAuthor: ShowSupport(); break;
            default:
                if (id >= ID_SaveSlot1 && id <= ID_SaveSlot4) SaveSlot(id - ID_SaveSlot1 + 1);
                else if (id >= ID_LoadSlot1 && id <= ID_LoadSlot4) LoadSlot(id - ID_LoadSlot1 + 1);
                break;
        }
    }

    void DoUndo() {
        if (sudoku.Undo()) {
            UpdateGrid();
            UpdateUndoButtonState();
            UpdateStatus(wxString::Format("Undo completed - %d states remaining", sudoku.GetUndoCount()));
        } else {
            UpdateStatus("Nothing to undo");
        }
    }

    void DoClearBoard() {
        int result = wxMessageBox("Clear all non-immutable cells? This action can be undone with Ctrl+Z.",
                                   "Clear Board", wxYES_NO | wxICON_QUESTION, this);
        if (result == wxYES) {
            sudoku.SaveBoardState();
            sudoku.ClearBoardExceptImmutable();
            UpdateGrid();
            UpdateUndoButtonState();
            UpdateStatus("Board cleared - immutable cells preserved");
        }
    }

    void DoCopyBoard() {
        wxString plain = "Sudoku Puzzle\n";
        for (int i = 0; i < 9; i++) {
            if (i % 3 == 0) plain += "+----+----+----+----+----+----+\n";
            for (int j = 0; j < 9; j++) {
                if (j % 3 == 0) plain += "|";
                int v = sudoku.GetValue(i, j);
                plain += (v >= 0) ? wxString::Format(" %d ", v + 1) : " . ";
            }
            plain += "|\n";
        }
        plain += "+----+----+----+----+----+----+\n";
        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(plain));
            wxTheClipboard->Close();
            UpdateStatus("Board copied to clipboard");
        } else {
            UpdateStatus("Failed to open clipboard");
        }
    }

    void SaveSlot(int slot) {
        wxString filename = wxString::Format("sudoku_slot_%d.txt", slot);
        sudoku.SaveToFile(filename.ToStdString());
        UpdateStatus("Game saved to " + filename);
    }

    void LoadSlot(int slot) {
        wxString filename = wxString::Format("sudoku_slot_%d.txt", slot);
        if (sudoku.LoadFromFile(filename.ToStdString())) {
            UpdateGrid();
            UpdateStatus("Game loaded from " + filename);
        } else {
            UpdateStatus("Failed to load " + filename);
        }
    }

    void ShowAbout() {
        wxString text =
            "Sudoku Solver\n\n"
            "(C) 2025 Jason Brian Hall\n"
            "MIT License - https://opensource.org/licenses/MIT\n\n"
            "GitHub: https://github.com/jasonbrianhall/sudoku_solver\n\n"
            "A powerful Sudoku puzzle generator and solver with support for multiple difficulty levels "
            "and advanced solving techniques including X-Wing, Swordfish, XY-Wing, and XYZ-Wing patterns.\n\n"
            "Features:\n"
            "- Generate puzzles at 6 difficulty levels\n"
            "- Real-time conflict detection and highlighting\n"
            "- Pencil mark notes for candidates\n"
            "- Multiple solving techniques\n"
            "- Save/load games\n"
            "- Export to Excel XML\n";
        wxMessageBox(text, "About Sudoku Solver", wxOK | wxICON_INFORMATION, this);
    }

    void ShowSupport() {
        wxMessageBox(
            "If you enjoy this Sudoku Solver, please consider supporting the author!\n\n"
            "Visit: https://buymeacoffee.com/jasonbrianhall\n\n"
            "Your support helps fund development and keeps this project active.",
            "Support the Author", wxOK | wxICON_INFORMATION, this);
    }

    void OnDebugTimer(wxTimerEvent&) {
        std::string msg;
        wxString batch;
        while (!(msg = sudoku.NativeSudoku()->get_next_debug_message()).empty())
            batch += msg + "\n";
        if (!batch.empty()) debugBox->AppendText(batch);
    }

    void OnCellMouseWheel(int row, int col, wxMouseEvent& e) {
        int currentValue = sudoku.GetValue(row, col);
        if (e.GetWheelRotation() > 0)
            currentValue = (currentValue + 1) % 10;
        else {
            currentValue--;
            if (currentValue < -1) currentValue = 8;
        }
        if (currentValue == -1) sudoku.ClearValue(row, col);
        else { sudoku.Clean(); sudoku.SetValue(row, col, currentValue); }
        board->SetCellValue(row, col, currentValue);
    }

    void OnCellMouseDown(int row, int col, wxMouseEvent& e) {
        if (e.MiddleDown()) {
            if (sudoku.IsCellImmutable(row, col)) return;
            sudoku.ClearValue(row, col);
            board->SetCellValue(row, col, -1);
        }
    }

    void OnCellKeyDown(int row, int col, wxKeyEvent& e) {
        if (e.ControlDown() && e.GetKeyCode() == 'Z') { DoUndo(); return; }

        int kc = e.GetKeyCode();
        switch (kc) {
            case WXK_LEFT:  board->FocusCell(row, col > 0 ? col - 1 : 8); return;
            case WXK_RIGHT: board->FocusCell(row, col < 8 ? col + 1 : 0); return;
            case WXK_UP:    board->FocusCell(row > 0 ? row - 1 : 8, col); return;
            case WXK_DOWN:  board->FocusCell(row < 8 ? row + 1 : 0, col); return;
            case WXK_F1: e.ShiftDown() ? GeneratePuzzle("extreme", "master") : GeneratePuzzle("easy", "easy"); return;
            case WXK_F2: if (!e.ShiftDown()) GeneratePuzzle("medium", "medium"); return;
            case WXK_F3: if (!e.ShiftDown()) GeneratePuzzle("hard", "hard"); return;
            case WXK_F4: if (!e.ShiftDown()) GeneratePuzzle("expert", "expert"); return;
            case WXK_F5: e.ShiftDown() ? LoadSlot(1) : SaveSlot(1); return;
            case WXK_F6: e.ShiftDown() ? LoadSlot(2) : SaveSlot(2); return;
            case WXK_F7: e.ShiftDown() ? LoadSlot(3) : SaveSlot(3); return;
            case WXK_F8: e.ShiftDown() ? LoadSlot(4) : SaveSlot(4); return;
            case WXK_F9:  if (!e.ShiftDown()) { sudoku.ExportToExcelXML("puzzle1.xml"); UpdateStatus("Saved puzzle1.xml"); } return;
            case WXK_F10: if (!e.ShiftDown()) { sudoku.ExportToExcelXML("puzzle2.xml"); UpdateStatus("Saved puzzle2.xml"); } return;
            case WXK_F11: if (!e.ShiftDown()) { sudoku.ExportToExcelXML("puzzle3.xml"); UpdateStatus("Saved puzzle3.xml"); } return;
            case WXK_F12: if (!e.ShiftDown()) { sudoku.ExportToExcelXML("puzzle4.xml"); UpdateStatus("Saved puzzle4.xml"); } return;
        }

        if (kc >= '1' && kc <= '9') {
            if (sudoku.IsCellImmutable(row, col)) {
                UpdateStatus("That cell is from the puzzle and cannot be changed");
                return;
            }
            sudoku.SaveBoardState();
            sudoku.Clean();
            sudoku.SetValue(row, col, kc - '1');
            UpdateGrid();
            UpdateUndoButtonState();
            return;
        }
        if (kc == '0') {
            if (sudoku.IsCellImmutable(row, col)) {
                UpdateStatus("That cell is from the puzzle and cannot be changed");
                return;
            }
            sudoku.SaveBoardState();
            sudoku.ClearValue(row, col);
            UpdateGrid();
            UpdateUndoButtonState();
            return;
        }

        switch (kc) {
            case 'S': RunAlgorithm("Standard elimination", [this]{ sudoku.StdElim(); }); return;
            case 'L': RunAlgorithm("Line elimination", [this]{ sudoku.LinElim(); }); return;
            case 'H': RunAlgorithm("Hidden pairs", [this]{ sudoku.FindHiddenPairs(); }); return;
            case 'P': RunAlgorithm("Pointing pairs", [this]{ sudoku.FindPointingPairs(); }); return;
            case 'N': RunAlgorithm("Hidden singles", [this]{ sudoku.FindHiddenSingles(); }); return;
            case 'K': RunAlgorithm("Naked sets", [this]{ sudoku.FindNakedSets(); }); return;
            case 'X': RunAlgorithm("X-Wing technique", [this]{ sudoku.FindXWing(); }); return;
            case 'F': RunAlgorithm("Swordfish technique", [this]{ sudoku.FindSwordFish(); }); return;
            case 'Y': RunAlgorithm("XY-Wing technique", [this]{ sudoku.FindXYWing(); }); return;
            case ';': if (!e.ShiftDown()) RunAlgorithm("XYZ-Wing technique", [this]{ sudoku.FindXYZWing(); }); return;
            case 'A': RunAlgorithm("Full solve", [this]{ sudoku.Solve(); }); return;
            case 'Z':
                ClearDebugBox();
                sudoku.NewGame();
                sudoku.ClearUndoHistory();
                UpdateGrid();
                UpdateUndoButtonState();
                UpdateStatus("New game started");
                return;
            case 'T':
                board->SetNotesVisible(!board->GetNotesVisible());
                UpdateStatus(board->GetNotesVisible() ? "Notes shown" : "Notes hidden");
                return;
        }
    }
};

class SolverApp : public wxApp {
public:
    bool OnInit() override {
        SolverFrame* frame = new SolverFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(SolverApp);
