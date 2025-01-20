#include <iostream>
#include <conio.h>
#include <dos.h>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <cstring>

#include "sudoku.h"
#include "generatepuzzle.h"

using namespace std;

// Constants for key codes in DOS
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_F1 59
#define KEY_F2 60
#define KEY_F3 61
#define KEY_F4 62
#define KEY_F5 63
#define KEY_F6 64
#define KEY_F7 65
#define KEY_F8 66
#define SHIFT_F1 84
#define SHIFT_F5 87
#define SHIFT_F6 88
#define SHIFT_F7 89
#define SHIFT_F8 90

/*void gotoxy(int x, int y) {
    union REGS regs;
    regs.h.ah = 0x02;
    regs.h.bh = 0x00;
    regs.h.dh = y;
    regs.h.dl = x;
    int86(0x10, &regs, &regs);
}*/

void print_usage() {
    cout << "Usage:" << endl;
    cout << "  sudoku                     - Run in interactive mode" << endl;
    cout << "  sudoku -f <input_file>     - Load and solve puzzle from file" << endl;
}

int main(int argc, char* argv[]) {
    int i, x, y, temp, input, x_pos=0, y_pos=0;
    Sudoku NewGame;
    std::ofstream logfile("sudoku_progress.txt", std::ios::app);
    
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
    
    if (!input_file.empty()) {
        if (!NewGame.LoadFromFile(input_file)) {
            cerr << "Failed to load puzzle from " << input_file << endl;
            return 1;
        }
    }

    _setcursortype(_NORMALCURSOR);

    while(1) {
        clrscr();
        
        cputs("Welcome to Sudoku Solver (Press F1-4 or Shift F1 to generate a random puzzle with increasing difficulty)\r\n");
        cputs("Commands:                          Solving techniques:                                Experimental Techniques\r\n");
        cputs(" Arrow keys - Move cursor           S - Standard elimination    N - Hidden singles     Y - Find XY Wing\r\n");
        cputs(" 1-9 - Fill number                  L - Line elimination        K - Naked sets         ; - Find XYZ Wing\r\n");
        cputs(" 0 - Clear cell                     H - Hidden pairs            X - X-Wing             C - Simple Coloring\r\n");
        cputs(" q - Quit                           P - Pointing pairs          F - Swordfish\r\n");
        cputs(" A - Run all techniques             Z - New Game                F(5-8) - Save Game     Shift F(5-8) - Load Game\r\n\r\n");
        
        for(y=0; y<9; y++) {
            if(y%3==0) {
                for(i=0; i<37; i++) cprintf("-");
            } else {
                for(i=0; i<37; i++) {
                    if(i%4==0) {
                        if(i%12==0) cprintf("+");
                        else cprintf("+");
                    }
                    else cprintf("-");
                }
            }
            cprintf("\r\n");
            
            for(x=0; x<9; x++) {
                if(x%3==0) cprintf("|");
                else cprintf("|");
                
                temp = NewGame.GetValue(x,y);
                if(temp >= 0 && temp <= 8) {
                    cprintf(" %d ", temp+1);
                }
                else cprintf("   ");
            }
            cprintf("|\r\n");
        }
        
        for(i=0; i<37; i++) cprintf("-");
        cprintf("\r\n");

        int cursor_y = 8 + y_pos * 2 + 1;
        gotoxy(x_pos*4 + 2, cursor_y);
        
        if (kbhit()) {
            input = getch();
            if (input == 0) {
                input = getch();
                switch(input) {
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
                    case KEY_F1:
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
                    case KEY_F2:
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
                    case KEY_F3:
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
                    case KEY_F4:
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
                    case KEY_F5:
                        NewGame.SaveToFile("sudoku_1.txt");
                        NewGame.print_debug("Game saved to sudoku_1.txt");
                        break;
                    case KEY_F6:
                        NewGame.SaveToFile("sudoku_2.txt");
                        NewGame.print_debug("Game saved to sudoku_2.txt");
                        break;
                    case KEY_F7:
                        NewGame.SaveToFile("sudoku_3.txt");
                        NewGame.print_debug("Game saved to sudoku_3.txt");
                        break;
                    case KEY_F8:
                        NewGame.SaveToFile("sudoku_4.txt");
                        NewGame.print_debug("Game saved to sudoku_4.txt");
                        break;
                    case SHIFT_F1:
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
                    case SHIFT_F5:
                        if (NewGame.LoadFromFile("sudoku_1.txt")) {
                            NewGame.print_debug("Game loaded from sudoku_1.txt");
                        } else {
                            NewGame.print_debug("Failed to load sudoku_1.txt");
                        }
                        break;
                    case SHIFT_F6:
                        if (NewGame.LoadFromFile("sudoku_2.txt")) {
                            NewGame.print_debug("Game loaded from sudoku_2.txt");
                        } else {
                            NewGame.print_debug("Failed to load sudoku_2.txt");
                        }
                        break;
                    case SHIFT_F7:
                        if (NewGame.LoadFromFile("sudoku_3.txt")) {
                            NewGame.print_debug("Game loaded from sudoku_3.txt");
                        } else {
                            NewGame.print_debug("Failed to load sudoku_3.txt");
                        }
                        break;
                    case SHIFT_F8:
                        if (NewGame.LoadFromFile("sudoku_4.txt")) {
                            NewGame.print_debug("Game loaded from sudoku_4.txt");
                        } else {
                            NewGame.print_debug("Failed to load sudoku_4.txt");
                        }
                        break;
                }
            } else {
                if(input >= 'a' && input <= 'z') {
                    input = input - 'a' + 'A';
                }
                
                switch(input) {
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
                        return 0;
                    case 'S':
                        NewGame.LogBoard(logfile, "Standard Elim Before");
                        NewGame.StdElim();
                        NewGame.LogBoard(logfile, "Standard Elim After");
                        break;
                    case 'L':
                        NewGame.LogBoard(logfile, "Line Elim Before");
                        NewGame.LinElim();
                        NewGame.LogBoard(logfile, "Line Elim After");
                        break;
                    case 'C':
                        NewGame.LogBoard(logfile, "Find Simple Coloring Before");
                        NewGame.FindSimpleColoring();
                        NewGame.LogBoard(logfile, "Find Simple Coloring After");
                        break;
                    case 'H':
                        NewGame.LogBoard(logfile, "Find Hidden Pairs Before");
                        NewGame.FindHiddenPairs();
                        NewGame.LogBoard(logfile, "Find Hidden Pairs After");
                        break;
                    case 'P':
                        NewGame.LogBoard(logfile, "Find Pointing Pairs Before");
                        NewGame.FindPointingPairs();
                        NewGame.LogBoard(logfile, "Find Pointing Pairs After");
                        break;
                    case 'X':
                        NewGame.LogBoard(logfile, "Find XWING Before");
                        NewGame.FindXWing();
                        NewGame.LogBoard(logfile, "Find XWING After");
                        break;
                    case 'Y':
                        NewGame.LogBoard(logfile, "Find XYWING Before");
                        NewGame.FindXYWing();
                        NewGame.LogBoard(logfile, "Find XYWING After");
                        break;
                    case ';':
                        NewGame.LogBoard(logfile, "Find XYZWING Before");
                        NewGame.FindXYZWing();
                        NewGame.LogBoard(logfile, "Find XYZWING After");
                        break;
                    case 'F':
                        NewGame.LogBoard(logfile, "Find Swordfish Before");
                        NewGame.FindSwordFish();
                        NewGame.LogBoard(logfile, "Find Swordfish After");
                        break;
                    case 'N':
                        NewGame.LogBoard(logfile, "Find Hidden Singles Before");
                        NewGame.FindHiddenSingles();
                        NewGame.LogBoard(logfile, "Find Hidden Singles After");
                        break;
                    case 'K':
                        NewGame.LogBoard(logfile, "Find Naked Sets Before");
                        NewGame.FindNakedSets();
                        NewGame.LogBoard(logfile, "Find Naked Sets After");
                        break;
                    case 'A':
                        NewGame.LogBoard(logfile, "Run All Techniques Before");
                        NewGame.Solve();
                        NewGame.LogBoard(logfile, "Run All Techniques After");
                        break;
                    case 'Z':
                        NewGame.NewGame();
                        break;
                }
            }
        }
    }
    return 0;
}
