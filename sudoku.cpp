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
  start_color();			/* Start color 			*/
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_BLUE, COLOR_BLACK);
  for(;;)
  {
     
    printw("Welcome to Sudoku Solver (Press S to Solve)\n\n");
    for(y=0;y<9;y++)
    {
	if(y%3==0)
	{
	    attron(COLOR_PAIR(1));
	}
	for(i=0;i<37;i++)
	{
	  printw("-");
	}
	printw("\n");
	for(x=0;x<9;x++)
	{
	    temp=NewGame.GetValue(x,y);
	    if(temp>=0 && temp<=8)
	    {
		if(x%3==0)
		{
		  attron(COLOR_PAIR(1));
		  printw("|");
		  attroff(COLOR_PAIR(1));
		}
		else
		{
		  printw("|");
		}
		attron(COLOR_PAIR(2));
		printw(" %i ", temp+1);
		attroff(COLOR_PAIR(2));	    
	    }
	    else
	    {
	      if(x%3==0)
	      {
		attron(COLOR_PAIR(1));
		printw("|");
		attroff(COLOR_PAIR(1));
	      }
	      else
	      {
		printw("|");
	      }
	      attron(COLOR_PAIR(2));
	      printw("   ");
	      attroff(COLOR_PAIR(2));
	    }
	  
	}
      attron(COLOR_PAIR(1));
      printw("|\n");
      attroff(COLOR_PAIR(1));
    }
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
	if(NewGame.LegalValue(x_pos, y_pos, input-'1')==TRUE)
	{
	  NewGame.SetValue(x_pos, y_pos, input - '1');
	}
	else
	{
	  move(23,0);
	  printw("Illegal Value\n");
	}
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
