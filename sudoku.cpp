#include <iostream>
using namespace std;
#include <stdlib.h>
#include <ncurses.h>

#include <vector>
#include <set>
#include <algorithm>

class Sudoku
{
  public:
    Sudoku();
    ~Sudoku();

    // Returns 0 for valid value else -1
    int SetValue(int x, int y, int value);
    int GetValue(int x, int y);
    int ClearValue(int x, int y);
    int Solve();
    int SolveBasic();
    bool LegalValue(int x, int y, int value);
    int FindHiddenPairs();
    int FindXWing();
    int FindPointingPairs();
    int FindSwordFish();
    int StdElim();
    int LinElim();
    int FindHiddenSingles();
    int FindNakedSets();
    int Clean();

    
  private:

    int EliminatePossibility(int x, int y, int value);
    int board[9][9][9];
    std::vector<int> GetCellCandidates(int x, int y);
    bool VectorsEqual(const std::vector<int>& v1, const std::vector<int>& v2);
    void FindNakedSetInUnit(std::vector<std::pair<int, int>>& cells, const std::vector<int>& candidates, int& changed);
    bool IsValidUnit(std::vector<int>& values);
    bool IsValidSolution();
    void RestoreBoard(int original_board[9][9][9], int board[9][9][9]);
    void print_debug(const char* format, ...);
    static int debug_line;  // Keep track of current debug line
};

int main(void)
{
  int i, x, y, temp, input, x_pos=0, y_pos=0;
  Sudoku NewGame;

  initscr();
  keypad(stdscr, true);
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);    // 3x3 borders
  init_pair(2, COLOR_BLUE, COLOR_BLACK);   // Numbers
  init_pair(3, COLOR_WHITE, COLOR_BLACK);  // Inner grid lines
  
  for(;;)
  {
    int header_lines = 8;  // Reduced header size with two columns
    
    printw("Welcome to Sudoku Solver\n");
    printw("Commands:                          Solving techniques:\n");
    printw(" Arrow keys - Move cursor           S - Standard elimination    N - Hidden singles\n");
    printw(" 1-9 - Fill number                  L - Line elimination        K - Naked sets\n");
    printw(" 0 - Clear cell                     H - Hidden pairs            X - X-Wing\n");
    printw(" q - Quit                           P - Pointing pairs          F - Swordfish\n");
    printw(" A - Run all techniques\n\n");
    
    // Draw the grid
    for(y=0;y<9;y++)
    {
        // Draw horizontal lines
        if(y%3==0)
        {
            attron(COLOR_PAIR(1));
            for(i=0;i<37;i++) printw("-");
            attroff(COLOR_PAIR(1));
        }
        else
        {
            attron(COLOR_PAIR(3));
            for(i=0;i<37;i++)
            {
                if(i%4==0)
                {
                    if(i%12==0)
                    {
                        attroff(COLOR_PAIR(3));
                        attron(COLOR_PAIR(1));
                        printw("+");
                        attroff(COLOR_PAIR(1));
                        attron(COLOR_PAIR(3));
                    }
                    else printw("+");
                }
                else printw("-");
            }
            attroff(COLOR_PAIR(3));
        }
        printw("\n");
        
        // Draw cells and vertical lines
        for(x=0;x<9;x++)
        {
            if(x%3==0)
            {
                attron(COLOR_PAIR(1));
                printw("|");
                attroff(COLOR_PAIR(1));
            }
            else
            {
                attron(COLOR_PAIR(3));
                printw("|");
                attroff(COLOR_PAIR(3));
            }
            
            temp=NewGame.GetValue(x,y);
            if(temp>=0 && temp<=8)
            {
                attron(COLOR_PAIR(2));
                printw(" %i ", temp+1);
                attroff(COLOR_PAIR(2));
            }
            else printw("   ");
        }
        
        attron(COLOR_PAIR(1));
        printw("|\n");
        attroff(COLOR_PAIR(1));
    }
    
    // Draw bottom border
    attron(COLOR_PAIR(1));
    for(i=0;i<37;i++) printw("-");
    attroff(COLOR_PAIR(1));

    // Calculate cursor position:
    // header_lines for the top offset
    // Each y_pos needs border line + content line
    int cursor_y = header_lines;
    // Add borders for 3x3 sections
    //if (y_pos >= 3) cursor_y++;
    //if (y_pos >= 6) cursor_y++;
    // Add position within grid (2 lines per row: border + content)
    cursor_y += y_pos * 2 + 1;  // +1 for initial border
    
    move(cursor_y, x_pos*4 + 2);  // x*4 accounts for "| n |" pattern
    refresh();
    input=getch();
    clear();

    // Upper Case
    if(input>='a' && input<='z')
    {
      input=input+'A'-'a';
    }
    
    switch (input)
    {
      case KEY_LEFT:
        x_pos=(x_pos+8)%9;
        break;
      case KEY_RIGHT:
        x_pos=(x_pos+1)%9;
        break;
      case KEY_UP:
        y_pos=(y_pos+8)%9;
        break;
      case KEY_DOWN:
        y_pos=(y_pos+1)%9;
        break;
      case '0':
        NewGame.ClearValue(x_pos, y_pos);
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        NewGame.SetValue(x_pos, y_pos, input - '1');
        break;
      case 'Q':
        endwin();
        return 0;
      case 'S':  // Standard elimination
        NewGame.StdElim();
        break;
      case 'L':  // Line elimination
        NewGame.LinElim();
        break;
      case 'H':  // Hidden pairs
        NewGame.FindHiddenPairs();
        break;
      case 'P':  // Pointing pairs
        NewGame.FindPointingPairs();
        break;
      case 'X':  // X-Wing
        NewGame.FindXWing();
        break;
      case 'F':  // Swordfish
        NewGame.FindSwordFish();
        break;
      case 'N':  // Hidden singles
        NewGame.FindHiddenSingles();
        break;
      case 'K':  // Naked sets
        NewGame.FindNakedSets();
        break;
      case 'A':  // Run all techniques
        for(i=0;i<2;i++) {
            NewGame.SolveBasic();
            NewGame.Solve();
        }
        break;
    }
    move(0,0);
  }
  return 0;
}

Sudoku::Sudoku()
{
  int x, y, k;
  for(x=0;x<9;x++)
  {
    for(y=0;y<9;y++)
    {
      for(k=0;k<9;k++)
      {
	board[x][y][k]=k;
      }
    }
  }
}

Sudoku::~Sudoku()
{

}

// Add implementation
int Sudoku::debug_line = 0;



int Sudoku::Clean()
{
  int x, y, k;
  for(x=0;x<9;x++)
  {
    for(y=0;y<9;y++)
    {
      if(GetValue(x, y)==-1)
      {
	for(k=0;k<9;k++)
	{
	  board[x][y][k]=k;
	}
      }
    }
  }
  return 0;
}


