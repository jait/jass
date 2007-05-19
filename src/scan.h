/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005-2007 Jari Tenhunen <jait@iki.fi>
 */

#ifndef SCAN_H_
#define SCAN_H_

#include "jass.h"

typedef int (*group_scan_func) (struct point[], int);

int scan_singles(void);
int scan_singles_rowcol(void);
int scan_boxes(void);

int scan_hidden_pairs_group(struct point cells[], int n_cells);
int scan_naked_pairs_group(struct point cells[], int n_cells);
int scan_box_line_group(struct point cells[], int n_cells);
int scan_rows_cols(group_scan_func scan, const char *name);
int scan_all_groups(group_scan_func scan, const char *name);

#endif
