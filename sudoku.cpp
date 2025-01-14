#include <iostream>
using namespace std;
#include <stdlib.h>
#include <ncurses.h>

#include <vector>
#include <set>
#include <algorithm>

#include <fstream>
#include <ctime>
#include <cstring>

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
    int FindXYWing();
    int FindXYZWing();
    int FindSimpleColoring();
    int FindPointingPairs();
    int FindSwordFish();
    int StdElim();
    int LinElim();
    int FindHiddenSingles();
    int FindNakedSets();
    int Clean();
    void LogBoard(std::ofstream& file, const char* algorithm_name);
    void NewGame();
    bool LoadFromFile(const string& filename);
    void SaveToFile(const string& filename);
    void print_debug(const char* format, ...);

    
  private:

    int EliminatePossibility(int x, int y, int value);
    int board[9][9][9];
    std::vector<int> GetCellCandidates(int x, int y);
    bool VectorsEqual(const std::vector<int>& v1, const std::vector<int>& v2);
    void FindNakedSetInUnit(std::vector<std::pair<int, int>>& cells, const std::vector<int>& candidates, int& changed);
    bool IsValidUnit(std::vector<int>& values);
    bool IsValidSolution();
    void RestoreBoard(int original_board[9][9][9], int board[9][9][9]);
    static int debug_line;  // Keep track of current debug line
};

void print_usage() {
    cout << "Usage:" << endl;
    cout << "  sudoku                     - Run in interactive mode" << endl;
    cout << "  sudoku -f <input_file>     - Load and solve puzzle from file" << endl;
    cout << "  sudoku -f <input_file> -o <output_file>  - Load, solve, and save to file" << endl;
}

int main(int argc, char* argv[]) {
 
    int i, x, y, temp, input, x_pos=0, y_pos=0;
    Sudoku NewGame;
    std::ofstream logfile("sudoku_progress.txt", std::ios::app);
   
    // Parse command line arguments
    string input_file = "";
    string output_file = "";
    bool has_output = false;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage();
            return 0;
        }
        else if (arg == "-f" && i + 1 < argc) {
            input_file = argv[++i];
        }
        else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
            has_output = true;
        }
    }
    
    // If input file specified, run in command line mode
    if (!input_file.empty()) {
        if (!NewGame.LoadFromFile(input_file)) {
            cerr << "Failed to load puzzle from " << input_file << endl;
            return 1;
        }
        
    }
 
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
    printw("Commands:                          Solving techniques:                                Experimental Techniques (not part of the all chain)\n");
    printw(" Arrow keys - Move cursor           S - Standard elimination    N - Hidden singles     Y - Find XY Wing\n");
    printw(" 1-9 - Fill number                  L - Line elimination        K - Naked sets         ; - Find XYZ Wing\n");  
    printw(" 0 - Clear cell                     H - Hidden pairs            X - X-Wing             C - Simple Coloring\n"); 
    printw(" q - Quit                           P - Pointing pairs          F - Swordfish\n");
    printw(" A - Run all techniques             Z - New Game                F(5-8) - Save Game     Shift F(5-8) - Load Game\n\n");
    
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
        NewGame.LogBoard(logfile, "Standard Elim Before");
        NewGame.StdElim();
        NewGame.LogBoard(logfile, "Standard Elim After");
        break;
      case 'L':  // Line elimination
        NewGame.LogBoard(logfile, "Line Elim Before");
        NewGame.LinElim();
        NewGame.LogBoard(logfile, "Line Elim After");
        break;
      case 'C':  // Standard elimination
        NewGame.LogBoard(logfile, "Find Simple Coloring Before");
        NewGame.FindSimpleColoring();
        NewGame.LogBoard(logfile, "Find Simple Coloring After");
        break;
      case 'H':  // Hidden pairs
        NewGame.LogBoard(logfile, "Find Hidden Pairs Before");
        NewGame.FindHiddenPairs();
        NewGame.LogBoard(logfile, "Find Hidden Pairs After");
        break;
      case 'P':  // Pointing pairs
        NewGame.LogBoard(logfile, "Find Pointing Pairs Before");
        NewGame.FindPointingPairs();
        NewGame.LogBoard(logfile, "Find Pointing Pairs After");
        break;
      case 'X':  // X-Wing
        NewGame.LogBoard(logfile, "Find XWING Before");
        NewGame.FindXWing();
        NewGame.LogBoard(logfile, "Find XWING After");
        break;
      case 'Y':  // XY-Wing (Broken)
        NewGame.LogBoard(logfile, "Find XYWING Before");
        NewGame.FindXYWing();
        NewGame.LogBoard(logfile, "Find XYWING After");
        break;
      case ';':  // XYZ-Wing (Broken)
        NewGame.LogBoard(logfile, "Find XYZWING Before");
        NewGame.FindXYZWing();
        NewGame.LogBoard(logfile, "Find XYZWING After");
        break;
      case 'F':  // Swordfish
        NewGame.LogBoard(logfile, "Find Swordfish Before");
        NewGame.FindSwordFish();
        NewGame.LogBoard(logfile, "Find Swordfish After");
        break;
      case 'N':  // Hidden singles
        NewGame.LogBoard(logfile, "Find Hidden Singles Before");
        NewGame.FindHiddenSingles();
        NewGame.LogBoard(logfile, "Find Hidden Singles After");

        break;
      case 'K':  // Naked sets
        NewGame.LogBoard(logfile, "Find Naked Sets Before");
        NewGame.FindNakedSets();
        NewGame.LogBoard(logfile, "Find Naked Sets After");

        break;
      case 'A':  // Run all techniques
        NewGame.LogBoard(logfile, "Run All Techniques Before");
        NewGame.Solve();
        NewGame.LogBoard(logfile, "Run All Techniques After");
        break;
      case 'Z':  // New Game
        NewGame.NewGame();
        break;
      case KEY_F(5):  // F5
        NewGame.SaveToFile("sudoku_1.txt");
        NewGame.print_debug("Game saved to sudoku_1.txt");
        break;
      case KEY_F(6):  // F6
        NewGame.SaveToFile("sudoku_2.txt");
        NewGame.print_debug("Game saved to sudoku_2.txt");
        break;
      case KEY_F(7):  // F7 
        NewGame.SaveToFile("sudoku_3.txt");
        NewGame.print_debug("Game saved to sudoku_3.txt");
        break;
      case KEY_F(8):  // F8
        NewGame.SaveToFile("sudoku_4.txt");
        NewGame.print_debug("Game saved to sudoku_4.txt");
        break;
      case KEY_F(17):  // Shift-F5 (KEY_F(5) + 12)
        if (NewGame.LoadFromFile("sudoku_1.txt")) {
            NewGame.print_debug("Game loaded from sudoku_1.txt");
        } else {
            NewGame.print_debug("Failed to load sudoku_1.txt");
        }
        break;
      case KEY_F(18):  // Shift-F6
        if (NewGame.LoadFromFile("sudoku_2.txt")) {
            NewGame.print_debug("Game loaded from sudoku_2.txt");
        } else {
            NewGame.print_debug("Failed to load sudoku_2.txt");
        }
        break;
      case KEY_F(19):  // Shift-F7
        if (NewGame.LoadFromFile("sudoku_3.txt")) {
            NewGame.print_debug("Game loaded from sudoku_3.txt");
        } else {
            NewGame.print_debug("Failed to load sudoku_3.txt");
        }
        break;
      case KEY_F(20):  // Shift-F8
        if (NewGame.LoadFromFile("sudoku_4.txt")) {
            NewGame.print_debug("Game loaded from sudoku_4.txt");
        } else {
            NewGame.print_debug("Failed to load sudoku_4.txt");
        }
        break;
    }
    move(0,0);
  }
  return 0;
}