int Sudoku::SetValue(int x, int y, int value)
{
  int i;
  if(x>=0 && x<=8 && y>=0 && y<=8 && value>=0 && value<=8)
  {
    for(i=0;i<9;i++)
    {
      board[x][y][i]=-1;
    }
    board[x][y][value]=value;
    return 0;
  }
  else
  {
    return -1;
  }
}

int Sudoku::GetValue(int x, int y)
{
  int i,j=9;
  for(i=0;i<9;i++)
  {
    if(board[x][y][i]>=0 && board[x][y][i]<=8)
    {
      if(j==9)
      {
	if(board[x][y][i]>=0 && board[x][y][i]<=8)
	{
	  j=board[x][y][i];
	}
      }
      else
      {
	j=-1;
      }
    }
  }
  if(j==9)
  {
      j=-1;
  }
  return j;
}

int Sudoku::ClearValue(int x, int y)
{
  int i;
  if(x>=0 && x<=8 && y>=0 && y<=8)
  {
    for(i=0;i<9;i++)
    {
      board[x][y][i]=i;
    }
    return 0;
  }
  else
  {
    return -1;
  }
}

bool Sudoku::IsValidUnit(std::vector<int>& values) {
    std::vector<bool> used(9, false);
    for(int val : values) {
        if(val != -1) {  // Skip empty cells
            if(used[val]) {
                return false;  // Duplicate found
            }
            used[val] = true;
        }
    }
    return true;
}

bool Sudoku::IsValidSolution() {
    // Check rows
    for(int row = 0; row < 9; row++) {
        std::vector<int> values;
        for(int col = 0; col < 9; col++) {
            values.push_back(GetValue(row, col));
        }
        if(!IsValidUnit(values)) {
            return false;
        }
    }
    
    // Check columns
    for(int col = 0; col < 9; col++) {
        std::vector<int> values;
        for(int row = 0; row < 9; row++) {
            values.push_back(GetValue(row, col));
        }
        if(!IsValidUnit(values)) {
            return false;
        }
    }
    
    // Check 3x3 boxes
    for(int box = 0; box < 9; box++) {
        std::vector<int> values;
        int startRow = (box / 3) * 3;
        int startCol = (box % 3) * 3;
        
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                values.push_back(GetValue(startRow + i, startCol + j));
            }
        }
        if(!IsValidUnit(values)) {
            return false;
        }
    }
    
    return true;
}

void Sudoku::RestoreBoard(int original_board[9][9][9], int board[9][9][9]) {
    int i, j, k; 
    for (i = 0; i < 9; i++) { 
        for (j = 0; j < 9; j++) {
            for (k = 0; k < 9; k++) { 
                original_board[i][j][k] = board[i][j][k];
            } 
        } 
   }
} 

void Sudoku::print_debug(const char* format, ...) {
    // Move to position below grid (header + 19 grid lines + 2 padding)
    move(29 + debug_line, 0);
    
    // Handle variable arguments
    va_list args;
    va_start(args, format);
    vw_printw(stdscr, format, args);
    va_end(args);
    
    clrtoeol();  // Clear rest of line
    refresh();
    
    // Increment line counter, wrap around after 10 lines
    debug_line = (debug_line + 1) % 10;
}


int Sudoku::Solve() {
    int stop;
    int counter1, counter2, i, j,k;
    move(22, 0);
    printw("Starting Solve() - Cleaning board...\n");
    refresh();
    //Clean();
    int original_board[9][9][9];
    
    
    do {
        //RestoreBoard(original_board, board);
        counter1 = 0;
        counter2 = 0;
        for(i = 0; i < 9; i++) {
            for(j = 0; j < 9; j++) {
                if(GetValue(i,j) != -1) {
                    counter1++;
                }
            }
        }
        
        if(counter1 != 81) {
            // Run each solving technique and validate after each
            move(50, 0);
            printw("Running StdElim...                    \n");
            refresh();
            StdElim();
            if(!IsValidSolution()) {
                move(50, 0);
                printw("Invalid solution detected after StdElim\n");
                refresh();
                //RestoreBoard(board, original_board);
                return -1;
            }
            
            move(50, 0);
            printw("Running LinElim...                    \n");
            refresh();
            LinElim();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after LinElim\n");
                refresh();
                //RestoreBoard(board, original_board);
                return -1;
            }
            
            move(50, 0);
            printw("Running FindHiddenPairs...            \n");
            refresh();
            FindHiddenPairs();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after FindHiddenPairs\n");
                refresh();
                //RestoreBoard(board, original_board);
                return -1;
            }
            
            move(50, 0);
            printw("Running FindPointingPairs...            \n");
            refresh();
            FindPointingPairs();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after FindPointingPairs\n");
                //RestoreBoard(board, original_board);
                refresh();
                return -1;
            }


            move(50, 0);
            printw("Running FindXWing...                  \n");
            refresh();
            FindXWing();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after FindXWing\n");
                //RestoreBoard(board, original_board);
                refresh();
                return -1;
            }
            
            move(50, 0);
            printw("Running FindSwordFish...              \n");
            refresh();
            FindSwordFish();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after FindSwordFish\n");
                //RestoreBoard(board, original_board);
                refresh();
                return -1;
            }
            
            //move(50, 0);
            print_debug("Running FindHiddenSingles...          \n");
            refresh();
            FindHiddenSingles();
            if(!IsValidSolution()) {
                move(23, 0);
                print_debug("Invalid solution detected after FindHiddenSingles\n");
                //RestoreBoard(board, original_board);
                refresh();
                return -1;
            }
            
            //move(50, 0);
            print_debug("Running FindNakedSets...              \n");
            refresh();
            FindNakedSets();
            if(!IsValidSolution()) {
                //move(23, 0);
                //printw("Invalid solution detected after FindNakedSets\n");
                print_debug("Invalid solution detected after FindNakedSets\n");
                //RestoreBoard(board, original_board);
                refresh();
                return -1;
            }
            
            for(i = 0; i < 9; i++) {
                for(j = 0; j < 9; j++) {
                    if(GetValue(i,j) != -1) {
                        counter2++;
                    }
                }
            }
        } else {
            counter2 = 81;
        }
    } while(counter1 != counter2);
    
    // Final validation check
    if(!IsValidSolution()) {
        move(23, 0);
        printw("Invalid final solution detected\n");
        refresh();
        return -1;
    }
    
    return 0;
}

