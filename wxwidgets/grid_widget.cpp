#include "grid_widget.h"
#include <array>
#include <utility>

SudokuGridWidget::SudokuGridWidget(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    BuildGrid();
    Bind(wxEVT_SIZE, &SudokuGridWidget::OnSize, this);
}

void SudokuGridWidget::BuildGrid() {
    // Black background shows through the small gaps we leave between
    // cells, which is what draws the grid lines (thicker at the 3x3 box
    // boundaries) without needing a pile of separate line panels.
    gridContainer = new wxPanel(this, wxID_ANY);
    gridContainer->SetBackgroundColour(*wxBLACK);

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            wxTextCtrl* cell = new wxTextCtrl(
                gridContainer, wxID_ANY, wxEmptyString,
                wxDefaultPosition, wxDefaultSize,
                wxTE_CENTRE | wxBORDER_NONE | wxTE_PROCESS_TAB);
            cell->SetMaxLength(1);
            cell->SetBackgroundColour(*wxWHITE);
            cellCtrl[row][col] = cell;

            wxTextCtrl* note = new wxTextCtrl(
                gridContainer, wxID_ANY, wxEmptyString,
                wxDefaultPosition, wxDefaultSize,
                wxTE_CENTRE | wxTE_MULTILINE | wxBORDER_SIMPLE);
            note->SetBackgroundColour(wxColour(245, 245, 245)); // WhiteSmoke
            note->Hide();
            noteCtrl[row][col] = note;

            // Suppress direct typing in the main cell - all digit entry
            // goes through onCellKeyDown so both apps can validate,
            // save undo state, play sounds, etc. before the value lands.
            cell->Bind(wxEVT_CHAR, [](wxKeyEvent&) { /* swallow */ });
            cell->Bind(wxEVT_KEY_DOWN, [this, row, col](wxKeyEvent& evt) {
                if (onCellKeyDown) onCellKeyDown(row, col, evt);
            });
            cell->Bind(wxEVT_MOUSEWHEEL, [this, row, col](wxMouseEvent& evt) {
                if (onCellMouseWheel) onCellMouseWheel(row, col, evt);
            });
            cell->Bind(wxEVT_LEFT_DOWN, [this, row, col](wxMouseEvent& evt) {
                if (onCellMouseDown) onCellMouseDown(row, col, evt);
                evt.Skip();
            });
            cell->Bind(wxEVT_MIDDLE_DOWN, [this, row, col](wxMouseEvent& evt) {
                if (onCellMouseDown) onCellMouseDown(row, col, evt);
            });
            cell->Bind(wxEVT_SET_FOCUS, [this, row, col](wxFocusEvent& evt) {
                if (onCellFocus) onCellFocus(row, col);
                evt.Skip();
            });
            cell->Bind(wxEVT_KILL_FOCUS, [this, row, col](wxFocusEvent& evt) {
                if (onCellLostFocus) onCellLostFocus(row, col);
                evt.Skip();
            });
            note->Bind(wxEVT_TEXT, [this, row, col](wxCommandEvent&) {
                if (onNotesChanged) onNotesChanged(row, col, noteCtrl[row][col]->GetValue());
            });
            // Arrow-key navigation should also work while a notes box has focus.
            note->Bind(wxEVT_KEY_DOWN, [this, row, col](wxKeyEvent& evt) {
                if (evt.GetKeyCode() == WXK_LEFT || evt.GetKeyCode() == WXK_RIGHT ||
                    evt.GetKeyCode() == WXK_UP || evt.GetKeyCode() == WXK_DOWN) {
                    if (onCellKeyDown) onCellKeyDown(row, col, evt);
                    return;
                }
                evt.Skip();
            });
        }
    }
}

void SudokuGridWidget::OnSize(wxSizeEvent& evt) {
    LayoutGrid();
    evt.Skip();
}

