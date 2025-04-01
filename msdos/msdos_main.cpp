#include <algorithm>
#include <conio.h>
#include <cstring>
#include <ctime>
#include <dos.h>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "generatepuzzle.h"
#include "sudoku.h"

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
#define KEY_F9 67
#define KEY_F10 68
#define KEY_F11 69
#define KEY_F12 70

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
  cputs(" Arrow keys - Move cursor    F5/Shift - Save/Load Slot 1\r\n");
  cputs(" 1-9       - Fill number     F6/Shift - Save/Load Slot 2\r\n");
  cputs(" 0         - Clear cell      F7/Shift - Save/Load Slot 3\r\n");
  cputs(" Q         - Quit game       F8/Shift - Save/Load Slot 4\r\n");
  cputs("\r\n");
  cputs("Puzzle Generation:\r\n");
  cputs(" F1          - Generate Easy puzzle\r\n");
  cputs(" F2          - Generate Medium puzzle\r\n");
  cputs(" F3          - Generate Hard puzzle\r\n");
  cputs(" F4          - Generate Expert puzzle\r\n");
  cputs(" Shift+F1    - Generate Extreme puzzle\r\n");
  cputs(" F5-F8       - Save Puzzle\r\n");
  cputs(" Shift F5-F8 - Load Puzzle\r\n");
  cputs(" F9-F12      - Save Puzzle as XML Spreadsheet (Excel compatible)\r\n");
  cputs("\r\n");
  cputs("Solving Techniques:\r\n");
  cputs(" S - Standard elimination    N - Hidden singles    Y - Find XY "
        "Wing\r\n");
  cputs(" L - Line elimination        K - Naked sets        ; - Find XYZ "
        "Wing\r\n");
  cputs(" I - Hidden pairs            X - X-Wing            C - Simple "
        "Coloring\r\n");
  cputs(" P - Pointing pairs          F - Swordfish         Z - New Game\r\n");
  cputs(" A - Run all techniques\r\n");
  cputs("\r\n");
  cputs("Press any key to return to game...");
  getch();
  clrscr();
}

void draw_screen(Sudoku &NewGame, int x_pos, int y_pos) {
  // clrscr();
  gotoxy(1, 1);

  textcolor(NORMAL);

  // Print header
  cputs("Welcome to Sudoku Solver for MS-DOS\r\n");
  cputs("Press (H) for Help\r\n\r\n");

  // Draw the grid
  for (int y = 0; y < 9; y++) {
    if (y % 3 == 0) {
      for (int i = 0; i < 37; i++) {
        printc('-', RED);
      }
    } else {
      for (int i = 0; i < 37; i++) {
        if (i % 12 == 0) {
          printc('+', RED); // 3x3 intersection
        } else if (i % 4 == 0) {
          printc('+', NORMAL); // Regular intersection
        } else {
          printc('-', NORMAL); // Regular horizontal line
        }
      }
    }
    cprintf("\r\n");

    for (int x = 0; x < 9; x++) {
      if (x % 3 == 0) {
        printc('|', RED);
      } else {
        printc('|', NORMAL);
      }

      int temp = NewGame.GetValue(x, y);
      textcolor(NORMAL);
      if (temp >= 0 && temp <= 8) {
        cprintf(" %d ", temp + 1);
      } else {
        cprintf("   ");
      }
    }
    printc('|', RED);
    cprintf("\r\n");
  }

  for (int i = 0; i < 37; i++) {
    printc('-', RED);
  }
  cprintf("\r\n");

  textcolor(NORMAL);
  gotoxy(x_pos * 4 + 2, 5 + y_pos * 2);
}