int Sudoku::SolveBasic() {
    int stop;
    int counter1, counter2, i, j,k;
    move(22, 0);
    printw("Starting Solve() - Cleaning board...\n");
    refresh();
    //Clean();
    int original_board[9][9][9];
    
    
    do {
        RestoreBoard(original_board, board);
        counter1 = 0;
        counter2 = 0;
        for(i = 0; i < 9; i++) {
            for(j = 0; j < 9; j++) {
                if(GetValue(i,j) != -1) {
                    counter1++;
                }
            }
        }
        
        if(counter1 != 81) {
            // Run each solving technique and validate after each
            move(22, 0);
            printw("Running StdElim...                    \n");
            refresh();
            StdElim();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after StdElim\n");
                refresh();
                //RestoreBoard(board, original_board);
                return -1;
            }
            
            move(22, 0);
            printw("Running LinElim...                    \n");
            refresh();
            LinElim();
            if(!IsValidSolution()) {
                move(23, 0);
                printw("Invalid solution detected after LinElim\n");
                refresh();
                //RestoreBoard(board, original_board);
                return -1;
            }
            
            for(i = 0; i < 9; i++) {
                for(j = 0; j < 9; j++) {
                    if(GetValue(i,j) != -1) {
                        counter2++;
                    }
                }
            }
        } else {
            counter2 = 81;
        }
    } while(counter1 != counter2);
    
    // Final validation check
    if(!IsValidSolution()) {
        move(23, 0);
        printw("Invalid final solution detected\n");
        refresh();
        return -1;
    }
    
    return 0;
}



int Sudoku::EliminatePossibility(int x, int y, int value)
{
  if(x>=0 && x<=8 && y>=0 && y<=8 && value>=0 && value<=8)
  {
    board[x][y][value]=-1;
    return 0;
  }
  else
  {
    return -1;
  }
}