// Implementation of new file loading functions
bool Sudoku::LoadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    // Clear the current board
    NewGame();
    
    string line;
    int row = 0;
    
    while (getline(file, line) && row < 9) {
        if (line.length() < 9) continue;  // Skip short lines
        
        for (int col = 0; col < 9; col++) {
            char c = line[col];
            if (c >= '1' && c <= '9') {
                SetValue(col, row, c - '1');
            }
            // Skip spaces, dots, and zeros
        }
        row++;
    }
    
    file.close();
    return row == 9;  // Return true if we read all 9 rows
}

void Sudoku::SaveToFile(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not create file " << filename << endl;
        return;
    }
    
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            int val = GetValue(col, row);
            if (val >= 0 && val <= 8) {
                file << (val + 1);
            } else {
                file << '.';
            }
        }
        file << endl;
    }
    
    file.close();
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

void Sudoku::NewGame() {
    // Clear the entire board
    for(int x = 0; x < 9; x++) {
        for(int y = 0; y < 9; y++) {
            ClearValue(x, y);
        }
    }
}

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
    // First pass: only look at filled cells
    for(int val : values) {
        if(val != -1) {  // Only check actual filled numbers
            if(used[val]) {
                return false;  // Real duplicate found
            }
            used[val] = true;
        }
    }
    // Don't check candidates - a partially filled valid unit is OK
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

void Sudoku::print_debug(const char *format, ...) {
    char buffer[256];  // Buffer for formatted string
    debug_line=0;
    // Format the string
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Move to position below grid
    move(29 + debug_line, 0);
    
    // Print the formatted string
    printw("%s", buffer);
    
    clrtoeol();  // Clear rest of line
    //refresh();
    
    // Increment line counter, wrap around after 20 lines
    debug_line = (debug_line + 1) % 20;
}

    void Sudoku::LogBoard(std::ofstream& file, const char* algorithm_name) {
        file << "\n=== " << algorithm_name << " ===\n";
        
        // Print timestamp
        time_t now = time(nullptr);
        file << "Time: " << ctime(&now);
        
        // Print horizontal border
        file << "+---+---+---+---+---+---+---+---+---+\n";
        
        // Print board contents
        for(int y = 0; y < 9; y++) {
            file << "|";
            for(int x = 0; x < 9; x++) {
                int value = GetValue(x, y);
                if(value >= 0 && value <= 8) {
                    file << " " << value + 1 << " ";
                } else {
                    file << " . ";
                }
                if((x + 1) % 3 == 0) file << "|";
                else file << " ";
            }
            file << "\n";
            
            // Print horizontal borders
            if((y + 1) % 3 == 0) {
                file << "+---+---+---+---+---+---+---+---+---+\n";
            }
        }
        file << "\n";
    }


