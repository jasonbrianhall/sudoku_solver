// Sudoku Game - wxWidgets port of sudoku_game.cpp (originally C++/CLI +
// Windows Forms). See README.md at the project root for a list of the
// handful of deliberate differences from the original.
#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/clipbrd.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <deque>
#include <random>
#include <cstring>
#include "sudoku_wrapper.h"
#include "generatepuzzle.h"
#include "highscores.h"
#include "grid_widget.h"
#include "game_sound.h"

namespace {
const wxColour kImmutableBg = *wxWHITE;
const wxColour kImmutableFgNormal(250, 128, 114);   // Salmon
const wxColour kQuasiBg(144, 238, 144);             // LightGreen
const wxColour kQuasiFg(0, 100, 0);                 // DarkGreen
const wxColour kConflictBg(255, 0, 0);
const wxColour kConflictFg(255, 255, 255);
const wxColour kHighlightBg(255, 255, 224);         // LightYellow
const wxColour kNormalBg = *wxWHITE;
const wxColour kNormalFg = *wxBLACK;

void PlayNewGameSound()       { GameSound::PlaySequenceAsync({{392,80},{523,80},{659,80},{784,80},{1047,200}}); }
void PlayWrongSound()         { GameSound::PlayAsync(200, 150); }
void PlayCorrectSound()       { GameSound::PlayAsync(880, 80); }
void PlayQuasiImmutableSound(){ GameSound::PlaySequenceAsync({{523,80},{659,80},{784,80},{1047,160}}); }
void PlayWinSound() {
    GameSound::PlaySequenceAsync({
        {523,100},{523,100},{0,50},{523,100},{0,50},{415,100},{523,100},
        {0,100},{784,400},{0,200},{392,400},{0,200},{523,500}});
}
} // namespace

class GameFrame : public wxFrame {
public:
    GameFrame()
        : wxFrame(nullptr, wxID_ANY, "Sudoku Game", wxDefaultPosition, wxSize(1000, 700)) {
        BuildMenu();
        BuildToolbar();
        BuildLayout();

        CreateStatusBar(2);
        int widths[2] = {-1, 90};
        GetStatusBar()->SetStatusWidths(2, widths);
        SetStatusText("Ready", 0);
        SetStatusText("00:00", 1);

        board->onCellKeyDown = [this](int r, int c, wxKeyEvent& e) { OnCellKeyDown(r, c, e); };
        board->onCellMouseWheel = [this](int r, int c, wxMouseEvent& e) { OnCellMouseWheel(r, c, e); };
        board->onCellMouseDown = [this](int r, int c, wxMouseEvent& e) { OnCellMouseDown(r, c, e); };
        board->onCellFocus = [this](int r, int c) { OnCellFocus(r, c); };
        board->onCellLostFocus = [this](int r, int c) { OnCellLostFocus(r, c); };

        debugTimer.Bind(wxEVT_TIMER, &GameFrame::OnDebugTimer, this);
        debugTimer.Start(16);
        gameTimer.Bind(wxEVT_TIMER, &GameFrame::OnGameTimer, this);
        gameTimer.Start(1000);
        celebrationTimer.Bind(wxEVT_TIMER, &GameFrame::OnCelebrationTick, this);

        Bind(wxEVT_ACTIVATE, &GameFrame::OnActivate, this);

        wxIcon icon;
        if (wxFileExists("app.ico") && icon.LoadFile("app.ico", wxBITMAP_TYPE_ICO))
            SetIcon(icon);

        // Auto-generate an easy puzzle on startup (as if F1 was pressed).
        GeneratePuzzle("easy", "easy", false);
    }

private:
    SudokuWrapper sudoku;
    Highscores highscores;
    SudokuGridWidget* board = nullptr;
    wxTextCtrl* instructionsBox = nullptr;
    wxTextCtrl* debugBox = nullptr;
    wxToolBar* toolbar = nullptr;
    wxTimer debugTimer, gameTimer, celebrationTimer;

