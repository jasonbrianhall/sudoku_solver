#include <iostream>
using namespace std;
#include <stdlib.h>

#ifndef _WIN32
    #include <ncurses.h>
#else
    #include <curses.h>
#endif

#define _NCURSES

#include <vector>
#include <set>
#include <algorithm>

#include <fstream>
#include <ctime>
#include <cstring>

#include "sudoku.h"
#include "generatepuzzle.h"


void print_usage() {
    cout << "Usage:" << endl;
    cout << "  sudoku                     - Run in interactive mode" << endl;
    cout << "  sudoku -f <input_file>     - Load and solve puzzle from file" << endl;
}

int main(int argc, char* argv[]) {
 
    int i, x, y, temp, input, x_pos=0, y_pos=0;
    Sudoku NewGame;
    std::ofstream logfile("sudoku_progress.txt", std::ios::app);
   
    // Parse command line arguments
    string input_file = "";
    string output_file = "";
    
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
    
    printw("Welcome to Sudoku Solver (Press F1-4 or Shift F1 to generate a random puzzle with increasing difficulty)\n");
    printw("Commands:                          Solving techniques:                                Experimental Techniques\n");
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
        NewGame.Clean();
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
      case KEY_F(1):  // F1
        {
            PuzzleGenerator generator(NewGame);
            if (generator.generatePuzzle("easy")) {
                NewGame.print_debug("Generated new easy puzzle");
            } else {
                NewGame.print_debug("Failed to generate easy puzzle");
                NewGame.NewGame();
            }
            NewGame.Clean();
        }
        break;
      case KEY_F(2):  // F2
        {
            PuzzleGenerator generator(NewGame);
            if (generator.generatePuzzle("medium")) {
                NewGame.print_debug("Generated new medium puzzle");
            } else {
                NewGame.print_debug("Failed to generate medium puzzle");
                NewGame.NewGame();
            }
            NewGame.Clean();

        }
        break;
      case KEY_F(3):  // F3
        {
            PuzzleGenerator generator(NewGame);
            if (generator.generatePuzzle("hard")) {
                NewGame.print_debug("Generated new hard puzzle");
            } else {
                NewGame.print_debug("Failed to generate hard puzzle");
                NewGame.NewGame();
            }
            NewGame.Clean();
        }
        break;
      case KEY_F(4):  // F4
        {
            PuzzleGenerator generator(NewGame);
            if (generator.generatePuzzle("expert")) {
                NewGame.print_debug("Generated new expert puzzle");
            } else {
                NewGame.print_debug("Failed to generate expert puzzle");
                NewGame.NewGame();
            }
            NewGame.Clean();
        }
        break;
      case KEY_F(13):  // Shift + F1
        {
            PuzzleGenerator generator(NewGame);
            if (generator.generatePuzzle("extreme")) {
                NewGame.print_debug("Generated new extreme puzzle");
            } else {
                NewGame.print_debug("Failed to generate extreme puzzle");
                NewGame.NewGame();
            }
            NewGame.Clean();
        }
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
    refresh();
  }
  return 0;
}