int Sudoku::Solve() {
    bool changes_made;
    int result;
    
    do {
        changes_made = false;
        
        // First phase: Alternate between Standard and Line elimination until no changes
        bool basic_changes;
        do {
            basic_changes = false;
            
            // Run Standard elimination
            print_debug("Running StdElim...\n");
            refresh();
            result = StdElim();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after StdElim\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                basic_changes = true;
                changes_made = true;
            }
            
            // Run Line elimination
            print_debug("Running LinElim...\n");
            refresh();
            result = LinElim();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after LinElim\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                basic_changes = true;
                changes_made = true;
            }
            
        } while (basic_changes);
        
        // If no basic changes, try advanced techniques in sequence
        if (!changes_made) {
            // Try Hidden Singles
            print_debug("Running FindHiddenSingles...\n");
            refresh();
            result = FindHiddenSingles();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindHiddenSingles\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }
            
            // Try Hidden Pairs
            print_debug("Running FindHiddenPairs...\n");
            refresh();
            result = FindHiddenPairs();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindHiddenPairs\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }
            
            // Try Pointing Pairs
            print_debug("Running FindPointingPairs...\n");
            refresh();
            result = FindPointingPairs();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindPointingPairs\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }
            
            // Try X-Wing
            print_debug("Running FindXWing...\n");
            refresh();
            result = FindXWing();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindXWing\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }
            
            // Try Swordfish
            print_debug("Running FindSwordFish...\n");
            refresh();
            result = FindSwordFish();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindSwordFish\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }
            
            // Try Naked Sets
            print_debug("Running FindNakedSets...\n");
            refresh();
            result = FindNakedSets();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindNakedSets\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }

            /*print_debug("Running FindSimpleColoring...\n");
            refresh();
            result = FindSimpleColoring();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindSimpleColoring\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }*/


            // Try Find XY Wing
            /*print_debug("Running Find XY Wing...\n");
            refresh();
            result = FindXYWing();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindXYWing\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }*/

            // Try Find XYZ Wing
            /*print_debug("Running Find XYZ Wing...\n");
            refresh();
            result = FindXYZWing();
            if (!IsValidSolution()) {
                print_debug("Invalid solution detected after FindXYZWing\n");
                refresh();
                return -1;
            }
            if (result > 0) {
                changes_made = true;
                continue;  // Start over with basic eliminations
            }*/


        }
        
    } while (changes_made);
    
    // Final validation check
    if (!IsValidSolution()) {
        print_debug("Invalid final solution detected\n");
        refresh();
        return -1;
    }
    
    return 0;
}
int Sudoku::SolveBasic() {
    int counter1, counter2, i, j;
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
            print_debug("Running StdElim...                    \n");
            refresh();
            StdElim();
            if(!IsValidSolution()) {
                print_debug("Invalid solution detected after StdElim\n");
                refresh();
                //RestoreBoard(board, original_board);
                return -1;
            }
            
            print_debug("Running LinElim...                    \n");
            refresh();
            LinElim();
            if(!IsValidSolution()) {
                print_debug("Invalid solution detected after LinElim\n");
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
        print_debug("Invalid final solution detected\n");
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

    // Helper to verify that a potential X-Wing position is valid
    auto isValidXWingCell = [this](int row, int col, int val) -> bool {
        // Cell must be empty
        if(GetValue(row, col) != -1) return false;
        
        // Must have value as a candidate and be legal
        if(board[row][col][val] != val || !LegalValue(row, col, val)) return false;
        
        // Must have at least one other candidate
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v && LegalValue(row, col, v)) candidateCount++;
        }
        return candidateCount > 1;
    };

    // Process column-based X-Wing patterns
    for(int val = 0; val < 9; val++) {
        for(int col1 = 0; col1 < 8; col1++) {
            for(int col2 = col1 + 1; col2 < 9; col2++) {
                std::vector<int> rows1, rows2;

                // Find candidate positions in columns
                for(int row = 0; row < 9; row++) {
                    if(isValidXWingCell(row, col1, val)) rows1.push_back(row);
                    if(isValidXWingCell(row, col2, val)) rows2.push_back(row);
                }

                // Check for X-Wing pattern
                if(rows1.size() == 2 && rows2.size() == 2 && 
                   rows1[0] == rows2[0] && rows1[1] == rows2[1]) {
                    
                    bool madeChange = false;
                    int backup[9][9][9];

                    // Backup current state
                    memcpy(backup, board, sizeof(backup));

                    // Try eliminations for this pattern
                    for(int row : rows1) {
                        int rowPlaces = 0;
                        for(int c = 0; c < 9; c++) {
                            if(isValidXWingCell(row, c, val)) rowPlaces++;
                        }
                        if(rowPlaces > 2) {  // Only eliminate if not too constrained
                            for(int col = 0; col < 9; col++) {
                                if(col != col1 && col != col2 && isValidXWingCell(row, col, val)) {
                                    board[row][col][val] = -1;
                                    madeChange = true;
                                }
                            }
                        }
                    }

                    // Validate all eliminations together
                    if(madeChange) {
                        if(!IsValidSolution()) {
                            // Restore state if invalid
                            memcpy(board, backup, sizeof(backup));
                        } else {
                            changed++;
                        }
                    }
                }
            }
        }
    }

    // Process row-based X-Wing patterns
    for(int val = 0; val < 9; val++) {
        for(int row1 = 0; row1 < 8; row1++) {
            for(int row2 = row1 + 1; row2 < 9; row2++) {
                std::vector<int> cols1, cols2;

                // Find candidate positions in rows
                for(int col = 0; col < 9; col++) {
                    if(isValidXWingCell(row1, col, val)) cols1.push_back(col);
                    if(isValidXWingCell(row2, col, val)) cols2.push_back(col);
                }

                // Check for X-Wing pattern
                if(cols1.size() == 2 && cols2.size() == 2 && 
                   cols1[0] == cols2[0] && cols1[1] == cols2[1]) {
                    
                    bool madeChange = false;
                    int backup[9][9][9];

                    // Backup current state
                    memcpy(backup, board, sizeof(backup));

                    // Try eliminations for this pattern
                    for(int col : cols1) {
                        int colPlaces = 0;
                        for(int r = 0; r < 9; r++) {
                            if(isValidXWingCell(r, col, val)) colPlaces++;
                        }
                        if(colPlaces > 2) {  // Only eliminate if not too constrained
                            for(int row = 0; row < 9; row++) {
                                if(row != row1 && row != row2 && isValidXWingCell(row, col, val)) {
                                    board[row][col][val] = -1;
                                    madeChange = true;
                                }
                            }
                        }
                    }

                    // Validate all eliminations together
                    if(madeChange) {
                        if(!IsValidSolution()) {
                            // Restore state if invalid
                            memcpy(board, backup, sizeof(backup));
                        } else {
                            changed++;
                        }
                    }
                }
            }
        }
    }

    return changed;
}