    int elapsedSeconds = 0;
    bool timerPaused = false;
    std::deque<std::pair<int,int>> correctQueue;
    std::string currentDifficulty;
    bool puzzleSolved = false;
    int highlightValue = -1;
    bool colorblindMode = false;

    int celebrationStep = 0;
    bool pendingHighScore = false;
    int pendingElapsedSeconds = 0;
    std::string pendingDiff;

    enum {
        ID_GenEasy = wxID_HIGHEST + 1, ID_GenMedium, ID_GenHard, ID_GenExpert, ID_GenMaster,
        ID_SaveSlot1, ID_SaveSlot2, ID_SaveSlot3, ID_SaveSlot4,
        ID_LoadSlot1, ID_LoadSlot2, ID_LoadSlot3, ID_LoadSlot4,
        ID_ViewHighscores, ID_ColorblindMode, ID_SupportAuthor,
        ID_ToggleNotes, ID_CopyBoard
    };

    // --- Construction ------------------------------------------------------

    void BuildMenu() {
        wxMenuBar* menuBar = new wxMenuBar();

        wxMenu* gameMenu = new wxMenu();
        wxMenu* newGameMenu = new wxMenu();
        newGameMenu->Append(ID_GenEasy, "Easy  F(1)");
        newGameMenu->Append(ID_GenMedium, "Medium  F(2)");
        newGameMenu->Append(ID_GenHard, "Hard  F(3)");
        newGameMenu->Append(ID_GenMaster, "Master  F(4)");
        newGameMenu->Append(ID_GenExpert, "Expert  Shift+F(1)");
        gameMenu->AppendSubMenu(newGameMenu, "New Game");

        wxMenu* saveMenu = new wxMenu();
        for (int i = 1; i <= 4; i++)
            saveMenu->Append(ID_SaveSlot1 + (i - 1), wxString::Format("Slot %d  F(%d)", i, i + 4));
        gameMenu->AppendSubMenu(saveMenu, "Save Game");

        wxMenu* loadMenu = new wxMenu();
        for (int i = 1; i <= 4; i++)
            loadMenu->Append(ID_LoadSlot1 + (i - 1), wxString::Format("Slot %d  Shift+F(%d)", i, i + 4));
        gameMenu->AppendSubMenu(loadMenu, "Load Game");

        gameMenu->Append(ID_ViewHighscores, "High Scores");
        gameMenu->AppendSeparator();
        gameMenu->Append(wxID_EXIT, "Quit");
        menuBar->Append(gameMenu, "&Game");

        wxMenu* optionsMenu = new wxMenu();
        optionsMenu->AppendCheckItem(ID_ColorblindMode, "Colorblind Mode");
        menuBar->Append(optionsMenu, "&Options");

        wxMenu* helpMenu = new wxMenu();
        helpMenu->Append(wxID_ABOUT, "About");
        helpMenu->Append(ID_SupportAuthor, "Support the Author (Buy Me a Coffee)");
        menuBar->Append(helpMenu, "&Help");

        SetMenuBar(menuBar);
        Bind(wxEVT_MENU, &GameFrame::OnMenu, this);
    }

    void BuildToolbar() {
        toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT | wxTB_NOICONS);
        toolbar->AddTool(ID_ToggleNotes, "Toggle Notes (T)", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->AddTool(ID_CopyBoard, "Copy Board", wxNullBitmap);
        toolbar->AddSeparator();
        toolbar->Realize();
        Bind(wxEVT_TOOL, &GameFrame::OnMenu, this);
    }

