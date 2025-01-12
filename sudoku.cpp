#include <iostream>
using namespace std;
#include <stdlib.h>
#include <ncurses.h>

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
    
    bool LegalValue(int x, int y, int value);
  private:

    int EliminatePossibility(int x, int y, int value);
    int board[9][9][9];
    int FindHiddenPairs();
    int FindXWing();
    int StdElim();
    int LinElim();
    int Clean();
};

int main(void)
{
  int i, x, y, temp, input, x_pos=0, y_pos=0;
  Sudoku NewGame;

  initscr();
  keypad(stdscr, true);
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);    // 3x3 borders
  init_pair(2, COLOR_BLUE, COLOR_BLACK);    // Numbers
  init_pair(3, COLOR_WHITE, COLOR_BLACK);   // Inner grid lines
  
  for(;;)
  {
    printw("Welcome to Sudoku Solver (Press 's' to solve, 0 to clear, 'q' to quit, and numbers to fill in the current position)\n\n");
    
    // Draw the grid
    for(y=0;y<9;y++)
    {
        // Draw horizontal lines
        if(y%3==0)
        {
            // Draw 3x3 border lines in blue
            attron(COLOR_PAIR(1));
            for(i=0;i<37;i++)
            {
                printw("-");
            }
            attroff(COLOR_PAIR(1));
        }
        else
        {
            // Draw inner horizontal lines in white
            attron(COLOR_PAIR(3));
            for(i=0;i<37;i++)
            {
                if(i%4==0)  // Position for vertical lines
                {
                    if(i%12==0)  // 3x3 border positions
                    {
                        attroff(COLOR_PAIR(3));
                        attron(COLOR_PAIR(1));
                        printw("+");
                        attroff(COLOR_PAIR(1));
                        attron(COLOR_PAIR(3));
                    }
                    else
                    {
                        printw("+");
                    }
                }
                else
                {
                    printw("-");
                }
            }
            attroff(COLOR_PAIR(3));
        }
        printw("\n");
        
        // Draw cells and vertical lines
        for(x=0;x<9;x++)
        {
            // Draw vertical lines
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
            
            // Draw cell content
            temp=NewGame.GetValue(x,y);
            if(temp>=0 && temp<=8)
            {
                attron(COLOR_PAIR(2));
                printw(" %i ", temp+1);
                attroff(COLOR_PAIR(2));
            }
            else
            {
                printw("   ");
            }
        }
        
        // Draw final vertical line of each row
        attron(COLOR_PAIR(1));
        printw("|\n");
        attroff(COLOR_PAIR(1));
    }
    
    // Draw bottom border
    attron(COLOR_PAIR(1));
    for(i=0;i<37;i++)
    {
        printw("-");
    }
    attroff(COLOR_PAIR(1));

    move(y_pos*2+3,x_pos*4+2);
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
	move(23,0);
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
	/*if(NewGame.LegalValue(x_pos, y_pos, input-'1')==TRUE)
	{
	  NewGame.SetValue(x_pos, y_pos, input - '1');
	}
	else
	{
	  move(23,0);
	  printw("Illegal Value\n");
	}*/
	break;
      case 'Q':
	endwin();
	return 0;
	break;
      case 'S':
	NewGame.Solve();
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

int Sudoku::Solve() 
{
  int stop;
  int counter1, counter2, i, j;
  Clean();
  do
  {
    counter1=0;
    counter2=0;
    for(i=0;i<9;i++)
    {
      for(j=0;j<9;j++)
      {
	if(GetValue(i,j)!=-1)
	{
	  counter1++;
	}
      }
    }
    if(counter1!=81)
    {
      do
      {
	stop=StdElim();
      }while(stop==0);
      LinElim();
      FindHiddenPairs();
      FindXWing();
      for(i=0;i<9;i++)
      {
	for(j=0;j<9;j++)
	{
	  if(GetValue(i,j)!=-1)
	  {
	    counter2++;
	  }
	}
      }
    }
    else
    {
      counter2=81;
    }
  }while(counter1!=counter2);
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
    
    // Check for X-Wing patterns in rows
    for(int val = 0; val < 9; val++) {
        for(int row1 = 0; row1 < 8; row1++) {
            for(int row2 = row1 + 1; row2 < 9; row2++) {
                // Find positions where val can appear in row1
                int cols1[2], cols2[2];
                int count1 = 0, count2 = 0;
                
                // Find candidates in row1
                for(int col = 0; col < 9; col++) {
                    if(GetValue(row1, col) == -1 && board[row1][col][val] == val) {
                        if(count1 < 2) {
                            cols1[count1++] = col;
                        } else {
                            count1++;
                            break;
                        }
                    }
                }
                
                // If exactly 2 positions in row1
                if(count1 == 2) {
                    // Check if same columns in row2 also have exactly 2 positions
                    for(int col = 0; col < 9; col++) {
                        if(GetValue(row2, col) == -1 && board[row2][col][val] == val) {
                            if(count2 < 2) {
                                cols2[count2++] = col;
                            } else {
                                count2++;
                                break;
                            }
                        }
                    }
                    
                    // If we found an X-Wing pattern
                    if(count2 == 2 && cols1[0] == cols2[0] && cols1[1] == cols2[1]) {
                        // Eliminate val from other cells in these columns
                        for(int row = 0; row < 9; row++) {
                            if(row != row1 && row != row2) {
                                // Check both columns
                                for(int i = 0; i < 2; i++) {
                                    int col = cols1[i];
                                    if(board[row][col][val] != -1) {
                                        board[row][col][val] = -1;
                                        changed++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Check for X-Wing patterns in columns
    for(int val = 0; val < 9; val++) {
        for(int col1 = 0; col1 < 8; col1++) {
            for(int col2 = col1 + 1; col2 < 9; col2++) {
                // Find positions where val can appear in col1
                int rows1[2], rows2[2];
                int count1 = 0, count2 = 0;
                
                // Find candidates in col1
                for(int row = 0; row < 9; row++) {
                    if(GetValue(row, col1) == -1 && board[row][col1][val] == val) {
                        if(count1 < 2) {
                            rows1[count1++] = row;
                        } else {
                            count1++;
                            break;
                        }
                    }
                }
                
                // If exactly 2 positions in col1
                if(count1 == 2) {
                    // Check if same rows in col2 also have exactly 2 positions
                    for(int row = 0; row < 9; row++) {
                        if(GetValue(row, col2) == -1 && board[row][col2][val] == val) {
                            if(count2 < 2) {
                                rows2[count2++] = row;
                            } else {
                                count2++;
                                break;
                            }
                        }
                    }
                    
                    // If we found an X-Wing pattern
                    if(count2 == 2 && rows1[0] == rows2[0] && rows1[1] == rows2[1]) {
                        // Eliminate val from other cells in these rows
                        for(int col = 0; col < 9; col++) {
                            if(col != col1 && col != col2) {
                                // Check both rows
                                for(int i = 0; i < 2; i++) {
                                    int row = rows1[i];
                                    if(board[row][col][val] != -1) {
                                        board[row][col][val] = -1;
                                        changed++;
                                    }
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

int Sudoku::FindHiddenPairs() {
    int changed = 0;
    
    // For each row
    for(int row = 0; row < 9; row++) {
        // Try each pair of values
        for(int val1 = 0; val1 < 8; val1++) {
            for(int val2 = val1 + 1; val2 < 9; val2++) {
                // Find which positions could contain val1 or val2
                int pos1 = -1;
                int pos2 = -1;
                int count = 0;
                
                // Check each position in this row
                for(int col = 0; col < 9; col++) {
                    // If cell is empty and could contain val1 or val2
                    if(GetValue(row, col) == -1 && 
                       (board[row][col][val1] == val1 || board[row][col][val2] == val2)) {
                        count++;
                        if(pos1 == -1)
                            pos1 = col;
                        else
                            pos2 = col;
                    }
                }
                
                // Found a hidden pair - exactly 2 positions can take these values
                if(count == 2) {
                    // Clear all other values from these two cells
                    bool cleared = false;
                    for(int val = 0; val < 9; val++) {
                        if(val != val1 && val != val2) {
                            if(board[row][pos1][val] != -1) {
                                board[row][pos1][val] = -1;
                                cleared = true;
                            }
                            if(board[row][pos2][val] != -1) {
                                board[row][pos2][val] = -1;
                                cleared = true;
                            }
                        }
                    }
                    if(cleared) changed++;
                }
            }
        }
    }
    
    // Same thing for columns
    for(int col = 0; col < 9; col++) {
        for(int val1 = 0; val1 < 8; val1++) {
            for(int val2 = val1 + 1; val2 < 9; val2++) {
                int pos1 = -1;
                int pos2 = -1;
                int count = 0;
                
                for(int row = 0; row < 9; row++) {
                    if(GetValue(row, col) == -1 && 
                       (board[row][col][val1] == val1 || board[row][col][val2] == val2)) {
                        count++;
                        if(pos1 == -1)
                            pos1 = row;
                        else
                            pos2 = row;
                    }
                }
                
                if(count == 2) {
                    bool cleared = false;
                    for(int val = 0; val < 9; val++) {
                        if(val != val1 && val != val2) {
                            if(board[pos1][col][val] != -1) {
                                board[pos1][col][val] = -1;
                                cleared = true;
                            }
                            if(board[pos2][col][val] != -1) {
                                board[pos2][col][val] = -1;
                                cleared = true;
                            }
                        }
                    }
                    if(cleared) changed++;
                }
            }
        }
    }
    
    // Same for 3x3 boxes
    for(int box = 0; box < 9; box++) {
        int startRow = (box / 3) * 3;
        int startCol = (box % 3) * 3;
        
        for(int val1 = 0; val1 < 8; val1++) {
            for(int val2 = val1 + 1; val2 < 9; val2++) {
                int row1 = -1, col1 = -1;
                int row2 = -1, col2 = -1;
                int count = 0;
                
                for(int i = 0; i < 3; i++) {
                    for(int j = 0; j < 3; j++) {
                        int row = startRow + i;
                        int col = startCol + j;
                        if(GetValue(row, col) == -1 && 
                           (board[row][col][val1] == val1 || board[row][col][val2] == val2)) {
                            count++;
                            if(row1 == -1) {
                                row1 = row;
                                col1 = col;
                            } else {
                                row2 = row;
                                col2 = col;
                            }
                        }
                    }
                }
                
                if(count == 2) {
                    bool cleared = false;
                    for(int val = 0; val < 9; val++) {
                        if(val != val1 && val != val2) {
                            if(board[row1][col1][val] != -1) {
                                board[row1][col1][val] = -1;
                                cleared = true;
                            }
                            if(board[row2][col2][val] != -1) {
                                board[row2][col2][val] = -1;
                                cleared = true;
                            }
                        }
                    }
                    if(cleared) changed++;
                }
            }
        }
    }
    
    return changed;
}

int Sudoku::StdElim()
{

  int x, y, temp, i, j, section1, section2, counter1=0, counter2=0, returned;
  for(x=0;x<9;x++)
  {
    for(y=0;y<9;y++)
    {
      temp=GetValue(x,y);
      if(temp>=0 && temp<=8)
      {
	counter1++;
      }
    }
  }
  for(y=0;y<9;y++)
  {
    for(x=0;x<9;x++)
    {
      temp=GetValue(x, y);
      if(temp>=0 && temp<=8)
      {
	for(i=0;i<x;i++)
	{
	  EliminatePossibility(i, y, temp);
	}
	for(i=x+1;i<9;i++)
	{
	  EliminatePossibility(i, y, temp);
	}
	for(i=0;i<y;i++)
	{
	  EliminatePossibility(x, i, temp);
	}
	for(i=y+1;i<9;i++)
	{
	  EliminatePossibility(x, i, temp);
	}

	/*  Square Elimination */
	
	section1=(x/3)*3;
	section2=(y/3)*3;
	for(i=0;i<3;i++)
	{
	  for(j=0;j<3;j++)
	  {
	    if(x!=(section1+j) || y!=(section2+i))
	    {
	      EliminatePossibility(section1+j, section2+i, temp);
	    }
	  }
	}
      }
    }
  }
  for(x=0;x<9;x++)
  {
    for(y=0;y<9;y++)
    {
      temp=GetValue(x,y);
      if(temp>=0 && temp<=8)
      {
	counter2++;
      }
    }
  }
  if(counter1==counter2)
  {
    returned=-1;
  }
  else
  {
    returned=0;
  }
  return returned;
}

int Sudoku::LinElim()
{
  int v, w, x, y, k, xpos, ypos, counter=0;
  for(k=0;k<9;k++)
  {
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
	    if(board[x][y][k]==k)
	    {
	      xpos=x;
	      ypos=y;
	      counter++;
	    }
	  }
	}
	if(counter==1)
	{
	  SetValue(xpos, ypos, k);
	}
      }
    }
  }

  for(k=0;k<9;k++)
  {
    for(x=0;x<9;x++)
    {
      xpos=-1;
      ypos=-1;
      counter=0;
    
      for(y=0;y<9;y++)
      {
	if(board[x][y][k]==k)
	{
	  xpos=x;
	  ypos=y;
	  counter++;
	 }
      }
      if(counter==1)
      {
	SetValue(xpos, ypos, k);
      }
    }
  }

  for(k=0;k<9;k++)
  {
    for(y=0;y<9;y++)
    {
      xpos=-1;
      ypos=-1;
      counter=0;
  
      for(x=0;x<9;x++)
      {
	if(board[x][y][k]==k)
	{
	  xpos=x;
	  ypos=y;
	  counter++;
	 }
      }
      if(counter==1)
      {
	SetValue(xpos, ypos, k);
      }
    }
  }


  return 0;
}
