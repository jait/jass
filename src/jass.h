/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005, 2006 Jari Tenhunen <jait@iki.fi>
 */

#ifndef JASS_H_
#define JASS_H_

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>

typedef uint8_t num_t;
typedef int8_t coord_t;


enum 
{
    P_FALSE = 0,
    P_TRUE = 1,
    X = 9,
    Y = 9,
    BOX_X = 3,
    BOX_Y = 3,
    NR_MAX = X /* should be max(X,Y) */
};

enum
{
    MODE_NORMAL,
    MODE_STEP
};

struct point 
{
    coord_t x;
    coord_t y;
};

struct cellgroup
{
    struct point *cells; /* pointer to n_cells number of points */
    int n_cells;
};

struct ptr_array
{
    void ** ptr;
    int n;
};

void init(void);
void print_board(void);
void fix(num_t x, num_t y, num_t val);

int parse_board(const char *str);
void dump_board(FILE *stream);
int solve(void);

void usage(void);

int get_mode(void);

#endif
