#ifndef GRID_WIDGET_H
#define GRID_WIDGET_H

#include <wx/wx.h>
#include <functional>

// The 9x9 grid of cells (each with a value box and a pencil-marks "notes"
// box below it), reused by both the Solver and the Game app. This owns
// only the widgets and their layout; all game-specific behaviour (what a
// keypress does, how a cell gets coloured) is supplied by the owning
// frame through the callbacks below - matching the way the original
// C++/CLI code kept a single MainForm class with grid[,] / notes[,]
// arrays, but as a reusable component instead of copy-pasted per app.
class SudokuGridWidget : public wxPanel {
public:
    explicit SudokuGridWidget(wxWindow* parent);

    // value: -1 for blank, 0-8 for a digit (displayed as 1-9).
    void SetCellValue(int row, int col, int value);
    int GetCellValue(int row, int col) const;

    void SetCellStyle(int row, int col, const wxColour& bg, const wxColour& fg,
                       bool bold, bool italic = false, bool underline = false);
    void SetCellReadOnly(int row, int col, bool readOnly);

    void SetNotesText(int row, int col, const wxString& text);
    wxString GetNotesText(int row, int col) const;
    void ClearNotes(int row, int col);
    void AddToNotes(int row, int col, const wxString& number);

    void SetNotesVisible(bool visible);
    bool GetNotesVisible() const { return notesVisible; }

    void FocusCell(int row, int col);
    bool IsCellFocused(int row, int col) const;
    // SelectAll() on whatever cell currently has focus (restores the
    // "block cursor" look after the text is programmatically replaced).
    void ReselectFocusedCell();

    wxTextCtrl* CellCtrl(int row, int col) const { return cellCtrl[row][col]; }

    // Callbacks the owning frame wires up. row/col are always in the
    // original (row, col) sense used throughout this codebase.
    std::function<void(int row, int col, wxKeyEvent&)> onCellKeyDown;
    std::function<void(int row, int col, wxMouseEvent&)> onCellMouseWheel;
    std::function<void(int row, int col, wxMouseEvent&)> onCellMouseDown;
    std::function<void(int row, int col)> onCellFocus;
    std::function<void(int row, int col)> onCellLostFocus;
    std::function<void(int row, int col, const wxString&)> onNotesChanged;

private:
    void BuildGrid();
    void LayoutGrid();
    void OnSize(wxSizeEvent& evt);

    wxTextCtrl* cellCtrl[9][9];
    wxTextCtrl* noteCtrl[9][9];
    wxPanel* gridContainer;
    bool notesVisible = false;
};

#endif // GRID_WIDGET_H