void SudokuGridWidget::LayoutGrid() {
    wxSize avail = GetClientSize();
    if (avail.GetWidth() < 50 || avail.GetHeight() < 50) return;

    // Maintain the original 6:7 (width:height) aspect ratio of the grid
    // (540x630 at the base 60x70 cell size) while fitting the available space.
    int gridWidth = avail.GetWidth();
    int gridHeight = (gridWidth * 7) / 6;
    if (gridHeight > avail.GetHeight()) {
        gridHeight = avail.GetHeight();
        gridWidth = (gridHeight * 6) / 7;
    }
    gridWidth = wxMax(gridWidth, 270);
    gridHeight = wxMax(gridHeight, 315);

    gridContainer->SetSize(gridWidth, gridHeight);
    gridContainer->SetPosition(wxPoint((avail.GetWidth() - gridWidth) / 2,
                                        (avail.GetHeight() - gridHeight) / 2));

    // Build cumulative pixel boundaries for the 9 columns/rows, leaving a
    // 1px gap between cells and a 3px gap (plus a 3px outer border) at
    // every 3x3 box boundary so the black gridContainer background shows
    // through as grid lines.
    auto buildBoundaries = [](int total) {
        int outer = 3, thin = 1, thick = 3;
        int thinCount = 6, thickCount = 2; // boundaries at 1,2,4,5,7,8 / 3,6
        int overhead = outer * 2 + thin * thinCount + thick * thickCount;
        int cellSize = wxMax(1, (total - overhead) / 9);
        std::array<int, 10> pos{};
        pos[0] = outer;
        for (int k = 1; k <= 9; k++) {
            int gap = (k == 9) ? 0 : ((k % 3 == 0) ? thick : thin);
            pos[k] = pos[k - 1] + cellSize + ((k < 9) ? 0 : 0);
            if (k < 9) pos[k] += gap; // gap sits after this cell, before next
        }
        return std::pair<std::array<int, 10>, int>(pos, cellSize);
    };

    auto [colPos, cellWidth] = buildBoundaries(gridWidth);
    auto [rowPos, cellHeight] = buildBoundaries(gridHeight);

    // Only reserve space for the notes strip when notes are actually
    // visible - otherwise leave the whole row to the main cell. (Reserving
    // it unconditionally left a visible black bar under every row, since
    // that space is empty container background while the notes box is
    // hidden.)
    int mainH = notesVisible ? (cellHeight * 45) / 70 : cellHeight;
    int notesH = cellHeight - mainH;
    float fontSize = wxMax(8.0f, mainH * 0.55f);
    float notesFontSize = wxMax(6.0f, notesH * 0.5f);
    wxFont cellFont(wxSize(0, (int)fontSize), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont noteFont(wxSize(0, (int)notesFontSize), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            wxTextCtrl* cell = cellCtrl[row][col];
            int x = colPos[col], y = rowPos[row];
            int w = colPos[col + 1] - x - ((col < 8) ? ((col + 1) % 3 == 0 ? 3 : 1) : 0);
            cell->SetSize(x, y, w, mainH);
            wxFont f = cell->GetFont();
            f.SetPointSize(wxMax(6, (int)fontSize));
            cell->SetFont(f);

            wxTextCtrl* note = noteCtrl[row][col];
            note->SetSize(x, y + mainH, w, notesH);
            wxFont nf = note->GetFont();
            nf.SetPointSize(wxMax(5, (int)notesFontSize));
            note->SetFont(nf);
        }
    }
}

void SudokuGridWidget::SetCellValue(int row, int col, int value) {
    wxString text = (value >= 0) ? wxString::Format("%d", value + 1) : wxString();
    if (cellCtrl[row][col]->GetValue() != text)
        cellCtrl[row][col]->ChangeValue(text);
}

int SudokuGridWidget::GetCellValue(int row, int col) const {
    wxString t = cellCtrl[row][col]->GetValue();
    long v;
    if (t.length() == 1 && t.ToLong(&v) && v >= 1 && v <= 9) return (int)v - 1;
    return -1;
}

void SudokuGridWidget::SetCellStyle(int row, int col, const wxColour& bg, const wxColour& fg,
                                     bool bold, bool italic, bool underline) {
    wxTextCtrl* cell = cellCtrl[row][col];
    cell->SetBackgroundColour(bg);
    cell->SetForegroundColour(fg);
    wxFont f = cell->GetFont();
    f.SetWeight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
    f.SetStyle(italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
    f.SetUnderlined(underline);
    cell->SetFont(f);
    cell->Refresh();
}

void SudokuGridWidget::SetCellReadOnly(int row, int col, bool readOnly) {
    // We never use wxTE_READONLY (it would block focus/navigation and the
    // key-down callback); "read only" for a locked cell is enforced by
    // the owning frame's key handler refusing to change the value.
    (void)row; (void)col; (void)readOnly;
}

void SudokuGridWidget::SetNotesText(int row, int col, const wxString& text) {
    noteCtrl[row][col]->ChangeValue(text);
}

wxString SudokuGridWidget::GetNotesText(int row, int col) const {
    return noteCtrl[row][col]->GetValue();
}

void SudokuGridWidget::ClearNotes(int row, int col) {
    noteCtrl[row][col]->ChangeValue(wxEmptyString);
}

void SudokuGridWidget::AddToNotes(int row, int col, const wxString& number) {
    wxString cur = noteCtrl[row][col]->GetValue();
    if (cur.Find(number) == wxNOT_FOUND) {
        noteCtrl[row][col]->ChangeValue(cur.IsEmpty() ? number : cur + " " + number);
    }
}

void SudokuGridWidget::SetNotesVisible(bool visible) {
    notesVisible = visible;
    for (int row = 0; row < 9; row++)
        for (int col = 0; col < 9; col++)
            noteCtrl[row][col]->Show(visible);
    LayoutGrid();
}

void SudokuGridWidget::FocusCell(int row, int col) {
    cellCtrl[row][col]->SetFocus();
}

bool SudokuGridWidget::IsCellFocused(int row, int col) const {
    return wxWindow::FindFocus() == cellCtrl[row][col];
}

void SudokuGridWidget::ReselectFocusedCell() {
    for (int row = 0; row < 9; row++)
        for (int col = 0; col < 9; col++)
            if (cellCtrl[row][col]->HasFocus())
                cellCtrl[row][col]->SelectAll();
}