int Sudoku::FindSwordFish() {
    int changed = 0;

    // Helper to check if a cell can have a value
    auto isCandidate = [this](int row, int col, int val) -> bool {
        return GetValue(row, col) == -1 && 
               board[row][col][val] == val && 
               LegalValue(row, col, val);
    };

    // Helper to validate elimination
    auto isSafeElimination = [this](int row, int col, int val) -> bool {
        if(GetValue(row, col) != -1) return false;
        if(board[row][col][val] != val) return false;
        
        // Count remaining candidates
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v && LegalValue(row, col, v)) {
                candidateCount++;
            }
        }
        return candidateCount > 1;
    };

    // Process row-based Swordfish
    for(int val = 0; val < 9; val++) {
        // Get candidate positions for each row
        std::vector<std::vector<int>> rowPositions(9);
        for(int row = 0; row < 9; row++) {
            for(int col = 0; col < 9; col++) {
                if(isCandidate(row, col, val)) {
                    rowPositions[row].push_back(col);
                }
            }
        }

        // Try each triplet of rows
        for(int row1 = 0; row1 < 7; row1++) {
            if(rowPositions[row1].size() < 2 || rowPositions[row1].size() > 3) continue;
            
            for(int row2 = row1 + 1; row2 < 8; row2++) {
                if(rowPositions[row2].size() < 2 || rowPositions[row2].size() > 3) continue;
                
                for(int row3 = row2 + 1; row3 < 9; row3++) {
                    if(rowPositions[row3].size() < 2 || rowPositions[row3].size() > 3) continue;

                    // Collect unique columns
                    std::set<int> uniqueCols;
                    for(int col : rowPositions[row1]) uniqueCols.insert(col);
                    for(int col : rowPositions[row2]) uniqueCols.insert(col);
                    for(int col : rowPositions[row3]) uniqueCols.insert(col);

                    // Check for Swordfish pattern
                    if(uniqueCols.size() == 3) {
                        // Make eliminations
                        bool madeChange = false;
                        int backup[9][9][9];
                        std::memcpy(backup, board, sizeof(backup));

                        // Eliminate from other rows in these columns
                        for(int col : uniqueCols) {
                            for(int row = 0; row < 9; row++) {
                                if(row != row1 && row != row2 && row != row3 &&
                                   isSafeElimination(row, col, val)) {
                                    board[row][col][val] = -1;
                                    madeChange = true;
                                }
                            }
                        }

                        // Validate changes
                        if(madeChange) {
                            if(!IsValidSolution()) {
                                std::memcpy(board, backup, sizeof(backup));
                            } else {
                                changed++;
                            }
                        }
                    }
                }
            }
        }
    }

    // Process column-based Swordfish
    for(int val = 0; val < 9; val++) {
        // Get candidate positions for each column
        std::vector<std::vector<int>> colPositions(9);
        for(int col = 0; col < 9; col++) {
            for(int row = 0; row < 9; row++) {
                if(isCandidate(row, col, val)) {
                    colPositions[col].push_back(row);
                }
            }
        }

        // Try each triplet of columns
        for(int col1 = 0; col1 < 7; col1++) {
            if(colPositions[col1].size() < 2 || colPositions[col1].size() > 3) continue;
            
            for(int col2 = col1 + 1; col2 < 8; col2++) {
                if(colPositions[col2].size() < 2 || colPositions[col2].size() > 3) continue;
                
                for(int col3 = col2 + 1; col3 < 9; col3++) {
                    if(colPositions[col3].size() < 2 || colPositions[col3].size() > 3) continue;

                    // Collect unique rows
                    std::set<int> uniqueRows;
                    for(int row : colPositions[col1]) uniqueRows.insert(row);
                    for(int row : colPositions[col2]) uniqueRows.insert(row);
                    for(int row : colPositions[col3]) uniqueRows.insert(row);

                    // Check for Swordfish pattern
                    if(uniqueRows.size() == 3) {
                        // Make eliminations
                        bool madeChange = false;
                        int backup[9][9][9];
                        std::memcpy(backup, board, sizeof(backup));

                        // Eliminate from other columns in these rows
                        for(int row : uniqueRows) {
                            for(int col = 0; col < 9; col++) {
                                if(col != col1 && col != col2 && col != col3 &&
                                   isSafeElimination(row, col, val)) {
                                    board[row][col][val] = -1;
                                    madeChange = true;
                                }
                            }
                        }

                        // Validate changes
                        if(madeChange) {
                            if(!IsValidSolution()) {
                                std::memcpy(board, backup, sizeof(backup));
                            } else {
                                changed++;
                            }
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
            print_debug("Cannot eliminate only candidate %d at (%d,%d)\n", value + 1, row + 1, col + 1);
            refresh();
            return false;
        }

        // Make the elimination
        board[row][col][value] = -1;

        // Validate resulting state
        if(!IsValidSolution()) {
            // Restore if invalid
            board[row][col][value] = value;
            print_debug("Eliminating %d from (%d,%d) would create invalid state\n", 
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
                print_debug("Processing filled cell (%d,%d) with value %d\n", x + 1, y + 1, value + 1);
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
                    print_debug("Invalid board state after processing cell (%d,%d)\n", x + 1, y + 1);
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
    //auto logMove = [](const char* unitType, int unit, int val, int pos) {
    auto logMove = [this](const char* unitType, int unit, int val, int pos) {
        print_debug("Found hidden single: value %d in %s %d at position %d\n", 
               val + 1, unitType, unit + 1, pos + 1);
        refresh();
    };

    // Helper function to validate before setting
    auto validateMove = [this](int row, int col, int val) -> bool {
        // Check row
        for(int c = 0; c < 9; c++) {
            if(c != col && GetValue(row, c) == val) {
                print_debug("Row conflict: %d already exists in row %d\n", val + 1, row + 1);
                refresh();
                return false;
            }
        }
        
        // Check column
        for(int r = 0; r < 9; r++) {
            if(r != row && GetValue(r, col) == val) {
                print_debug("Column conflict: %d already exists in column %d\n", val + 1, col + 1);
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
                    print_debug("Box conflict: %d already exists in box\n", val + 1);
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
                        print_debug("Invalid solution after setting %d at (%d,%d)\n", 
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
                        print_debug("Invalid solution after setting %d at (%d,%d)\n", 
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
                        print_debug("Invalid solution after setting %d at (%d,%d)\n", 
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

std::vector<int> Sudoku::GetCellCandidates(int x, int y) {
    std::vector<int> candidates;
    if(GetValue(x, y) != -1) return candidates;  // Return empty if cell is filled
    
    for(int val = 0; val < 9; val++) {
        if(board[x][y][val] == val && LegalValue(x, y, val)) {
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

    // Helper to get candidates for a cell
    auto getCandidates = [this](int row, int col) -> std::vector<int> {
        std::vector<int> candidates;
        if(GetValue(row, col) == -1) {  // Only if cell is empty
            for(int val = 0; val < 9; val++) {
                if(board[row][col][val] == val && LegalValue(row, col, val)) {
                    candidates.push_back(val);
                }
            }
        }
        return candidates;
    };

    // Helper to check if candidates form a naked set
    auto isNakedSet = [](const std::vector<std::vector<int>>& candidateSets, const std::vector<int>& unionCandidates) -> bool {
        // All cells must contain only candidates from the union set
        for(const auto& candidates : candidateSets) {
            for(int val : candidates) {
                if(std::find(unionCandidates.begin(), unionCandidates.end(), val) == unionCandidates.end()) {
                    return false;
                }
            }
        }
        return true;
    };

    // Process each row
    for(int row = 0; row < 9; row++) {
        std::vector<std::pair<int, std::vector<int>>> cells;  // col, candidates
        
        // Collect empty cells and their candidates
        for(int col = 0; col < 9; col++) {
            auto candidates = getCandidates(row, col);
            if(!candidates.empty()) {
                cells.push_back({col, candidates});
            }
        }

        // Try naked sets of size 2-4
        for(int setSize = 2; setSize <= 4; setSize++) {
            // Skip if we don't have enough cells
            if(cells.size() < setSize) continue;

            // Try each combination of cells
            std::vector<bool> selected(cells.size(), false);
            for(int i = 0; i < setSize; i++) selected[cells.size() - 1 - i] = true;

            do {
                std::vector<int> setCols;
                std::vector<std::vector<int>> setCandidates;
                std::set<int> uniqueCandidates;

                // Collect selected cells
                for(size_t i = 0; i < cells.size(); i++) {
                    if(selected[i]) {
                        setCols.push_back(cells[i].first);
                        setCandidates.push_back(cells[i].second);
                        for(int val : cells[i].second) {
                            uniqueCandidates.insert(val);
                        }
                    }
                }

                // If number of unique candidates equals set size, we found a naked set
                if(uniqueCandidates.size() == setSize) {
                    std::vector<int> candidates(uniqueCandidates.begin(), uniqueCandidates.end());
                    if(isNakedSet(setCandidates, candidates)) {
                        // Eliminate these candidates from other cells in row
                        bool madeChange = false;
                        for(int col = 0; col < 9; col++) {
                            if(std::find(setCols.begin(), setCols.end(), col) == setCols.end() && 
                               GetValue(row, col) == -1) {
                                for(int val : candidates) {
                                    if(board[row][col][val] == val) {
                                        board[row][col][val] = -1;
                                        madeChange = true;
                                    }
                                }
                            }
                        }
                        if(madeChange) changed++;
                    }
                }
            } while(std::next_permutation(selected.begin(), selected.end()));
        }
    }

    // Process each column (similar logic)
    for(int col = 0; col < 9; col++) {
        std::vector<std::pair<int, std::vector<int>>> cells;  // row, candidates
        
        for(int row = 0; row < 9; row++) {
            auto candidates = getCandidates(row, col);
            if(!candidates.empty()) {
                cells.push_back({row, candidates});
            }
        }

        for(int setSize = 2; setSize <= 4; setSize++) {
            if(cells.size() < setSize) continue;

            std::vector<bool> selected(cells.size(), false);
            for(int i = 0; i < setSize; i++) selected[cells.size() - 1 - i] = true;

            do {
                std::vector<int> setRows;
                std::vector<std::vector<int>> setCandidates;
                std::set<int> uniqueCandidates;

                for(size_t i = 0; i < cells.size(); i++) {
                    if(selected[i]) {
                        setRows.push_back(cells[i].first);
                        setCandidates.push_back(cells[i].second);
                        for(int val : cells[i].second) {
                            uniqueCandidates.insert(val);
                        }
                    }
                }

                if(uniqueCandidates.size() == setSize) {
                    std::vector<int> candidates(uniqueCandidates.begin(), uniqueCandidates.end());
                    if(isNakedSet(setCandidates, candidates)) {
                        bool madeChange = false;
                        for(int row = 0; row < 9; row++) {
                            if(std::find(setRows.begin(), setRows.end(), row) == setRows.end() && 
                               GetValue(row, col) == -1) {
                                for(int val : candidates) {
                                    if(board[row][col][val] == val) {
                                        board[row][col][val] = -1;
                                        madeChange = true;
                                    }
                                }
                            }
                        }
                        if(madeChange) changed++;
                    }
                }
            } while(std::next_permutation(selected.begin(), selected.end()));
        }
    }

    // Process each 3x3 box
    for(int boxRow = 0; boxRow < 3; boxRow++) {
        for(int boxCol = 0; boxCol < 3; boxCol++) {
            std::vector<std::pair<std::pair<int,int>, std::vector<int>>> cells;
            
            for(int i = 0; i < 3; i++) {
                for(int j = 0; j < 3; j++) {
                    int row = boxRow * 3 + i;
                    int col = boxCol * 3 + j;
                    auto candidates = getCandidates(row, col);
                    if(!candidates.empty()) {
                        cells.push_back({{row, col}, candidates});
                    }
                }
            }

            for(int setSize = 2; setSize <= 4; setSize++) {
                if(cells.size() < setSize) continue;

                std::vector<bool> selected(cells.size(), false);
                for(int i = 0; i < setSize; i++) selected[cells.size() - 1 - i] = true;

                do {
                    std::vector<std::pair<int,int>> setPositions;
                    std::vector<std::vector<int>> setCandidates;
                    std::set<int> uniqueCandidates;

                    for(size_t i = 0; i < cells.size(); i++) {
                        if(selected[i]) {
                            setPositions.push_back(cells[i].first);
                            setCandidates.push_back(cells[i].second);
                            for(int val : cells[i].second) {
                                uniqueCandidates.insert(val);
                            }
                        }
                    }

                    if(uniqueCandidates.size() == setSize) {
                        std::vector<int> candidates(uniqueCandidates.begin(), uniqueCandidates.end());
                        if(isNakedSet(setCandidates, candidates)) {
                            bool madeChange = false;
                            for(int i = 0; i < 3; i++) {
                                for(int j = 0; j < 3; j++) {
                                    int row = boxRow * 3 + i;
                                    int col = boxCol * 3 + j;
                                    if(std::find(setPositions.begin(), setPositions.end(), 
                                       std::make_pair(row,col)) == setPositions.end() && 
                                       GetValue(row, col) == -1) {
                                        for(int val : candidates) {
                                            if(board[row][col][val] == val) {
                                                board[row][col][val] = -1;
                                                madeChange = true;
                                            }
                                        }
                                    }
                                }
                            }
                            if(madeChange) changed++;
                        }
                    }
                } while(std::next_permutation(selected.begin(), selected.end()));
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

int Sudoku::FindXYWing() {
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

    // Helper to check if cells share a unit
    auto shareUnit = [](int row1, int col1, int row2, int col2) -> bool {
        return row1 == row2 ||  // Same row
               col1 == col2 ||  // Same column
               (row1/3 == row2/3 && col1/3 == col2/3);  // Same box
    };

    // Store board state before any changes
    int originalBoard[9][9][9];
    memcpy(originalBoard, board, sizeof(board));

    struct Elimination {
        int row, col, val;
        Elimination(int r, int c, int v) : row(r), col(c), val(v) {}
    };
    std::vector<Elimination> potentialEliminations;

    // Find all potential eliminations first
    for(int pivotRow = 0; pivotRow < 9; pivotRow++) {
        for(int pivotCol = 0; pivotCol < 9; pivotCol++) {
            auto pivotCands = getCandidates(pivotRow, pivotCol);
            if(pivotCands.size() != 2) continue;

            for(int wing1Row = 0; wing1Row < 9; wing1Row++) {
                for(int wing1Col = 0; wing1Col < 9; wing1Col++) {
                    if(wing1Row == pivotRow && wing1Col == pivotCol) continue;
                    if(!shareUnit(pivotRow, pivotCol, wing1Row, wing1Col)) continue;

                    auto wing1Cands = getCandidates(wing1Row, wing1Col);
                    if(wing1Cands.size() != 2) continue;

                    // Find shared candidate between pivot and wing1
                    int sharedWithWing1 = -1;
                    for(int val : pivotCands) {
                        if(std::find(wing1Cands.begin(), wing1Cands.end(), val) != wing1Cands.end()) {
                            sharedWithWing1 = val;
                            break;
                        }
                    }
                    if(sharedWithWing1 == -1) continue;

                    for(int wing2Row = 0; wing2Row < 9; wing2Row++) {
                        for(int wing2Col = 0; wing2Col < 9; wing2Col++) {
                            if((wing2Row == pivotRow && wing2Col == pivotCol) ||
                               (wing2Row == wing1Row && wing2Col == wing1Col)) continue;
                            if(!shareUnit(pivotRow, pivotCol, wing2Row, wing2Col)) continue;

                            auto wing2Cands = getCandidates(wing2Row, wing2Col);
                            if(wing2Cands.size() != 2) continue;

                            // Find shared candidate between pivot and wing2
                            int sharedWithWing2 = -1;
                            for(int val : pivotCands) {
                                if(val != sharedWithWing1 &&
                                   std::find(wing2Cands.begin(), wing2Cands.end(), val) != wing2Cands.end()) {
                                    sharedWithWing2 = val;
                                    break;
                                }
                            }
                            if(sharedWithWing2 == -1) continue;

                            // Find common candidate between wings
                            int commonWingVal = -1;
                            for(int val : wing1Cands) {
                                if(val != sharedWithWing1 &&
                                   std::find(wing2Cands.begin(), wing2Cands.end(), val) != wing2Cands.end()) {
                                    commonWingVal = val;
                                    break;
                                }
                            }
                            if(commonWingVal == -1) continue;

                            // Store potential eliminations
                            for(int row = 0; row < 9; row++) {
                                for(int col = 0; col < 9; col++) {
                                    if((row == pivotRow && col == pivotCol) ||
                                       (row == wing1Row && col == wing1Col) ||
                                       (row == wing2Row && col == wing2Col)) continue;

                                    if(shareUnit(row, col, wing1Row, wing1Col) &&
                                       shareUnit(row, col, wing2Row, wing2Col) &&
                                       GetValue(row, col) == -1 &&
                                       board[row][col][commonWingVal] == commonWingVal) {
                                        potentialEliminations.push_back(Elimination(row, col, commonWingVal));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Try each elimination one at a time
    for(const auto& elim : potentialEliminations) {
        // Restore original board state
        memcpy(board, originalBoard, sizeof(board));

        // Try this single elimination
        print_debug("Trying to eliminate %d from (%d,%d)\n", 
                   elim.val + 1, elim.row + 1, elim.col + 1);
        refresh();

        board[elim.row][elim.col][elim.val] = -1;

        // Validate the change
        if(IsValidSolution()) {
            print_debug("Successfully eliminated %d from (%d,%d)\n", 
                       elim.val + 1, elim.row + 1, elim.col + 1);
            refresh();
            return 1; // Return after one successful change
        }

        // If invalid, next iteration will restore original state
    }

    // No valid eliminations found
    memcpy(board, originalBoard, sizeof(board));
    return 0;
}

int Sudoku::FindXYZWing() {
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

    // Helper to check if cells can see each other
    auto canSee = [](int row1, int col1, int row2, int col2) -> bool {
        return row1 == row2 ||  // Same row
               col1 == col2 ||  // Same column
               (row1/3 == row2/3 && col1/3 == col2/3);  // Same box
    };

    // Helper to validate elimination
    auto isSafeElimination = [this](int row, int col, int val) -> bool {
        if(GetValue(row, col) != -1) return false;
        if(board[row][col][val] != val) return false;
        
        int candidateCount = 0;
        for(int v = 0; v < 9; v++) {
            if(board[row][col][v] == v) candidateCount++;
        }
        return candidateCount > 1;
    };

    // For each potential pivot cell (must have exactly 3 candidates)
    for(int pivotRow = 0; pivotRow < 9; pivotRow++) {
        for(int pivotCol = 0; pivotCol < 9; pivotCol++) {
            auto pivotCands = getCandidates(pivotRow, pivotCol);
            if(pivotCands.size() != 3) continue;  // Must have exactly 3 candidates

            // For each potential first wing
            for(int wing1Row = 0; wing1Row < 9; wing1Row++) {
                for(int wing1Col = 0; wing1Col < 9; wing1Col++) {
                    if(wing1Row == pivotRow && wing1Col == pivotCol) continue;
                    if(!canSee(pivotRow, pivotCol, wing1Row, wing1Col)) continue;

                    auto wing1Cands = getCandidates(wing1Row, wing1Col);
                    if(wing1Cands.size() != 2) continue;  // Must have exactly 2 candidates

                    // Verify wing1 shares exactly 2 candidates with pivot
                    std::vector<int> sharedWithWing1;
                    for(int val : wing1Cands) {
                        if(std::find(pivotCands.begin(), pivotCands.end(), val) != pivotCands.end()) {
                            sharedWithWing1.push_back(val);
                        }
                    }
                    if(sharedWithWing1.size() != 2) continue;

                    // For each potential second wing
                    for(int wing2Row = 0; wing2Row < 9; wing2Row++) {
                        for(int wing2Col = 0; wing2Col < 9; wing2Col++) {
                            if((wing2Row == pivotRow && wing2Col == pivotCol) ||
                               (wing2Row == wing1Row && wing2Col == wing1Col)) continue;
                            if(!canSee(pivotRow, pivotCol, wing2Row, wing2Col)) continue;

                            auto wing2Cands = getCandidates(wing2Row, wing2Col);
                            if(wing2Cands.size() != 2) continue;

                            // Verify wing2 shares exactly 2 candidates with pivot
                            std::vector<int> sharedWithWing2;
                            for(int val : wing2Cands) {
                                if(std::find(pivotCands.begin(), pivotCands.end(), val) != pivotCands.end()) {
                                    sharedWithWing2.push_back(val);
                                }
                            }
                            if(sharedWithWing2.size() != 2) continue;

                            // Find the common candidate Z (present in all three cells)
                            int Z = -1;
                            for(int val : pivotCands) {
                                if(std::find(wing1Cands.begin(), wing1Cands.end(), val) != wing1Cands.end() &&
                                   std::find(wing2Cands.begin(), wing2Cands.end(), val) != wing2Cands.end()) {
                                    Z = val;
                                    break;
                                }
                            }
                            if(Z == -1) continue;

                            // Found XYZ-Wing pattern! Look for cells that see all three cells
                            print_debug("Found XYZ-Wing: pivot(%d,%d) wings(%d,%d)(%d,%d) Z=%d\n",
                                      pivotRow + 1, pivotCol + 1, 
                                      wing1Row + 1, wing1Col + 1,
                                      wing2Row + 1, wing2Col + 1,
                                      Z + 1);
                            refresh();

                            bool madeChange = false;
                            for(int row = 0; row < 9; row++) {
                                for(int col = 0; col < 9; col++) {
                                    // Skip the cells in the pattern
                                    if((row == pivotRow && col == pivotCol) ||
                                       (row == wing1Row && col == wing1Col) ||
                                       (row == wing2Row && col == wing2Col)) continue;

                                    // Must see all three cells
                                    if(canSee(row, col, pivotRow, pivotCol) &&
                                       canSee(row, col, wing1Row, wing1Col) &&
                                       canSee(row, col, wing2Row, wing2Col) &&
                                       isSafeElimination(row, col, Z)) {
                                        
                                        board[row][col][Z] = -1;
                                        madeChange = true;
                                        print_debug("Eliminated %d from (%d,%d)\n", 
                                                  Z + 1, row + 1, col + 1);
                                        refresh();
                                    }
                                }
                            }

                            if(madeChange) {
                                changed++;
                                // Validate after changes
                                if(!IsValidSolution()) {
                                    print_debug("Invalid solution after XYZ-Wing elimination\n");
                                    refresh();
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return changed;
}

int Sudoku::FindSimpleColoring() {
    int changed = 0;
    
    // Helper to get candidates for a cell
    auto getCandidates = [this](int row, int col) -> std::vector<int> {
        std::vector<int> candidates;
        if(GetValue(row, col) != -1) return candidates;
        for(int val = 0; val < 9; val++) {
            if(board[row][col][val] == val && LegalValue(row, col, val)) {
                candidates.push_back(val);
            }
        }
        return candidates;
    };

    // Helper to check if cells can see each other (same row, column, or box)
    auto canSee = [](int row1, int col1, int row2, int col2) -> bool {
        return row1 == row2 ||  // Same row
               col1 == col2 ||  // Same column
               (row1/3 == row2/3 && col1/3 == col2/3);  // Same box
    };

    // Structure to represent a cell in the coloring chain
    struct ColoredCell {
        int row, col;
        bool color;  // true for color1, false for color2
        ColoredCell(int r, int c, bool clr) : row(r), col(c), color(clr) {}
    };

    // For each candidate value
    for(int val = 0; val < 9; val++) {
        // Find all cells that have this candidate
        std::vector<std::pair<int, int>> candidateCells;
        for(int row = 0; row < 9; row++) {
            for(int col = 0; col < 9; col++) {
                auto candidates = getCandidates(row, col);
                if(std::find(candidates.begin(), candidates.end(), val) != candidates.end()) {
                    candidateCells.push_back({row, col});
                }
            }
        }

        // For each starting cell, try to build a chain
        for(const auto& start : candidateCells) {
            std::vector<ColoredCell> chain;
            std::vector<std::vector<bool>> visited(9, std::vector<bool>(9, false));
            
            // Start with the first cell colored as true
            chain.emplace_back(start.first, start.second, true);
            visited[start.first][start.second] = true;

            bool chainChanged;
            do {
                chainChanged = false;
                
                // Look for cells that can extend the chain
                for(const auto& cell : chain) {
                    for(const auto& candidate : candidateCells) {
                        int row = candidate.first;
                        int col = candidate.second;
                        
                        if(!visited[row][col] && canSee(cell.row, cell.col, row, col)) {
                            // Add cell with opposite color
                            chain.emplace_back(row, col, !cell.color);
                            visited[row][col] = true;
                            chainChanged = true;
                        }
                    }
                }
            } while(chainChanged);

            // Check for chain-based eliminations
            if(chain.size() >= 2) {
                // Check for same-colored cells that can see each other
                for(size_t i = 0; i < chain.size(); i++) {
                    for(size_t j = i + 1; j < chain.size(); j++) {
                        if(chain[i].color == chain[j].color && 
                           canSee(chain[i].row, chain[i].col, chain[j].row, chain[j].col)) {
                            // Invalid coloring - can eliminate this candidate from all cells of the opposite color
                            bool madeChange = false;
                            int backup[9][9][9];
                            memcpy(backup, board, sizeof(backup));

                            for(const auto& cell : chain) {
                                if(cell.color != chain[i].color) {
                                    board[cell.row][cell.col][val] = -1;
                                    madeChange = true;
                                }
                            }

                            // Validate changes
                            if(madeChange) {
                                if(!IsValidSolution()) {
                                    memcpy(board, backup, sizeof(backup));
                                } else {
                                    changed++;
                                    print_debug("Simple Coloring: eliminated %d from opposite colored cells\n", 
                                              val + 1);
                                    refresh();
                                }
                            }
                        }
                    }
                }

                // Check for cells that can see both colors
                for(int row = 0; row < 9; row++) {
                    for(int col = 0; col < 9; col++) {
                        if(visited[row][col]) continue;

                        bool seesColor1 = false;
                        bool seesColor2 = false;

                        for(const auto& cell : chain) {
                            if(canSee(row, col, cell.row, cell.col)) {
                                if(cell.color) seesColor1 = true;
                                else seesColor2 = true;
                            }
                        }

                        // If cell sees both colors, we can eliminate the candidate
                        if(seesColor1 && seesColor2 && board[row][col][val] == val) {
                            int backup[9][9][9];
                            memcpy(backup, board, sizeof(backup));

                            board[row][col][val] = -1;

                            // Validate change
                            if(!IsValidSolution()) {
                                memcpy(board, backup, sizeof(backup));
                            } else {
                                changed++;
                                print_debug("Simple Coloring: eliminated %d from (%d,%d) - sees both colors\n", 
                                          val + 1, row + 1, col + 1);
                                refresh();
                            }
                        }
                    }
                }
            }
        }
    }

    return changed;
}