    void BuildLayout() {
        wxPanel* root = new wxPanel(this);
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        instructionsBox = new wxTextCtrl(root, wxID_ANY,
            "Instructions:\n\n"
            "  - Use keypad (1-9) to enter numbers in cells. Middle mouse click or press 0 to clear.\n"
            "  - Press 'T' or click 'Toggle Notes' to show/hide the notes areas under cells.\n"
            "  - Click in a notes area to add candidate numbers (e.g., '2 5 8').\n"
            "  - Press F1-F4 for new puzzles (easy to master). Press Shift+F1 for expert.\n"
            "  - Press F5-F8 to save, Shift+F5-F8 to load. Arrow keys navigate cells.",
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

    // --- Save directory (per-user config dir instead of hardcoded paths) --

    wxString GetSaveDir() {
        wxString dir = wxStandardPaths::Get().GetUserDataDir();
        if (!wxFileName::DirExists(dir)) wxFileName::Mkdir(dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        return dir;
    }
    wxString GetSavePath(const wxString& filename) {
        return GetSaveDir() + wxFileName::GetPathSeparator() + filename;
    }

    // --- Status / grid rendering -------------------------------------

    void UpdateStatus(const wxString& msg) {
        SetStatusText(msg, 0);
        debugBox->AppendText(msg + "\n");
    }

    void ClearDebugBox() { debugBox->Clear(); }

    void UpdateGrid() {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                board->SetCellValue(i, j, sudoku.GetValue(i, j));
        ValidateAndHighlight();
        CheckForWin();
        board->ReselectFocusedCell();
    }

    void ValidateAndHighlight() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (sudoku.IsCellImmutable(i, j)) {
                    if (colorblindMode)
                        board->SetCellStyle(i, j, *wxWHITE, kNormalFg, true, false, true);
                    else
                        board->SetCellStyle(i, j, kImmutableBg, kImmutableFgNormal, true);
                } else if (sudoku.IsCellQuasiImmutable(i, j)) {
                    if (colorblindMode)
                        board->SetCellStyle(i, j, *wxWHITE, kNormalFg, true, true);
                    else
                        board->SetCellStyle(i, j, kQuasiBg, kQuasiFg, true);
                } else {
                    board->SetCellStyle(i, j, kNormalBg, kNormalFg, false);
                }
            }
        }

        if (highlightValue >= 0) {
            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                    if (sudoku.GetValue(i, j) == highlightValue)
                        board->SetCellStyle(i, j, kHighlightBg, kNormalFg,
                                             sudoku.IsCellImmutable(i, j) || sudoku.IsCellQuasiImmutable(i, j));
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
                if (conflict) board->SetCellStyle(row, col, kConflictBg, kConflictFg, true);
            }
        }
    }

    // --- New game / puzzle generation -----------------------------------

    bool ConfirmNewGame() {
        if (puzzleSolved || currentDifficulty.empty()) return true;
        bool hasProgress = false;
        for (int i = 0; i < 9 && !hasProgress; i++)
            for (int j = 0; j < 9 && !hasProgress; j++)
                if (!sudoku.IsCellImmutable(i, j) && sudoku.GetValue(i, j) != -1)
                    hasProgress = true;
        if (!hasProgress) return true;
        return wxMessageBox("You have a game in progress. Start a new game?", "New Game",
                             wxYES_NO | wxICON_QUESTION, this) == wxYES;
    }

    void GeneratePuzzle(const std::string& difficulty, const std::string& label, bool confirm) {
        if (confirm && !ConfirmNewGame()) return;
        celebrationTimer.Stop();
        PuzzleGenerator generator(*sudoku.NativeSudoku());
        if (generator.generatePuzzle(difficulty)) {
            sudoku.Clean();
            sudoku.MarkPuzzleAsGenerated();
            currentDifficulty = label;
            puzzleSolved = false;
            correctQueue.clear();
            ResetTimer();
            UpdateGrid();
            PlayNewGameSound();
            UpdateStatus("Generated new " + label + " puzzle - clues are immutable");
        } else {
            UpdateStatus("Failed to generate " + label + " puzzle");
            sudoku.NewGame();
            UpdateGrid();
        }
    }

    void ResetTimer() {
        elapsedSeconds = 0;
        timerPaused = false;
        SetStatusText("00:00", 1);
        gameTimer.Start(1000);
    }

    // --- Win detection & celebration -------------------------------

    void CheckForWin() {
        if (puzzleSolved || currentDifficulty.empty()) return;
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (sudoku.GetValue(i, j) == -1) return;

        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                int value = sudoku.GetValue(row, col);
                for (int c = 0; c < 9; c++)
                    if (c != col && sudoku.GetValue(row, c) == value) return;
                for (int r = 0; r < 9; r++)
                    if (r != row && sudoku.GetValue(r, col) == value) return;
                int boxRow = (row / 3) * 3, boxCol = (col / 3) * 3;
                for (int r = boxRow; r < boxRow + 3; r++)
                    for (int c = boxCol; c < boxCol + 3; c++)
                        if ((r != row || c != col) && sudoku.GetValue(r, c) == value) return;
            }
        }

        puzzleSolved = true;
        gameTimer.Stop();

        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (!sudoku.IsCellImmutable(i, j) && !sudoku.IsCellQuasiImmutable(i, j))
                    sudoku.SetQuasiImmutable(i, j);
        UpdateGridColorsOnly();

        PlayWinSound();

        bool isHigh = highscores.isHighScore(elapsedSeconds, currentDifficulty);
        UpdateStatus("Congratulations! You solved the " + currentDifficulty + " puzzle!");
        StartVictoryCelebration(isHigh, elapsedSeconds, currentDifficulty);
    }

    // Re-applies cell colours without re-triggering CheckForWin (used right
    // after locking cells at the moment of victory).
    void UpdateGridColorsOnly() {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                board->SetCellValue(i, j, sudoku.GetValue(i, j));
        ValidateAndHighlight();
    }

    void StartVictoryCelebration(bool isHighScore, int elapsed, const std::string& diff) {
        pendingHighScore = isHighScore;
        pendingElapsedSeconds = elapsed;
        pendingDiff = diff;
        celebrationStep = 0;
        celebrationTimer.Start(30);
    }

    void OnCelebrationTick(wxTimerEvent&) {
        const int GRID_SIZE = 9;
        int maxDiag = GRID_SIZE + GRID_SIZE - 2;
        int totalSteps = maxDiag + 30;

        for (int row = 0; row < GRID_SIZE; row++) {
            for (int col = 0; col < GRID_SIZE; col++) {
                int diag = row + col;
                int waveFront = celebrationStep;
                int waveBack = celebrationStep - 8;
                bool inWave = (diag <= waveFront && diag >= waveBack);
                bool behindWave = (diag < waveBack);

                if (inWave) {
                    double t = (double)(waveFront - diag) / 8.0;
                    int r = (int)(255 - t * 155), g = (int)(200 + t * 20), b = (int)(t * 30);
                    board->SetCellStyle(row, col,
                        wxColour(wxClip(r), wxClip(g), wxClip(b)), kNormalFg, false);
                } else if (behindWave) {
                    int fadeStepsDone = celebrationStep - (diag + 8);
                    double fadeFraction = wxMax(0.0, wxMin(1.0, (double)fadeStepsDone / 30.0));
                    int r = (int)(100 + fadeFraction * 155);
                    int g = (int)(220 + fadeFraction * 35);
                    int b = (int)(80 + fadeFraction * 175);
                    board->SetCellStyle(row, col, wxColour(wxClip(r), wxClip(g), wxClip(b)), kNormalFg, false);
                }
            }
        }
        celebrationStep++;

        if (celebrationStep > totalSteps) {
            celebrationTimer.Stop();
            UpdateGridColorsOnly();

            int mins = pendingElapsedSeconds / 60, secs = pendingElapsedSeconds % 60;
            wxString timeStr = wxString::Format("%02d:%02d", mins, secs);
            wxString msg = wxString::Format("Congratulations! You solved the %s puzzle in %s!",
                                             pendingDiff, timeStr);

            if (pendingHighScore) {
                wxString name = PromptForName(msg + "\nNew high score! Enter your name:");
                Score score;
                score.name = name.ToStdString();
                score.time = pendingElapsedSeconds;
                score.difficulty = pendingDiff;
                highscores.addScore(score);
                ShowHighscoresDialog(pendingDiff);
            } else {
                wxMessageBox(msg, "Puzzle Solved!", wxOK | wxICON_INFORMATION, this);
            }
            UpdateStatus(wxString::Format("Puzzle solved in %s!", timeStr));
        }
    }

    static int wxClip(int v) { return wxMax(0, wxMin(255, v)); }

    wxString PromptForName(const wxString& message) {
        wxDialog dlg(this, wxID_ANY, "New High Score!", wxDefaultPosition, wxSize(360, 170));
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(new wxStaticText(&dlg, wxID_ANY, message, wxDefaultPosition,
                                     wxSize(320, -1)), 0, wxALL, 10);
        wxTextCtrl* nameTxt = new wxTextCtrl(&dlg, wxID_ANY, "Player", wxDefaultPosition,
                                              wxDefaultSize, wxTE_PROCESS_ENTER);
        sizer->Add(nameTxt, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
        wxButton* okBtn = new wxButton(&dlg, wxID_OK, "OK");
        sizer->Add(okBtn, 0, wxALIGN_RIGHT | wxALL, 10);
        dlg.SetSizer(sizer);
        dlg.SetAffirmativeId(wxID_OK);
        nameTxt->Bind(wxEVT_TEXT_ENTER, [&dlg](wxCommandEvent&) { dlg.EndModal(wxID_OK); });
        nameTxt->SetFocus();
        nameTxt->SelectAll();

        wxString name = "Anonymous";
        if (dlg.ShowModal() == wxID_OK) {
            name = nameTxt->GetValue().Trim().Trim(false);
            if (name.IsEmpty()) name = "Anonymous";
        }
        return name;
    }

    void ShowHighscoresDialog(const std::string& highlightDiff) {
        wxDialog dlg(this, wxID_ANY, "High Scores", wxDefaultPosition, wxSize(420, 480));
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxNotebook* tabs = new wxNotebook(&dlg, wxID_ANY);

        const char* diffs[] = {"easy", "medium", "hard", "master", "expert"};
        for (const char* d : diffs) {
            wxPanel* page = new wxPanel(tabs);
            wxBoxSizer* pageSizer = new wxBoxSizer(wxVERTICAL);
            wxListCtrl* lv = new wxListCtrl(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                             wxLC_REPORT | wxLC_SINGLE_SEL);
            lv->InsertColumn(0, "Rank", wxLIST_FORMAT_LEFT, 50);
            lv->InsertColumn(1, "Name", wxLIST_FORMAT_LEFT, 180);
            lv->InsertColumn(2, "Time", wxLIST_FORMAT_LEFT, 100);

            auto scores = highscores.getScoresByDifficulty(d);
            int rank = 1;
            for (const auto& s : scores) {
                int m = s.time / 60, sec = s.time % 60;
                long idx = lv->InsertItem(rank - 1, wxString::Format("%d", rank));
                lv->SetItem(idx, 1, s.name);
                lv->SetItem(idx, 2, wxString::Format("%02d:%02d", m, sec));
                if (std::string(d) == highlightDiff && rank == 1)
                    lv->SetItemBackgroundColour(idx, wxColour(250, 250, 165));
                rank++;
            }
            if (scores.empty()) {
                long idx = lv->InsertItem(0, "-");
                lv->SetItem(idx, 1, "No scores yet");
                lv->SetItem(idx, 2, "-");
            }
            pageSizer->Add(lv, 1, wxEXPAND);
            page->SetSizer(pageSizer);

            wxString label = wxString(d);
            label[0] = wxToupper(label[0]);
            tabs->AddPage(page, label, std::string(d) == highlightDiff);
        }

        sizer->Add(tabs, 1, wxEXPAND | wxALL, 5);
        wxButton* closeBtn = new wxButton(&dlg, wxID_OK, "Close");
        sizer->Add(closeBtn, 0, wxALIGN_CENTER | wxALL, 5);
        dlg.SetSizer(sizer);
        dlg.ShowModal();
    }

    // --- Hint / cheat ------------------------------------------------

    int GetCorrectValue(int row, int col) {
        Sudoku copy;
        std::memcpy(copy.board, sudoku.NativeSudoku()->board, sizeof(copy.board));
        copy.Solve();
        return copy.GetValue(row, col);
    }

    void HintCell() {
        Sudoku copy;
        std::memcpy(copy.board, sudoku.NativeSudoku()->board, sizeof(copy.board));
        copy.Solve();

        std::vector<std::pair<int,int>> emptyCells;
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (sudoku.GetValue(i, j) == -1) emptyCells.emplace_back(i, j);

        if (emptyCells.empty()) { UpdateStatus("No empty cells to fill"); return; }

        std::mt19937 rng(std::random_device{}());
        auto [row, col] = emptyCells[rng() % emptyCells.size()];
        int solvedVal = copy.GetValue(row, col);
        if (solvedVal == -1) { UpdateStatus("Cheat: Could not solve current board"); return; }

        sudoku.SaveBoardState();
        sudoku.SetValue(row, col, solvedVal);
        UpdateGrid();
        UpdateStatus(wxString::Format("Cheat: placed %d at row %d, col %d", solvedVal + 1, row + 1, col + 1));
    }

    // --- Algorithms (run synchronously; see README for why these aren't
    // backgrounded like the original's threaded solver calls) -----------

    void RunAlgorithm(const wxString& label, std::function<void()> fn) {
        if (!sudoku.IsValidSolution()) { UpdateStatus("Current board is invalid"); return; }
        ClearDebugBox();
        fn();
        UpdateGrid();
        UpdateStatus(label + " completed");
    }

    // --- Event handlers ----------------------------------------------

    void OnMenu(wxCommandEvent& evt) {
        int id = evt.GetId();
        switch (id) {
            case ID_GenEasy: GeneratePuzzle("easy", "easy", true); return;
            case ID_GenMedium: GeneratePuzzle("medium", "medium", true); return;
            case ID_GenHard: GeneratePuzzle("hard", "hard", true); return;
            case ID_GenMaster: GeneratePuzzle("extreme", "master", true); return;
            case ID_GenExpert: GeneratePuzzle("expert", "expert", true); return;
            case ID_ViewHighscores: ShowHighscoresDialog(""); return;
            case wxID_EXIT: Close(true); return;
            case ID_ColorblindMode: {
                colorblindMode = evt.IsChecked();
                ValidateAndHighlight();
                return;
            }
            case wxID_ABOUT: ShowAbout(); return;
            case ID_SupportAuthor: ShowSupport(); return;
            case ID_ToggleNotes:
                board->SetNotesVisible(!board->GetNotesVisible());
                UpdateStatus(board->GetNotesVisible() ? "Notes shown" : "Notes hidden");
                return;
            case ID_CopyBoard: DoCopyBoard(); return;
        }
        if (id >= ID_SaveSlot1 && id <= ID_SaveSlot4) { SaveSlot(id - ID_SaveSlot1 + 1); return; }
        if (id >= ID_LoadSlot1 && id <= ID_LoadSlot4) { LoadSlot(id - ID_LoadSlot1 + 1); return; }
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
        }
    }

    void SaveSlot(int slot) {
        wxString filename = GetSavePath(wxString::Format("sudoku_slot_%d.txt", slot));
        sudoku.SaveToFile(filename.ToStdString(), elapsedSeconds);
        UpdateStatus(wxString::Format("Game saved to slot %d", slot));
    }

    void LoadSlot(int slot) {
        wxString filename = GetSavePath(wxString::Format("sudoku_slot_%d.txt", slot));
        if (sudoku.LoadFromFile(filename.ToStdString())) {
            elapsedSeconds = sudoku.savedElapsedSeconds;
            UpdateGrid();
            UpdateStatus(wxString::Format("Game loaded from slot %d", slot));
        } else {
            UpdateStatus(wxString::Format("Failed to load slot %d", slot));
        }
    }

    void ShowAbout() {
        wxMessageBox(
            "Sudoku Game\n\n"
            "(C) 2026 Jason Brian Hall\n"
            "MIT License - https://opensource.org/licenses/MIT\n\n"
            "GitHub: https://github.com/jasonbrianhall/sudoku_solver\n\n"
            "A feature-rich Sudoku game with puzzle generation across five difficulty levels, "
            "real-time conflict detection, and a progressive locking system that rewards correct answers.\n\n"
            "Features:\n"
            "- Generate puzzles at 5 difficulty levels (Easy to Expert)\n"
            "- Real-time conflict detection and highlighting\n"
            "- Progressive cell locking - solve 5 correct in a row to earn green locks\n"
            "- Pencil mark notes for candidates\n"
            "- Game timer with save/load (timer persists across saves)\n"
            "- High score tracking per difficulty\n"
            "- Sound feedback for correct, incorrect, locks, and wins\n"
            "- Save/load games across 4 slots\n",
            "About Sudoku Game", wxOK | wxICON_INFORMATION, this);
    }

    void ShowSupport() {
        wxMessageBox(
            "If you enjoy Sudoku Game, please consider supporting the author!\n\n"
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

    void OnGameTimer(wxTimerEvent&) {
        elapsedSeconds++;
        SetStatusText(wxString::Format("%02d:%02d", elapsedSeconds / 60, elapsedSeconds % 60), 1);
    }

    void OnActivate(wxActivateEvent& evt) {
        if (!evt.GetActive()) {
            if (!timerPaused) { timerPaused = true; gameTimer.Stop(); UpdateStatus("Game paused"); }
        } else {
            if (timerPaused && !puzzleSolved) { timerPaused = false; gameTimer.Start(1000); UpdateStatus("Game resumed"); }
        }
        evt.Skip();
    }

    void OnCellFocus(int row, int col) {
        highlightValue = sudoku.GetValue(row, col);
        ValidateAndHighlight();
    }
    void OnCellLostFocus(int, int) {
        highlightValue = -1;
        ValidateAndHighlight();
    }

    void OnCellMouseWheel(int row, int col, wxMouseEvent& e) {
        int currentValue = sudoku.GetValue(row, col);
        if (e.GetWheelRotation() > 0) currentValue = (currentValue + 1) % 10;
        else { currentValue--; if (currentValue < -1) currentValue = 8; }
        if (currentValue == -1) sudoku.ClearValue(row, col);
        else { sudoku.Clean(); sudoku.SetValue(row, col, currentValue); }
        board->SetCellValue(row, col, currentValue);
    }

    void OnCellMouseDown(int row, int col, wxMouseEvent& e) {
        if (e.MiddleDown()) {
            if (sudoku.IsCellLocked(row, col)) return;
            sudoku.ClearValue(row, col);
            board->SetCellValue(row, col, -1);
        }
    }

    void OnCellKeyDown(int row, int col, wxKeyEvent& e) {
        if (e.ControlDown() && e.GetKeyCode() == 'Z') {
            if (sudoku.CanUndo()) {
                sudoku.Undo();
                UpdateGrid();
                UpdateStatus(wxString::Format("Undo completed - %d states remaining", sudoku.GetUndoCount()));
            } else {
                UpdateStatus("Nothing to undo");
            }
            return;
        }

        int kc = e.GetKeyCode();
        switch (kc) {
            case WXK_LEFT:  board->FocusCell(row, col > 0 ? col - 1 : 8); return;
            case WXK_RIGHT: board->FocusCell(row, col < 8 ? col + 1 : 0); return;
            case WXK_UP:    board->FocusCell(row > 0 ? row - 1 : 8, col); return;
            case WXK_DOWN:  board->FocusCell(row < 8 ? row + 1 : 0, col); return;
            case WXK_F1: e.ShiftDown() ? GeneratePuzzle("extreme", "master", true) : GeneratePuzzle("easy", "easy", true); return;
            case WXK_F2: if (!e.ShiftDown()) GeneratePuzzle("medium", "medium", true); return;
            case WXK_F3: if (!e.ShiftDown()) GeneratePuzzle("hard", "hard", true); return;
            case WXK_F4: if (!e.ShiftDown()) GeneratePuzzle("expert", "expert", true); return;
            case WXK_F5: e.ShiftDown() ? LoadSlot(1) : SaveSlot(1); return;
            case WXK_F6: e.ShiftDown() ? LoadSlot(2) : SaveSlot(2); return;
            case WXK_F7: e.ShiftDown() ? LoadSlot(3) : SaveSlot(3); return;
            case WXK_F8: e.ShiftDown() ? LoadSlot(4) : SaveSlot(4); return;
            case WXK_F9:  if (!e.ShiftDown()) sudoku.ExportToExcelXML("puzzle1.xml"); return;
            case WXK_F10: if (!e.ShiftDown()) sudoku.ExportToExcelXML("puzzle2.xml"); return;
            case WXK_F11:
                if (e.ShiftDown()) RunAlgorithm("Full solve", [this]{ sudoku.Solve(); });
                else sudoku.ExportToExcelXML("puzzle3.xml");
                return;
            case WXK_F12:
                if (e.ShiftDown()) HintCell();
                else sudoku.ExportToExcelXML("puzzle4.xml");
                return;
        }

        if (kc >= '1' && kc <= '9') {
            if (sudoku.IsCellLocked(row, col)) { UpdateStatus("That cell is locked and cannot be changed"); return; }
            int enteredVal = kc - '1';
            int correctVal = GetCorrectValue(row, col);
            sudoku.SaveBoardState();
            sudoku.Clean();
            sudoku.SetValue(row, col, enteredVal);
            UpdateGrid();

            if (correctVal != -1 && enteredVal == correctVal) {
                bool alreadyQueued = false;
                for (auto& c : correctQueue) if (c.first == row && c.second == col) { alreadyQueued = true; break; }
                if (alreadyQueued) {
                    correctQueue.clear();
                    PlayWrongSound();
                } else {
                    correctQueue.emplace_back(row, col);
                    if (correctQueue.size() >= 5) {
                        auto toLock = correctQueue.front();
                        correctQueue.pop_front();
                        sudoku.SetQuasiImmutable(toLock.first, toLock.second);
                        UpdateGrid();
                        PlayQuasiImmutableSound();
                        UpdateStatus("Correct! Cell locked.");
                    } else {
                        PlayCorrectSound();
                    }
                }
            } else {
                PlayWrongSound();
                correctQueue.clear();
            }
            return;
        }

        if (kc == '0') {
            if (sudoku.IsCellLocked(row, col)) { UpdateStatus("That cell is locked and cannot be changed"); return; }
            correctQueue.clear();
            sudoku.SaveBoardState();
            sudoku.ClearValue(row, col);
            UpdateGrid();
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
                UpdateGrid();
                UpdateStatus("New game started");
                return;
            case 'T':
                board->SetNotesVisible(!board->GetNotesVisible());
                UpdateStatus(board->GetNotesVisible() ? "Notes shown" : "Notes hidden");
                return;
        }
    }
};

class GameApp : public wxApp {
public:
    bool OnInit() override {
        GameFrame* frame = new GameFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(GameApp);
