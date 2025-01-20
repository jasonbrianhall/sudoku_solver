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
#define SHIFT_F2 85
#define SHIFT_F3 86
#define SHIFT_F4 87
#define SHIFT_F5 88
#define SHIFT_F6 89
#define SHIFT_F7 90
#define SHIFT_F8 91

// Color attributes
#define RED LIGHTRED
#define NORMAL LIGHTGRAY

void printc(char c, int color) {
    textcolor(color);
    putch(c);
}

void show_help() {
    clrscr();
    cputs("Sudoku Help\r\n");
    cputs("\r\n");
    cputs("Game Controls:\r\n");
    cputs(" Arrow keys - Move cursor\r\n");
    cputs(" 1-9       - Fill number\r\n");
    cputs(" 0         - Clear cell\r\n");
    cputs(" Q         - Quit game\r\n");
    cputs("\r\n");
    cputs("Puzzle Generation:\r\n");
    cputs(" F1        - Generate Easy puzzle\r\n");
    cputs(" F2        - Generate Medium puzzle\r\n");
    cputs(" F3        - Generate Hard puzzle\r\n");
    cputs(" F4        - Generate Expert puzzle\r\n");
    cputs(" Shift+F1  - Generate Extreme puzzle\r\n");
    cputs("\r\n");
    cputs("Solving Techniques:\r\n");
    cputs(" S - Standard elimination    N - Hidden singles    Y - Find XY Wing\r\n");
    cputs(" L - Line elimination        K - Naked sets        ; - Find XYZ Wing\r\n");
    cputs(" H - Hidden pairs            X - X-Wing            C - Simple Coloring\r\n");
    cputs(" P - Pointing pairs          F - Swordfish\r\n");
    cputs(" A - Run all techniques\r\n");
    cputs("\r\n");
    cputs("Press any key to return to game...");
    getch();
}

void draw_screen(Sudoku& NewGame, int x_pos, int y_pos) {
    clrscr();
    textcolor(NORMAL);
    
    // Print header
    cputs("Welcome to Sudoku Solver for MS-DOS\r\n");
    cputs("Press (H) for Help\r\n\r\n");
    
    // Draw the grid
    for(int y=0; y<9; y++) {
        if(y%3==0) {
            for(int i=0; i<37; i++) {
                printc('-', RED);
            }
        } else {
            for(int i=0; i<37; i++) {
                if(i%12==0) {
                    printc('+', RED);  // 3x3 intersection
                } else if(i%4==0) {
                    printc('+', NORMAL);  // Regular intersection
                } else {
                    printc('-', NORMAL);  // Regular horizontal line
                }
            }
        }
        cprintf("\r\n");
        
        for(int x=0; x<9; x++) {
            if(x%3==0) {
                printc('|', RED);
            } else {
                printc('|', NORMAL);
            }
            
            int temp = NewGame.GetValue(x,y);
            textcolor(NORMAL);
            if(temp >= 0 && temp <= 8) {
                cprintf(" %d ", temp+1);
            }
            else {
                cprintf("   ");
            }
        }
        printc('|', RED);
        cprintf("\r\n");
    }
    
    for(int i=0; i<37; i++) {
        printc('-', RED);
    }
    cprintf("\r\n");

    textcolor(NORMAL);
    gotoxy(x_pos*4 + 2, 5 + y_pos * 2);
}

int main(int argc, char* argv[]) {
    int x_pos=0, y_pos=0;
    Sudoku NewGame;
    
    _setcursortype(_NORMALCURSOR);
    draw_screen(NewGame, x_pos, y_pos);

    while(1) {
        if (kbhit()) {
            int input = getch();
            bool need_redraw = true;

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
                            generator.generatePuzzle("easy");
                            NewGame.Clean();
                        }
                        break;
                    case KEY_F2:
                        {
                            PuzzleGenerator generator(NewGame);
                            generator.generatePuzzle("medium");
                            NewGame.Clean();
                        }
                        break;
                    case KEY_F3:
                        {
                            PuzzleGenerator generator(NewGame);
                            generator.generatePuzzle("hard");
                            NewGame.Clean();
                        }
                        break;
                    case KEY_F4:
                        {
                            PuzzleGenerator generator(NewGame);
                            generator.generatePuzzle("expert");
                            NewGame.Clean();
                        }
                        break;
                    case SHIFT_F1:
                        {
                            PuzzleGenerator generator(NewGame);
                            generator.generatePuzzle("extreme");
                            NewGame.Clean();
                        }
                        break;
                    case KEY_F5:
                        {
                            NewGame.SaveToFile("sudoku1.txt");
                            NewGame.print_debug("Game saved to sudoku1.txt");
                            break;
                        }
                        break;
                    case KEY_F6:
                        {
                            NewGame.SaveToFile("sudoku2.txt");
                            NewGame.print_debug("Game saved to sudoku2.txt");
                            break;
                        }
                        break;
                    case KEY_F7:
                        {
                            NewGame.SaveToFile("sudoku3.txt");
                            NewGame.print_debug("Game saved to sudoku3.txt");
                            break;
                        }
                        break;
                    case KEY_F8:
                        {
                            NewGame.SaveToFile("sudoku4.txt");
                            NewGame.print_debug("Game saved to sudoku4.txt");
                            break;
                        }
                        break;
                    case SHIFT_F5:
                        {
                            NewGame.LoadFromFile("sudoku1.txt");
                            NewGame.print_debug("Game saved to sudoku1.txt");
                            break;
                        }
                        break;
                    case SHIFT_F6:
                        {
                            NewGame.LoadFromFile("sudoku2.txt");
                            NewGame.print_debug("Game saved to sudoku2.txt");
                            break;
                        }
                        break;
                    case SHIFT_F7:
                        {
                            NewGame.LoadFromFile("sudoku3.txt");
                            NewGame.print_debug("Game saved to sudoku3.txt");
                            break;
                        }
                        break;
                    case SHIFT_F8:
                        {
                            NewGame.LoadFromFile("sudoku4.txt");
                            NewGame.print_debug("Game saved to sudoku4.txt");
                            break;
                        }
                        break;
                        
                    default:
                        need_redraw = false;
                        break;
                }
            } else {
                if(input >= 'a' && input <= 'z') {
                    input = input - 'a' + 'A';
                }
                
                switch(input) {
                    case 'H':
                        show_help();
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
                        return 0;
                    case 'S':
                        NewGame.StdElim();
                        break;
                    case 'L':
                        NewGame.LinElim();
                        break;
                    case 'C':
                        NewGame.FindSimpleColoring();
                        break;
                    case 'P':
                        NewGame.FindPointingPairs();
                        break;
                    case 'X':
                        NewGame.FindXWing();
                        break;
                    case 'Y':
                        NewGame.FindXYWing();
                        break;
                    case ';':
                        NewGame.FindXYZWing();
                        break;
                    case 'F':
                        NewGame.FindSwordFish();
                        break;
                    case 'N':
                        NewGame.FindHiddenSingles();
                        break;
                    case 'K':
                        NewGame.FindNakedSets();
                        break;
                    case 'A':
                        NewGame.Solve();
                        break;
                    default:
                        need_redraw = false;
                        break;
                }
            }

            if (need_redraw) {
                draw_screen(NewGame, x_pos, y_pos);
            }
        }
    }
    return 0;
}
