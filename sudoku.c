/*
 * 
 * Project name: sudoku.c
 * CSS430 Project 2 : Multi-Threaded Sudoku Validator
 * Student Name: Eunsol Cho
 * Student e-mail: echo2@uw.edu
 * Date of Last Modification: 7/23/2021
 * Description: This project consists of designing a multithreaded application that verifies a Sudoku puzzle of any size is valid AND whether a Sudoku puzzle that has 0s instead of numbers can be completed to be a valid puzzle.
 * complie: gcc sudoku.c -pthread -lm
 * run: ./a.out puzzle.txt
 * 
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// structure to pass the row and column 
typedef struct {
  int row;
  int column;
  int **grid;
  int psize;

  int fill; 
} parameters;

int status[3] = {}; // to check status for complete thread, row thread, and column thread
int zeroCount = 0;

// checks if the grid is complete (no 0's)
void *checkComplete(void *data) {
  parameters *param = (parameters *)data;
  for (int i = 1; i <= param->psize; i++) {
    for (int j =1; j <= param->psize; j++)
    {
      if (param->grid[i][j] == 0) {
        status[0] = -1; // -1 is for incomplete puzzle
        pthread_exit(0);
      }
    }
  }
  status[0] = 1; // 1 is for complete puzzle
  pthread_exit(0);
}

// checks if rows of grid are valid
void *checkValidRows(void *data) {
  parameters *param = (parameters *)data;
  int checkDigit[param->psize];
  for (int i = 1; i <= param->psize; i++) {
    for (int j = 1; j <= param->psize; j++)
    {
      if (checkDigit[param->grid[i][j] - 1] == 1) {
        pthread_exit(0); // not valid row
      }
      checkDigit[param->grid[i][j] - 1] = 1;
    }
    memset(checkDigit, 0, sizeof(checkDigit));
  }
  status[1] = 1; // valid row
  pthread_exit(0);
}

// checks if columns of grid are valid
void *checkValidColumns(void *data) {
  parameters *param = (parameters *)data;
  int checkDigit[param->psize];
  for (int i = 1; i <= param->psize; i++) {
    for (int j =1; j <= param->psize; j++)
    {
      if (checkDigit[param->grid[j][i] - 1] == 1) {
        pthread_exit(0); // not valid column
      }
      checkDigit[param->grid[j][i] - 1] = 1;
    }
    memset(checkDigit, 0, sizeof(checkDigit));
  }
  status[2] = 1; // valid column
  pthread_exit(0);
}

// checks if box of grid is valid
void *checkValidBox(void *data) {
  parameters *param = (parameters *)data;
  int checkDigit[param->psize];
  for (int i = param->row; i < param->row + sqrt(param->psize); i++) {
    for (int j = param->column; j < param->column + sqrt(param->psize); j++) {
      if (checkDigit[param->grid[i][j]-1] == 1) {
        return (void *) 0; // not valid box
      }
      checkDigit[param->grid[i][j]-1] = 1;
    }
    memset(checkDigit, 0, sizeof(checkDigit));
  }
  return (void *) 1; // valid sudoku box
}


// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity

void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
// checks if the grid is complete
  pthread_t tid_complete;
  parameters *data_complete = (parameters *) malloc(sizeof(parameters));
  data_complete->row = 1;
  data_complete->column = 1; 
  data_complete->psize = psize;
  data_complete->grid = grid;

  pthread_create(&tid_complete, NULL, checkComplete, (void *) data_complete);
  pthread_join(tid_complete, NULL);

  if (status[0] == 1) {
    *complete = true;
  }
  if (complete) {
// checks if rows are valid
    pthread_t tid_rows;
    parameters *data_rows = (parameters *) malloc(sizeof(parameters));
    data_rows->row = 1;
    data_rows->column = 1;
    data_rows->psize = psize;
    data_rows->grid = grid;

    pthread_create(&tid_rows, NULL, checkValidRows, (void *) data_rows);
    pthread_join(tid_rows, NULL);
// checks if columns are valid
    pthread_t tid_columns;
    parameters *data_columns = (parameters *) malloc(sizeof(parameters));
    data_columns->row = 1;
    data_columns->column = 1;
    data_columns->psize = psize;
    data_columns->grid = grid;

    pthread_create(&tid_columns, NULL, checkValidColumns, (void *) data_columns);
    pthread_join(tid_columns, NULL);
// checks if boxes are valid
    pthread_t tid_box[psize];
    int tid_box_status[psize];

    double s = sqrt(psize);
    int i =0;
    parameters *data_box[psize];
    for (int r = 1; r <= psize; r += s) {
      for (int c = 1; c <= psize; c += s, i++) {
        data_box[i] = (parameters *) malloc(sizeof(parameters));
        data_box[i]->row = r;
        data_box[i]->column = c;
        data_box[i]->psize = psize;
        data_box[i]->grid = grid;

      }
    }

    int checkAll;
    for (int i = 0; i < psize; i++) {
      pthread_create(&tid_box[i], NULL, checkValidBox, (void *)data_box[i]);
    }
    // exit if box is not valid
    for (int i = 0; i < psize; i++) {
      if (pthread_join(tid_box[i], NULL) == checkAll) {
        *valid = false;
        exit(EXIT_FAILURE);
      }
    }
    if (status[1] == 1 && status[2] == 1) {
      *valid = true;
    }
  }
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}


// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }

  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}

