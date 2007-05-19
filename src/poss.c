/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005, 2006 Jari Tenhunen <jait@iki.fi>
 */

#include <string.h>
#include "jass.h"
#include "poss.h"

static num_t poss[Y][X][NR_MAX];

inline char is_poss(num_t y, num_t x, num_t candidate)
{
    return poss[y][x][candidate];
    /* return (poss[y][x] & (1 << candidate)); */
}

/*
 * Sets candidate to be possible or not possible for cell (x,y)
 *
 */
inline char set_poss(num_t y, num_t x, num_t candidate, char val)
{
    char prev = poss[y][x][candidate];
    poss[y][x][candidate] = val;
    return prev;

    /* bit-vector version */
    /*
    char prev = is_poss(y, x, candidate);
    if (val)
        poss[y][x] |= (1 << candidate); // set
    else
        poss[y][x] &= ~(1 << candidate); // clear

    return prev;
    */
}

/**
 * Return the only possibility (1...NR_MAX) or zero when there are zero or more than one
 * possibility
 */
num_t get_only_poss(coord_t y, coord_t x)
{
    num_t k, val = 0;

    for (k = 0; k < NR_MAX; ++k) 
    {
        if (is_poss(y,x,k)) 
        {
            if (val != 0) 
            {
                /* at least two possibilities */
                val = 0;
                break;
            }
            val = k + 1; /* because val is 1... and index k is 0... */
        }
    }
    return val;
}

int poss_get_pair(coord_t y, coord_t x, num_t *a, num_t *b)
{
    return 0;
}

inline int poss_equals(coord_t y1, coord_t x1, coord_t y2, coord_t x2)
{
    return (memcmp(poss[y1][x1], poss[y2][x2], NR_MAX) == 0);
}

/**
 * Bit-vector versions
 */
inline poss_t get_poss(coord_t y, coord_t x)
{
    return 0;
}

inline int poss_contains_set(coord_t y, coord_t x, poss_t set)
{
    return ((0 & set) == set);
}
