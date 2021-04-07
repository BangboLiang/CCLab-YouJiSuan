#include <assert.h>
#include <strings.h>
#include <stdio.h>
#include <algorithm>

#include "sudoku.h"

int neighbors[N][NEIGHBOR];
//static bool occupied[N][NUM+1];
//static int arity[N];

//int board[N];
//int spaces[N];
//int nspaces;
//int (*chess)[COL] = (int (*)[COL])board;
static void print_neighbors(const bool adjacent[ROW][COL], int row, int col, int myneighbors[NEIGHBOR])
{
  for (int y = 0; y < ROW; ++y) {
    for (int x = 0; x < COL; ++x) {
      if (y == row && x == col)
        putchar('X');
      else
        putchar(adjacent[y][x] ? 'o' : '.');
    }
    printf("\n");
  }
  for (int i = 0; i < NEIGHBOR; ++i) {
    printf("%2d, ", myneighbors[i]);
  }
  puts("\n");
}

static void collect_neighbors(const bool adjacent[ROW][COL], int row, int col, int myneighbors[NEIGHBOR])
{
  int n = 0;
  for (int y = 0; y < ROW; ++y) {
    for (int x = 0; x < COL; ++x) {
      if (adjacent[y][x] && !(y == row && x == col)) {
        assert(n < NEIGHBOR);
        myneighbors[n++] = y*COL + x;
      }
    }
  }
  assert(n == NEIGHBOR);
}

static void mark_adjacent(bool adjacent[ROW][COL], int row, int col)
{
  for (int i = 0; i < NUM; ++i) {
    adjacent[row][i] = true;
    adjacent[i][col] = true;
  }
  int top = (row/3)*3;
  int left = (col/3)*3;
  adjacent[top][left] = true;
  adjacent[top][left+1] = true;
  adjacent[top][left+2] = true;
  adjacent[top+1][left] = true;
  adjacent[top+1][left+1] = true;
  adjacent[top+1][left+2] = true;
  adjacent[top+2][left] = true;
  adjacent[top+2][left+1] = true;
  adjacent[top+2][left+2] = true;
}

/*public*/ void init_neighbors()
{
  for (int row = 0; row < ROW; ++row) {
    for (int col = 0; col < COL; ++col) {
      bool adjacent[ROW][COL];
      bzero(adjacent, sizeof adjacent);
      mark_adjacent(adjacent, row, col);

      int me = row*COL + col;
      collect_neighbors(adjacent, row, col, neighbors[me]);

      if (DEBUG_MODE)
        print_neighbors(adjacent, row, col, neighbors[me]);
    }
  }
}

bool solved(int board[N])
{
  int (*chess)[COL] = (int (*)[COL])board;
  for (int row = 0; row < ROW; ++row) {
    // check row
    int occurs[10] = { 0 };
    for (int col = 0; col < COL; ++col) {
      int val = chess[row][col];
      assert(1 <= val && val <= NUM);
      ++occurs[val];
    }

    if (std::count(occurs, occurs+10, 1) != NUM)
      return false;
  }

  for (int col = 0; col < COL; ++col) {
    int occurs[10] = { 0 };
    for (int row = 0; row < ROW; ++row) {
      int val = chess[row][col];
      // assert(1 <= val && val <= NUM);
      ++occurs[val];
    }

    if (std::count(occurs, occurs+10, 1) != NUM)
      return false;
  }

  for (int row = 0; row < ROW; row += 3) {
    for (int col = 0; col < COL; col += 3) {
      int occurs[10] = { 0 };
      ++occurs[chess[row  ][col]];
      ++occurs[chess[row  ][col+1]];
      ++occurs[chess[row  ][col+2]];
      ++occurs[chess[row+1][col]];
      ++occurs[chess[row+1][col+1]];
      ++occurs[chess[row+1][col+2]];
      ++occurs[chess[row+2][col]];
      ++occurs[chess[row+2][col+1]];
      ++occurs[chess[row+2][col+2]];

      if (std::count(occurs, occurs+10, 1) != NUM)
        return false;
    }
  }
  return true;
}


static int find_spaces(int board[N],int spaces[N],int nspaces)
{
  nspaces = 0;
  for (int cell = 0; cell < N; ++cell) {
    if (board[cell] == 0)
      spaces[nspaces++] = cell;
  }
  return nspaces;
}

int input(const char in[N],int board[N],int spaces[N])
{
  //printf("input start!\n");
  for (int cell = 0; cell < N; ++cell) {
    board[cell] = in[cell] - '0';
    assert(0 <= board[cell] && board[cell] <= NUM);
  }
  //printf("input over,begin to find spaces\n");
  int t = find_spaces(board,spaces,0);
  return t;
}

bool available(int guess, int cell,int board[N])
{
  for (int i = 0; i < NEIGHBOR; ++i) {
    int neighbor = neighbors[cell][i];
    if (board[neighbor] == guess) {
      return false;
    }
  }
  return true;
}

static void find_min_arity(int space,int arity[N],int spaces[N],int nspaces)
{
  int cell = spaces[space];
  int min_space = space;
  int min_arity = arity[cell];

  for (int sp = space+1; sp < nspaces && min_arity > 1; ++sp) {
    int cur_arity = arity[spaces[sp]];
    if (cur_arity < min_arity) {
      min_arity = cur_arity;
      min_space = sp;
    }
  }

  if (space != min_space) {
    std::swap(spaces[min_space], spaces[space]);
  }
}

void init_cache(bool occupied[N][NUM+1],int arity[N],int board[N])
{
  //bzero(occupied, sizeof(occupied));
  for (int i = 0; i < N; ++i){
    for (int j = 0;j < NUM+1; j++){
      occupied[i][j] = 0;
    }
  }
  std::fill(arity, arity + N, NUM);
  for (int cell = 0; cell < N; ++cell) {
    occupied[cell][0] = true;
    int val = board[cell];
    if (val > 0) {
      occupied[cell][val] = true;
      for (int n = 0; n < NEIGHBOR; ++n) {
        int neighbor = neighbors[cell][n];
        if (!occupied[neighbor][val]) {
          occupied[neighbor][val] = true;
          --arity[neighbor];
        }
      }
    }
  }
}

bool solve_sudoku_min_arity_cache(int which_space,
  bool occupied[N][NUM+1],int arity[N],int board[N],int spaces[N],int nspaces)
{
  if (which_space >= nspaces) {
    return true;
  }

  find_min_arity(which_space,arity,spaces,nspaces);
  int cell = spaces[which_space];

  for (int guess = 1; guess <= NUM; ++guess) {
    if (!occupied[cell][guess]) {
      // hold
      assert(board[cell] == 0);
      board[cell] = guess;
      occupied[cell][guess] = true;

      // remember changes
      int modified[NEIGHBOR];
      int nmodified = 0;
      for (int n = 0; n < NEIGHBOR; ++n) {
        int neighbor = neighbors[cell][n];
        if (!occupied[neighbor][guess]) {
          occupied[neighbor][guess] = true;
          --arity[neighbor];
          modified[nmodified++] = neighbor;
        }
      }

      // try
      if (solve_sudoku_min_arity_cache(which_space+1,occupied,arity,board,spaces,nspaces)) {
        return true;
      }

      // unhold
      occupied[cell][guess] = false;
      assert(board[cell] == guess);
      board[cell] = 0;

      // undo changes
      for (int i = 0; i < nmodified; ++i) {
        int neighbor = modified[i];
        assert(occupied[neighbor][guess]);
        occupied[neighbor][guess] = false;
        ++arity[neighbor];
      }
    }
  }
  return false;
}