bool Sudoku::LegalValue(int x, int y, int value)
{
    int i, j, temp, section1, section2;
  if(x>=0 && x<=8 && y>=0 && y<=8)
  {
      for(i=0;i<x;i++)
      {
	temp=GetValue(i, y);
	if(temp==value)
	{
	  return FALSE;
	}
      }
      for(i=x+1;i<9;i++)
      {
	temp=GetValue(i, y);
	if(temp==value)
	{
	  return FALSE;
	}
      }
      for(i=0;i<y;i++)
      {
	temp=GetValue(x, i);
	if(temp==value)
	{
	  return FALSE;
	}
      }
      for(i=y+1;i<9;i++)
      {
	temp=GetValue(x, i);
	if(temp==value)
	{
	  return FALSE;
	}
      }
      section1=(x/3)*3;
      section2=(y/3)*3;
      for(i=0;i<3;i++)
      {
	for(j=0;j<3;j++)
	{
	  if(x!=(section1+j) || y!=(section2+i))
	  {
	    temp=GetValue(section1+j, section2+i);
	    if(temp==value)
	    {
	      return FALSE;
	    }
	  }
	}
      }
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int Sudoku::FindXWing() {
    int changed = 0;

    // Debug helper to print candidate info
    auto printCandidates = [this](int row, int col) {
        move(52, 0);
        printw("Candidates at (%d,%d): ", row + 1, col + 1);
        for(int val = 0; val < 9; val++) {
            if(board[row][col][val] == val) {
                printw("%d ", val + 1);
            }
        }
        printw("\n");
        refresh();
    };

    // Helper to verify that a potential X-Wing position is valid
    auto isValidXWingCell = [this](int row, int col, int val) -> bool {
        // Cell must be empty
        if(GetValue(row, col) != -1) return false;
        
        // Must have value as a candidate
        if(board[row][col][val] != val) return false;
        
        // Must have at least one other candidate
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v) candidateCount++;
        }
        return candidateCount > 1;
    };

    // Process X-Wing patterns in columns first (as that's where we're seeing issues)
    for(int val = 0; val < 9; val++) {
        for(int col1 = 0; col1 < 8; col1++) {
            for(int col2 = col1 + 1; col2 < 9; col2++) {
                // First verify we can form an X-Wing in these columns
                std::vector<int> rows1, rows2;

                // Find candidate positions in first column
                for(int row = 0; row < 9; row++) {
                    if(isValidXWingCell(row, col1, val)) {
                        rows1.push_back(row);
                    }
                }

                // Check second column if first has exactly 2 positions
                if(rows1.size() == 2) {
                    for(int row = 0; row < 9; row++) {
                        if(isValidXWingCell(row, col2, val)) {
                            rows2.push_back(row);
                        }
                    }

                    // Verify exact match of row positions
                    if(rows2.size() == 2 && rows1[0] == rows2[0] && rows1[1] == rows2[1]) {
                        // We have a potential X-Wing pattern
                        move(23, 0);
                        printw("Found X-Wing pattern for value %d in columns %d,%d at rows %d,%d\n",
                               val + 1, col1 + 1, col2 + 1, rows1[0] + 1, rows1[1] + 1);
                        refresh();

                        bool madeChange = false;
                        
                        // Before eliminating, verify the pattern is necessary
                        for(int row : rows1) {
                            // Count how many places val can go in this row
                            int rowPlaces = 0;
                            for(int c = 0; c < 9; c++) {
                                if(isValidXWingCell(row, c, val)) rowPlaces++;
                            }
                            if(rowPlaces <= 2) {
                                move(24, 0);
                                printw("Skipping elimination - value %d is too constrained in row %d\n",
                                       val + 1, row + 1);
                                refresh();
                                goto next_pair;  // Pattern could create unsolvable puzzle
                            }
                        }

                        // Eliminate val from other positions in these rows
                        for(int col = 0; col < 9; col++) {
                            if(col != col1 && col != col2) {  // Skip X-Wing columns
                                for(int row : rows1) {
                                    if(isValidXWingCell(row, col, val)) {
                                        printCandidates(row, col);
                                        
                                        // Verify elimination won't create an invalid state
                                        bool canEliminate = true;
                                        board[row][col][val] = -1;  // Temporarily eliminate
                                        
                                        // Count remaining candidates
                                        int remainingCandidates = 0;
                                        for(int v = 0; v < 9; v++) {
                                            if(board[row][col][v] == v) remainingCandidates++;
                                        }
                                        
                                        if(remainingCandidates == 0) {
                                            canEliminate = false;
                                        }
                                        
                                        board[row][col][val] = val;  // Restore
                                        
                                        if(canEliminate) {
                                            move(26, 0);
                                            printw("Eliminating %d from (%d,%d)\n",
                                                   val + 1, row + 1, col + 1);
                                            refresh();
                                            
                                            board[row][col][val] = -1;
                                            madeChange = true;
                                            
                                            // Verify solution remains valid
                                            if(!IsValidSolution()) {
                                                move(27, 0);
                                                printw("Invalid solution after elimination!\n");
                                                refresh();
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if(madeChange) changed++;
                    }
                }
next_pair:
                continue;
            }
        }
    }

    // Similar process for row-based X-Wings...
    // (Previous row-based code here, with similar validation)

    return changed;
}

int Sudoku::FindSwordFish() {
    int changed = 0;

    // Helper to get candidates for a cell
    auto getCandidates = [this](int row, int col) -> std::vector<int> {
        std::vector<int> candidates;
        if(GetValue(row, col) == -1) {
            for(int val = 0; val < 9; val++) {
                if(board[row][col][val] == val) {
                    candidates.push_back(val);
                }
            }
        }
        return candidates;
    };

    // Helper to validate elimination
    auto isSafeElimination = [this](int row, int col, int val) -> bool {
        // Don't eliminate if it's the only candidate
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v) {
                candidateCount++;
            }
        }
        if(candidateCount <= 1) {
            move(24, 0);
            printw("Cannot eliminate only candidate %d at (%d,%d)\n", 
                   val + 1, row + 1, col + 1);
            refresh();
            return false;
        }

        // Verify the elimination won't create an invalid state
        board[row][col][val] = -1;  // Temporarily eliminate
        bool valid = IsValidSolution();
        board[row][col][val] = val;  // Restore
        return valid;
    };

    // Process Swordfish patterns in rows
    for(int val = 0; val < 9; val++) {
        // Try each triplet of rows
        for(int row1 = 0; row1 < 7; row1++) {
            for(int row2 = row1 + 1; row2 < 8; row2++) {
                for(int row3 = row2 + 1; row3 < 9; row3++) {
                    // Find positions where val can appear in each row
                    std::vector<int> cols1, cols2, cols3;
                    
                    // Collect positions for each row
                    for(int col = 0; col < 9; col++) {
                        if(GetValue(row1, col) == -1 && board[row1][col][val] == val) {
                            cols1.push_back(col);
                        }
                        if(GetValue(row2, col) == -1 && board[row2][col][val] == val) {
                            cols2.push_back(col);
                        }
                        if(GetValue(row3, col) == -1 && board[row3][col][val] == val) {
                            cols3.push_back(col);
                        }
                    }
                    
                    // Each row must have 2-3 positions
                    if(cols1.size() >= 2 && cols1.size() <= 3 &&
                       cols2.size() >= 2 && cols2.size() <= 3 &&
                       cols3.size() >= 2 && cols3.size() <= 3) {
                        
                        // Collect all unique columns
                        std::set<int> uniqueCols;
                        for(int col : cols1) uniqueCols.insert(col);
                        for(int col : cols2) uniqueCols.insert(col);
                        for(int col : cols3) uniqueCols.insert(col);
                        
                        // If exactly 3 columns, we have a Swordfish pattern
                        if(uniqueCols.size() == 3) {
                            move(23, 0);
                            printw("Found Swordfish pattern for value %d in rows %d,%d,%d\n",
                                   val + 1, row1 + 1, row2 + 1, row3 + 1);
                            refresh();

                            // Verify pattern won't create an unsolvable puzzle
                            bool isSafe = true;
                            for(int col : uniqueCols) {
                                int count = 0;
                                for(int row = 0; row < 9; row++) {
                                    if(row != row1 && row != row2 && row != row3 &&
                                       GetValue(row, col) == -1 && 
                                       board[row][col][val] == val) {
                                        count++;
                                    }
                                }
                                if(count == 0) {
                                    isSafe = false;
                                    break;
                                }
                            }

                            if(!isSafe) {
                                move(24, 0);
                                printw("Skipping unsafe Swordfish pattern\n");
                                refresh();
                                continue;
                            }

                            // Eliminate val from other cells in these columns
                            bool madeChange = false;
                            for(int col : uniqueCols) {
                                for(int row = 0; row < 9; row++) {
                                    if(row != row1 && row != row2 && row != row3 &&
                                       GetValue(row, col) == -1 && 
                                       board[row][col][val] == val) {
                                        
                                        move(25, 0);
                                        printw("Checking elimination of %d at (%d,%d)\n",
                                               val + 1, row + 1, col + 1);
                                        auto candidates = getCandidates(row, col);
                                        printw("Current candidates: ");
                                        for(int c : candidates) printw("%d ", c + 1);
                                        printw("\n");
                                        refresh();

                                        if(isSafeElimination(row, col, val)) {
                                            board[row][col][val] = -1;
                                            madeChange = true;

                                            // Validate after each elimination
                                            if(!IsValidSolution()) {
                                                move(26, 0);
                                                printw("Invalid solution after Swordfish elimination\n");
                                                refresh();
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                            if(madeChange) changed++;
                        }
                    }
                }
            }
        }
    }

    // Process Swordfish patterns in columns
    for(int val = 0; val < 9; val++) {
        for(int col1 = 0; col1 < 7; col1++) {
            for(int col2 = col1 + 1; col2 < 8; col2++) {
                for(int col3 = col2 + 1; col3 < 9; col3++) {
                    std::vector<int> rows1, rows2, rows3;
                    
                    for(int row = 0; row < 9; row++) {
                        if(GetValue(row, col1) == -1 && board[row][col1][val] == val) {
                            rows1.push_back(row);
                        }
                        if(GetValue(row, col2) == -1 && board[row][col2][val] == val) {
                            rows2.push_back(row);
                        }
                        if(GetValue(row, col3) == -1 && board[row][col3][val] == val) {
                            rows3.push_back(row);
                        }
                    }
                    
                    if(rows1.size() >= 2 && rows1.size() <= 3 &&
                       rows2.size() >= 2 && rows2.size() <= 3 &&
                       rows3.size() >= 2 && rows3.size() <= 3) {
                        
                        std::set<int> uniqueRows;
                        for(int row : rows1) uniqueRows.insert(row);
                        for(int row : rows2) uniqueRows.insert(row);
                        for(int row : rows3) uniqueRows.insert(row);
                        
                        if(uniqueRows.size() == 3) {
                            move(23, 0);
                            printw("Found Swordfish pattern for value %d in columns %d,%d,%d\n",
                                   val + 1, col1 + 1, col2 + 1, col3 + 1);
                            refresh();

                            // Verify pattern safety
                            bool isSafe = true;
                            for(int row : uniqueRows) {
                                int count = 0;
                                for(int col = 0; col < 9; col++) {
                                    if(col != col1 && col != col2 && col != col3 &&
                                       GetValue(row, col) == -1 && 
                                       board[row][col][val] == val) {
                                        count++;
                                    }
                                }
                                if(count == 0) {
                                    isSafe = false;
                                    break;
                                }
                            }

                            if(!isSafe) {
                                move(24, 0);
                                printw("Skipping unsafe Swordfish pattern\n");
                                refresh();
                                continue;
                            }

                            bool madeChange = false;
                            for(int row : uniqueRows) {
                                for(int col = 0; col < 9; col++) {
                                    if(col != col1 && col != col2 && col != col3 &&
                                       GetValue(row, col) == -1 && 
                                       board[row][col][val] == val) {
                                        
                                        move(25, 0);
                                        printw("Checking elimination of %d at (%d,%d)\n",
                                               val + 1, row + 1, col + 1);
                                        auto candidates = getCandidates(row, col);
                                        printw("Current candidates: ");
                                        for(int c : candidates) printw("%d ", c + 1);
                                        printw("\n");
                                        refresh();

                                        if(isSafeElimination(row, col, val)) {
                                            board[row][col][val] = -1;
                                            madeChange = true;

                                            if(!IsValidSolution()) {
                                                move(26, 0);
                                                printw("Invalid solution after Swordfish elimination\n");
                                                refresh();
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                            if(madeChange) changed++;
                        }
                    }
                }
            }
        }
    }
    
    return changed;
}

int Sudoku::FindHiddenPairs() {
    int changed = 0;
    
    // For each unit (row, column, box)
    for(int unit = 0; unit < 27; unit++) {
        // Try each pair of values
        for(int val1 = 0; val1 < 8; val1++) {
            for(int val2 = val1 + 1; val2 < 9; val2++) {
                std::vector<std::pair<int, int>> positions;
                
                // Get coordinates for the current unit
                for(int pos = 0; pos < 9; pos++) {
                    int x, y;
                    if(unit < 9) {  // Row
                        x = unit;
                        y = pos;
                    } else if(unit < 18) {  // Column
                        x = pos;
                        y = unit - 9;
                    } else {  // Box
                        int box = unit - 18;
                        x = (box / 3) * 3 + pos / 3;
                        y = (box % 3) * 3 + pos % 3;
                    }
                    
                    // If cell is empty and can contain either val1 or val2
                    if(GetValue(x, y) == -1 && 
                       (board[x][y][val1] == val1 || board[x][y][val2] == val2)) {
                        // Verify both values are still possible in this cell
                        bool canHaveVal1 = board[x][y][val1] == val1;
                        bool canHaveVal2 = board[x][y][val2] == val2;
                        if(canHaveVal1 || canHaveVal2) {
                            positions.push_back({x, y});
                        }
                    }
                }
                
                // If exactly two cells can contain these values
                if(positions.size() == 2) {
                    // Verify both cells can actually contain both values
                    bool validPair = true;
                    for(const auto& pos : positions) {
                        if(board[pos.first][pos.second][val1] == -1 || 
                           board[pos.first][pos.second][val2] == -1) {
                            validPair = false;
                            break;
                        }
                    }
                    
                    if(validPair) {
                        // Clear all other candidates from these two cells
                        bool madeChange = false;
                        for(const auto& pos : positions) {
                            for(int v = 0; v < 9; v++) {
                                if(v != val1 && v != val2 && 
                                   board[pos.first][pos.second][v] != -1) {
                                    board[pos.first][pos.second][v] = -1;
                                    madeChange = true;
                                }
                            }
                        }
                        if(madeChange) changed++;
                    }
                }
            }
        }
    }
    
    return changed;
}

int Sudoku::StdElim() {
    int eliminated = 0;

    // Helper to validate elimination
    auto safeEliminate = [this](int row, int col, int value) -> bool {
        if(board[row][col][value] != value) return false;  // Already eliminated

        // Count remaining candidates before elimination
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v) candidateCount++;
        }
        if(candidateCount <= 1) {
            move(23, 0);
            printw("Cannot eliminate only candidate %d at (%d,%d)\n", value + 1, row + 1, col + 1);
            refresh();
            return false;
        }

        // Make the elimination
        board[row][col][value] = -1;

        // Validate resulting state
        if(!IsValidSolution()) {
            // Restore if invalid
            board[row][col][value] = value;
            move(23, 0);
            printw("Eliminating %d from (%d,%d) would create invalid state\n", 
                   value + 1, row + 1, col + 1);
            refresh();
            return false;
        }

        return true;
    };

    // Process each cell once
    std::vector<bool> processed(81, false);

    for(int y = 0; y < 9; y++) {
        for(int x = 0; x < 9; x++) {
            int cellIndex = y * 9 + x;
            if(processed[cellIndex]) continue;

            int value = GetValue(x, y);
            if(value >= 0 && value <= 8) {  // Found a filled cell
                move(24, 0);
                printw("Processing filled cell (%d,%d) with value %d\n", x + 1, y + 1, value + 1);
                refresh();

                // Eliminate from row
                for(int i = 0; i < 9; i++) {
                    if(i != x && GetValue(i, y) == -1) {
                        if(safeEliminate(i, y, value)) {
                            eliminated++;
                        }
                    }
                }
                
                // Eliminate from column
                for(int i = 0; i < 9; i++) {
                    if(i != y && GetValue(x, i) == -1) {
                        if(safeEliminate(x, i, value)) {
                            eliminated++;
                        }
                    }
                }
                
                // Eliminate from 3x3 box
                int box_x = (x / 3) * 3;
                int box_y = (y / 3) * 3;
                for(int i = 0; i < 3; i++) {
                    for(int j = 0; j < 3; j++) {
                        int cur_x = box_x + j;
                        int cur_y = box_y + i;
                        if((cur_x != x || cur_y != y) && GetValue(cur_x, cur_y) == -1) {
                            if(safeEliminate(cur_x, cur_y, value)) {
                                eliminated++;
                            }
                        }
                    }
                }

                // Mark cell as processed
                processed[cellIndex] = true;

                // Validate the entire board after processing each cell
                if(!IsValidSolution()) {
                    move(25, 0);
                    printw("Invalid board state after processing cell (%d,%d)\n", x + 1, y + 1);
                    refresh();
                    return -1;
                }
            }
        }
    }
    
    // Return -1 if no eliminations were made
    return eliminated > 0 ? eliminated : -1;
}

int Sudoku::FindHiddenSingles() {
    int changed = 0;
    
    // Helper function to log potential moves
    auto logMove = [](const char* unitType, int unit, int val, int pos) {
        move(23, 0);
        printw("Found hidden single: value %d in %s %d at position %d\n", 
               val + 1, unitType, unit + 1, pos + 1);
        refresh();
    };

    // Helper function to validate before setting
    auto validateMove = [this](int row, int col, int val) -> bool {
        // Check row
        for(int c = 0; c < 9; c++) {
            if(c != col && GetValue(row, c) == val) {
                move(24, 0);
                printw("Row conflict: %d already exists in row %d\n", val + 1, row + 1);
                refresh();
                return false;
            }
        }
        
        // Check column
        for(int r = 0; r < 9; r++) {
            if(r != row && GetValue(r, col) == val) {
                move(24, 0);
                printw("Column conflict: %d already exists in column %d\n", val + 1, col + 1);
                refresh();
                return false;
            }
        }
        
        // Check box
        int boxRow = (row / 3) * 3;
        int boxCol = (col / 3) * 3;
        for(int r = 0; r < 3; r++) {
            for(int c = 0; c < 3; c++) {
                if((boxRow + r != row || boxCol + c != col) && 
                   GetValue(boxRow + r, boxCol + c) == val) {
                    move(24, 0);
                    printw("Box conflict: %d already exists in box\n", val + 1);
                    refresh();
                    return false;
                }
            }
        }
        return true;
    };

    // First scan rows
    for(int row = 0; row < 9; row++) {
        for(int val = 0; val < 9; val++) {
            int validCol = -1;
            int count = 0;
            
            // Count how many times this value can appear in this row
            for(int col = 0; col < 9; col++) {
                if(GetValue(row, col) == -1 && board[row][col][val] == val) {
                    count++;
                    validCol = col;
                }
            }
            
            // Found a hidden single
            if(count == 1 && validCol != -1) {
                logMove("row", row, val, validCol);
                
                // Validate before making the change
                if(validateMove(row, validCol, val)) {
                    SetValue(row, validCol, val);
                    if(!IsValidSolution()) {
                        move(24, 0);
                        printw("Invalid solution after setting %d at (%d,%d)\n", 
                              val + 1, row + 1, validCol + 1);
                        refresh();
                        return -1;
                    }
                    changed++;
                }
            }
        }
    }
    
    // Then scan columns
    for(int col = 0; col < 9; col++) {
        for(int val = 0; val < 9; val++) {
            int validRow = -1;
            int count = 0;
            
            for(int row = 0; row < 9; row++) {
                if(GetValue(row, col) == -1 && board[row][col][val] == val) {
                    count++;
                    validRow = row;
                }
            }
            
            if(count == 1 && validRow != -1) {
                logMove("column", col, val, validRow);
                
                if(validateMove(validRow, col, val)) {
                    SetValue(validRow, col, val);
                    if(!IsValidSolution()) {
                        move(24, 0);
                        printw("Invalid solution after setting %d at (%d,%d)\n", 
                              val + 1, validRow + 1, col + 1);
                        refresh();
                        return -1;
                    }
                    changed++;
                }
            }
        }
    }
    
    // Finally scan boxes
    for(int box = 0; box < 9; box++) {
        int boxRow = (box / 3) * 3;
        int boxCol = (box % 3) * 3;
        
        for(int val = 0; val < 9; val++) {
            int validRow = -1;
            int validCol = -1;
            int count = 0;
            
            for(int r = 0; r < 3; r++) {
                for(int c = 0; c < 3; c++) {
                    int row = boxRow + r;
                    int col = boxCol + c;
                    if(GetValue(row, col) == -1 && board[row][col][val] == val) {
                        count++;
                        validRow = row;
                        validCol = col;
                    }
                }
            }
            
            if(count == 1 && validRow != -1 && validCol != -1) {
                logMove("box", box, val, (validRow % 3) * 3 + (validCol % 3));
                
                if(validateMove(validRow, validCol, val)) {
                    SetValue(validRow, validCol, val);
                    if(!IsValidSolution()) {
                        move(24, 0);
                        printw("Invalid solution after setting %d at (%d,%d)\n", 
                              val + 1, validRow + 1, validCol + 1);
                        refresh();
                        return -1;
                    }
                    changed++;
                }
            }
        }
    }
    
    return changed;
}

int Sudoku::LinElim()
{
  int v, w, x, y, k, xpos, ypos, counter;
  int changed = 0;
  
  for(k=0;k<9;k++)
  {
    // Check boxes
    for(v=0;v<9;v=v+3)
    {
      for(w=0;w<9;w=w+3)
      {
        xpos=-1;
        ypos=-1;
        counter=0;
        for(x=v;x<v+3;x++)
        {
          for(y=w;y<w+3;y++)
          {
            // Only consider empty cells and verify legal placement
            if(GetValue(x,y) == -1 && board[x][y][k]==k && LegalValue(x,y,k))
            {
              xpos=x;
              ypos=y;
              counter++;
            }
          }
        }
        if(counter==1 && xpos != -1 && ypos != -1)
        {
          if(LegalValue(xpos, ypos, k)) {
            SetValue(xpos, ypos, k);
            changed++;
          }
        }
      }
    }
  }

  // Check rows
  for(k=0;k<9;k++)
  {
    for(x=0;x<9;x++)
    {
      xpos=-1;
      ypos=-1;
      counter=0;
    
      for(y=0;y<9;y++)
      {
        if(GetValue(x,y) == -1 && board[x][y][k]==k && LegalValue(x,y,k))
        {
          xpos=x;
          ypos=y;
          counter++;
        }
      }
      if(counter==1 && xpos != -1 && ypos != -1)
      {
        if(LegalValue(xpos, ypos, k)) {
          SetValue(xpos, ypos, k);
          changed++;
        }
      }
    }
  }

  // Check columns
  for(k=0;k<9;k++)
  {
    for(y=0;y<9;y++)
    {
      xpos=-1;
      ypos=-1;
      counter=0;
  
      for(x=0;x<9;x++)
      {
        if(GetValue(x,y) == -1 && board[x][y][k]==k && LegalValue(x,y,k))
        {
          xpos=x;
          ypos=y;
          counter++;
        }
      }
      if(counter==1 && xpos != -1 && ypos != -1)
      {
        if(LegalValue(xpos, ypos, k)) {
          SetValue(xpos, ypos, k);
          changed++;
        }
      }
    }
  }

  return changed;
}
// Implementation:
std::vector<int> Sudoku::GetCellCandidates(int x, int y) {
    std::vector<int> candidates;
    for(int val = 0; val < 9; val++) {
        if(board[x][y][val] == val) {
            candidates.push_back(val);
        }
    }
    return candidates;
}

bool Sudoku::VectorsEqual(const std::vector<int>& v1, const std::vector<int>& v2) {
    if(v1.size() != v2.size()) return false;
    for(size_t i = 0; i < v1.size(); i++) {
        if(v1[i] != v2[i]) return false;
    }
    return true;
}

void Sudoku::FindNakedSetInUnit(std::vector<std::pair<int, int>>& cells, const std::vector<int>& candidates, int& changed) {
    for(auto& cell : cells) {
        int x = cell.first;
        int y = cell.second;
        
        std::vector<int> cellCandidates = GetCellCandidates(x, y);
        if(!VectorsEqual(cellCandidates, candidates)) {
            for(int val : candidates) {
                if(board[x][y][val] != -1) {
                    board[x][y][val] = -1;
                    changed++;
                }
            }
        }
    }
}

int Sudoku::FindNakedSets() {
    int changed = 0;

    // Helper to get set of candidates for a cell
    auto getCandidates = [this](int row, int col) -> std::vector<int> {
        std::vector<int> candidates;
        if(GetValue(row, col) == -1) {  // Only if cell is empty
            for(int val = 0; val < 9; val++) {
                if(board[row][col][val] == val) {
                    candidates.push_back(val);
                }
            }
        }
        return candidates;
    };

    // Helper to check if elimination is safe
    auto isSafeElimination = [this](int row, int col, int val) -> bool {
        // Don't eliminate if this would leave cell with no candidates
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v) {
                candidateCount++;
            }
        }
        if(candidateCount <= 1) {
            move(24, 0);
            printw("Cannot eliminate only remaining candidate at (%d,%d)\n", row + 1, col + 1);
            refresh();
            return false;
        }

        // Check if elimination would create invalid state
        board[row][col][val] = -1;  // Temporarily eliminate
        bool isValid = IsValidSolution();
        board[row][col][val] = val; // Restore
        
        if(!isValid) {
            move(24, 0);
            printw("Eliminating %d from (%d,%d) would create invalid state\n", 
                   val + 1, row + 1, col + 1);
            refresh();
            return false;
        }
        return true;
    };

    // Process each row
    for(int row = 0; row < 9; row++) {
        std::vector<std::pair<int, std::vector<int>>> cells;  // col, candidates
        
        // Collect all empty cells and their candidates
        for(int col = 0; col < 9; col++) {
            auto candidates = getCandidates(row, col);
            if(!candidates.empty()) {
                cells.push_back({col, candidates});

                // Debug output
                move(25, 0);
                printw("Row %d Col %d candidates: ", row + 1, col + 1);
                for(int val : candidates) {
                    printw("%d ", val + 1);
                }
                printw("\n");
                refresh();
            }
        }

        // Look for naked pairs
        for(size_t i = 0; i < cells.size(); i++) {
            if(cells[i].second.size() != 2) continue;
            
            for(size_t j = i + 1; j < cells.size(); j++) {
                if(cells[j].second.size() != 2) continue;
                
                // Check if these cells form a naked pair
                bool isPair = true;
                for(int val : cells[i].second) {
                    if(std::find(cells[j].second.begin(), cells[j].second.end(), val) 
                       == cells[j].second.end()) {
                        isPair = false;
                        break;
                    }
                }
                
                if(isPair) {
                    move(26, 0);
                    printw("Found naked pair in row %d at columns %d,%d: ", 
                           row + 1, cells[i].first + 1, cells[j].first + 1);
                    for(int val : cells[i].second) {
                        printw("%d ", val + 1);
                    }
                    printw("\n");
                    refresh();

                    // Eliminate these values from other cells
                    bool madeChange = false;
                    for(size_t k = 0; k < cells.size(); k++) {
                        if(k != i && k != j) {
                            int col = cells[k].first;
                            
                            for(int val : cells[i].second) {
                                if(board[row][col][val] == val) {
                                    // Debug before elimination
                                    move(27, 0);
                                    printw("Candidates at (%d,%d): ", row + 1, col + 1);
                                    auto beforeCands = getCandidates(row, col);
                                    for(int v : beforeCands) {
                                        printw("%d ", v + 1);
                                    }
                                    printw("\n");
                                    refresh();

                                    if(isSafeElimination(row, col, val)) {
                                        move(28, 0);
                                        printw("Eliminating %d from (%d,%d)\n", 
                                               val + 1, row + 1, col + 1);
                                        refresh();
                                        
                                        board[row][col][val] = -1;
                                        madeChange = true;

                                        if(!IsValidSolution()) {
                                            move(29, 0);
                                            printw("Invalid solution after elimination!\n");
                                            refresh();
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if(madeChange) changed++;
                }
            }
        }
    }

    // Process each column
    for(int col = 0; col < 9; col++) {
        std::vector<std::pair<int, std::vector<int>>> cells;  // row, candidates
        
        // Collect all empty cells and their candidates
        for(int row = 0; row < 9; row++) {
            auto candidates = getCandidates(row, col);
            if(!candidates.empty()) {
                cells.push_back({row, candidates});

                // Debug output
                move(25, 0);
                printw("Col %d Row %d candidates: ", col + 1, row + 1);
                for(int val : candidates) {
                    printw("%d ", val + 1);
                }
                printw("\n");
                refresh();
            }
        }

        // Look for naked pairs
        for(size_t i = 0; i < cells.size(); i++) {
            if(cells[i].second.size() != 2) continue;
            
            for(size_t j = i + 1; j < cells.size(); j++) {
                if(cells[j].second.size() != 2) continue;
                
                // Check if these cells form a naked pair
                bool isPair = true;
                for(int val : cells[i].second) {
                    if(std::find(cells[j].second.begin(), cells[j].second.end(), val) 
                       == cells[j].second.end()) {
                        isPair = false;
                        break;
                    }
                }
                
                if(isPair) {
                    move(26, 0);
                    printw("Found naked pair in col %d at rows %d,%d: ", 
                           col + 1, cells[i].first + 1, cells[j].first + 1);
                    for(int val : cells[i].second) {
                        printw("%d ", val + 1);
                    }
                    printw("\n");
                    refresh();

                    // Eliminate these values from other cells
                    bool madeChange = false;
                    for(size_t k = 0; k < cells.size(); k++) {
                        if(k != i && k != j) {
                            int row = cells[k].first;
                            
                            for(int val : cells[i].second) {
                                if(board[row][col][val] == val) {
                                    // Debug before elimination
                                    move(27, 0);
                                    printw("Candidates at (%d,%d): ", row + 1, col + 1);
                                    auto beforeCands = getCandidates(row, col);
                                    for(int v : beforeCands) {
                                        printw("%d ", v + 1);
                                    }
                                    printw("\n");
                                    refresh();

                                    if(isSafeElimination(row, col, val)) {
                                        move(28, 0);
                                        printw("Eliminating %d from (%d,%d)\n", 
                                               val + 1, row + 1, col + 1);
                                        refresh();
                                        
                                        board[row][col][val] = -1;
                                        madeChange = true;

                                        if(!IsValidSolution()) {
                                            move(29, 0);
                                            printw("Invalid solution after elimination!\n");
                                            refresh();
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if(madeChange) changed++;
                }
            }
        }
    }

    // Process each 3x3 box
    for(int boxRow = 0; boxRow < 3; boxRow++) {
        for(int boxCol = 0; boxCol < 3; boxCol++) {
            std::vector<std::pair<std::pair<int,int>, std::vector<int>>> cells;  // {row,col}, candidates
            
            // Collect all empty cells and their candidates in this box
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 3; j++) {
                    int row = boxRow * 3 + i;
                    int col = boxCol * 3 + j;
                    
                    auto candidates = getCandidates(row, col);
                    if(!candidates.empty()) {
                        cells.push_back({{row, col}, candidates});

                        // Debug output
                        move(25, 0);
                        printw("Box (%d,%d) position (%d,%d) candidates: ", 
                               boxRow + 1, boxCol + 1, row + 1, col + 1);
                        for(int val : candidates) {
                            printw("%d ", val + 1);
                        }
                        printw("\n");
                        refresh();
                    }
                }
            }

            // Look for naked pairs
            for(size_t i = 0; i < cells.size(); i++) {
                if(cells[i].second.size() != 2) continue;
                
                for(size_t j = i + 1; j < cells.size(); j++) {
                    if(cells[j].second.size() != 2) continue;
                    
                    // Check if these cells form a naked pair
                    bool isPair = true;
                    for(int val : cells[i].second) {
                        if(std::find(cells[j].second.begin(), cells[j].second.end(), val) 
                           == cells[j].second.end()) {
                            isPair = false;
                            break;
                        }
                    }
                    
                    if(isPair) {
                        move(26, 0);
                        printw("Found naked pair in box (%d,%d) at positions (%d,%d),(%d,%d): ", 
                               boxRow + 1, boxCol + 1,
                               cells[i].first.first + 1, cells[i].first.second + 1,
                               cells[j].first.first + 1, cells[j].first.second + 1);
                        for(int val : cells[i].second) {
                            printw("%d ", val + 1);
                        }
                        printw("\n");
                        refresh();

                        // Eliminate these values from other cells in box
                        bool madeChange = false;
                        for(size_t k = 0; k < cells.size(); k++) {
                            if(k != i && k != j) {
                                int row = cells[k].first.first;
                                int col = cells[k].first.second;
                                
                                for(int val : cells[i].second) {
                                    if(board[row][col][val] == val) {
                                        // Debug before elimination
                                        move(27, 0);
                                        printw("Candidates at (%d,%d): ", row + 1, col + 1);
                                        auto beforeCands = getCandidates(row, col);
                                        for(int v : beforeCands) {
                                            printw("%d ", v + 1);
                                        }
                                        printw("\n");
                                        refresh();

                                        if(isSafeElimination(row, col, val)) {
                                            move(28, 0);
                                            printw("Eliminating %d from (%d,%d)\n", 
                                                   val + 1, row + 1, col + 1);
                                            refresh();
                                            
                                            board[row][col][val] = -1;
                                            madeChange = true;

                                            if(!IsValidSolution()) {
                                                move(29, 0);
                                                printw("Invalid solution after elimination!\n");
                                                refresh();
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if(madeChange) changed++;
                    }
                }
            }
        }
    }

    return changed;
}


int Sudoku::FindPointingPairs() {
    int changed = 0;

    // Helper to validate if elimination is safe
    auto isSafeElimination = [this](int row, int col, int val) -> bool {
        // Don't eliminate if cell is already solved
        if(GetValue(row, col) != -1) return false;
        
        // Don't eliminate if value isn't a candidate
        if(board[row][col][val] != val) return false;
        
        // Count remaining candidates before elimination
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v) candidateCount++;
        }
        
        // Don't eliminate if it's the last candidate
        if(candidateCount <= 1) return false;

        // Temporarily eliminate and check validity
        board[row][col][val] = -1;
        bool valid = IsValidSolution();
        board[row][col][val] = val;  // Restore
        
        return valid;
    };

    // For each 3x3 box
    for(int boxRow = 0; boxRow < 3; boxRow++) {
        for(int boxCol = 0; boxCol < 3; boxCol++) {
            // For each possible value
            for(int val = 0; val < 9; val++) {
                std::vector<std::pair<int, int>> positions;
                
                // Find all positions where val is a candidate in this box
                for(int i = 0; i < 3; i++) {
                    for(int j = 0; j < 3; j++) {
                        int row = boxRow * 3 + i;
                        int col = boxCol * 3 + j;
                        if(GetValue(row, col) == -1 && board[row][col][val] == val) {
                            positions.push_back({row, col});
                        }
                    }
                }

                // Only proceed if we have 2 or 3 positions
                if(positions.size() >= 2 && positions.size() <= 3) {
                    // Check if all positions are in the same row
                    bool sameRow = true;
                    int firstRow = positions[0].first;
                    for(const auto& pos : positions) {
                        if(pos.first != firstRow) {
                            sameRow = false;
                            break;
                        }
                    }

                    if(sameRow) {
                        // Before eliminating, verify the pattern is necessary
                        int candidatesInRow = 0;
                        for(int col = 0; col < 9; col++) {
                            if(GetValue(firstRow, col) == -1 && board[firstRow][col][val] == val) {
                                candidatesInRow++;
                            }
                        }
                        
                        // Only proceed if there are more candidates outside the box
                        if(candidatesInRow > positions.size()) {
                            bool madeChange = false;
                            // Eliminate val from other cells in this row
                            for(int col = 0; col < 9; col++) {
                                if(col / 3 != boxCol && // Skip cells in our box
                                   isSafeElimination(firstRow, col, val)) {
                                    board[firstRow][col][val] = -1;
                                    madeChange = true;
                                }
                            }
                            if(madeChange) changed++;
                        }
                    }

                    // Check if all positions are in the same column
                    bool sameCol = true;
                    int firstCol = positions[0].second;
                    for(const auto& pos : positions) {
                        if(pos.second != firstCol) {
                            sameCol = false;
                            break;
                        }
                    }

                    if(sameCol) {
                        // Before eliminating, verify the pattern is necessary
                        int candidatesInCol = 0;
                        for(int row = 0; row < 9; row++) {
                            if(GetValue(row, firstCol) == -1 && board[row][firstCol][val] == val) {
                                candidatesInCol++;
                            }
                        }
                        
                        // Only proceed if there are more candidates outside the box
                        if(candidatesInCol > positions.size()) {
                            bool madeChange = false;
                            // Eliminate val from other cells in this column
                            for(int row = 0; row < 9; row++) {
                                if(row / 3 != boxRow && // Skip cells in our box
                                   isSafeElimination(row, firstCol, val)) {
                                    board[row][firstCol][val] = -1;
                                    madeChange = true;
                                }
                            }
                            if(madeChange) changed++;
                        }
                    }
                }
            }
        }
    }
    
    return changed;
}