int main(int argc, char *argv[]) {
  int x_pos = 0, y_pos = 0;
  Sudoku NewGame;

  _setcursortype(_NORMALCURSOR);
  draw_screen(NewGame, x_pos, y_pos);

  while (1) {
    if (kbhit()) {
      int input = getch();
      bool need_redraw = true;

      if (input == 0) {
        input = getch();
        switch (input) {
        case KEY_LEFT:
          x_pos = (x_pos + 8) % 9;
          break;
        case KEY_RIGHT:
          x_pos = (x_pos + 1) % 9;
          break;
        case KEY_UP:
          y_pos = (y_pos + 8) % 9;
          break;
        case KEY_DOWN:
          y_pos = (y_pos + 1) % 9;
          break;
        case KEY_F1: {
          PuzzleGenerator generator(NewGame);
          NewGame.print_debug("Generating easy puzzle.  Be patient.");
          if (generator.generatePuzzle("easy")) {
            NewGame.print_debug("Successfully generated easy puzzle");
          } else {
            NewGame.print_debug("Failed to generate easy puzzle");
          }
          NewGame.Clean();
        } break;
        case KEY_F2: {
          NewGame.print_debug("Generating medium puzzle.  Be patient.");
          PuzzleGenerator generator(NewGame);
          if (generator.generatePuzzle("medium")) {
            NewGame.print_debug("Successfully generated medium puzzle");
          } else {
            NewGame.print_debug("Failed to generate medium puzzle");
          }
          NewGame.Clean();
        } break;
        case KEY_F3: {
          PuzzleGenerator generator(NewGame);
          NewGame.print_debug("Generating hard puzzle.  Be patient.");
          if (generator.generatePuzzle("hard")) {
            NewGame.print_debug("Successfully generated hard puzzle");
          } else {
            NewGame.print_debug("Failed to generate hard puzzle");
          }
          NewGame.Clean();
        } break;
        case KEY_F4: {
          PuzzleGenerator generator(NewGame);
          NewGame.print_debug("Generating expert puzzle.  Be patient.");
          if (generator.generatePuzzle("expert")) {
            NewGame.print_debug("Successfully generated expert puzzle");
          } else {
            NewGame.print_debug("Failed to generate expert puzzle");
          }
          NewGame.Clean();
        } break;
        case SHIFT_F1: {
          PuzzleGenerator generator(NewGame);
          NewGame.print_debug("Generating extreme puzzle.  Be patient.");
          if (generator.generatePuzzle("extreme")) {
            NewGame.print_debug("Successfully generated extreme puzzle");
          } else {
            NewGame.print_debug("Failed to generate extreme puzzle");
          }
          NewGame.Clean();
        } break;
        case KEY_F5: {
          NewGame.SaveToFile("sudoku1.txt");
          NewGame.print_debug("Game saved to sudoku1.txt");
          break;
        } break;
        case KEY_F6: {
          NewGame.SaveToFile("sudoku2.txt");
          NewGame.print_debug("Game saved to sudoku2.txt");
          break;
        } break;
        case KEY_F7: {
          NewGame.SaveToFile("sudoku3.txt");
          NewGame.print_debug("Game saved to sudoku3.txt");
          break;
        } break;
        case KEY_F8: {
          NewGame.SaveToFile("sudoku4.txt");
          NewGame.print_debug("Game saved to sudoku4.txt");
          break;
        } break;
        case KEY_F9: {
          NewGame.ExportToExcelXML("puzzle1.xml");
          NewGame.print_debug("Puzzle Saved to puzzle1.xml");
          break;
        } break;
        case KEY_F10: {
          NewGame.ExportToExcelXML("puzzle2.xml");
          NewGame.print_debug("Puzzle Saved to puzzle2.xml");
          break;
        } break;
        case KEY_F11: {
          NewGame.ExportToExcelXML("puzzle3.xml");
          NewGame.print_debug("Puzzle Saved to puzzle3.xml");
          break;
        } break;
        case KEY_F12: {
          NewGame.ExportToExcelXML("puzzle4.xml");
          NewGame.print_debug("Puzzle Saved to puzzle4.xml");
          break;
        } break;
        case SHIFT_F5: {
          NewGame.LoadFromFile("sudoku1.txt");
          NewGame.print_debug("Game saved to sudoku1.txt");
          break;
        } break;
        case SHIFT_F6: {
          NewGame.LoadFromFile("sudoku2.txt");
          NewGame.print_debug("Game saved to sudoku2.txt");
          break;
        } break;
        case SHIFT_F7: {
          NewGame.LoadFromFile("sudoku3.txt");
          NewGame.print_debug("Game saved to sudoku3.txt");
          break;
        } break;
        case SHIFT_F8: {
          NewGame.LoadFromFile("sudoku4.txt");
          NewGame.print_debug("Game saved to sudoku4.txt");
          break;
        } break;

        default:
          need_redraw = false;
          break;
        }
      } else {
        if (input >= 'a' && input <= 'z') {
          input = input - 'a' + 'A';
        }

        switch (input) {
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
        case 'I':
          NewGame.FindHiddenPairs();
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
        case 'A': {
          NewGame.print_debug("Running all solving techniques...");
          NewGame.Solve();
          // Force a complete screen refresh
          clrscr();
          draw_screen(NewGame, x_pos, y_pos);
        } break;
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
