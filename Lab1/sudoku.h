#ifndef SUDOKU_H
#define SUDOKU_H

const bool DEBUG_MODE = false;
enum { ROW=9, COL=9, N = 81, NEIGHBOR = 20 };
const int NUM = 9;

extern int neighbors[N][NEIGHBOR];
extern int board[N];
extern int spaces[N];
extern int nspaces;
extern int (*chess)[COL];

void init_neighbors();
int  input(const char in[N],int board[N],int spaces[N]);
void init_cache(bool occupied[N][NUM+1],int arity[N],int board[N]);

bool available(int guess, int cell);
bool solve_sudoku_min_arity_cache(int which_space,
  bool occupied[N][NUM+1],int arity[N],int board[N],int spaces[N],int nspaces);
bool solved(int board[N]);
#endif
