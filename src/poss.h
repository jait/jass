/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005, 2006 Jari Tenhunen <jait@iki.fi>
 */

#ifndef POSS_H_
#define POSS_H_

#include <inttypes.h>
#include "jass.h"

typedef uint16_t poss_t;

inline char is_poss(num_t y, num_t x, num_t val);
inline char set_poss(num_t y, num_t x, num_t candidate, char val);

inline poss_t get_poss(coord_t y, coord_t x);

inline int poss_contains_set(coord_t y, coord_t x, poss_t set);

num_t get_only_poss(coord_t y, coord_t x);

inline int poss_equals(coord_t y1, coord_t x1, coord_t y2, coord_t x2);

#endif
