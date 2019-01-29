/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005-2007 Jari Tenhunen <jait@iki.fi>
 *
 * TODO:
 * - naked triples/quads, hidden triples/quads
 * - X-Wing?
 *
 */

#include <stdlib.h>

#include "jass.h"
#include "scan.h"
#include "poss.h"
#include "log.h"

extern num_t board[Y][X];

static void find_possible_cells(struct point cells[], int n_cells, struct ptr_array poss_cells[]);
static int get_box(coord_t y, coord_t x);
static int eliminate_from_box_excluding(num_t nr, int box, struct ptr_array *skip);

static int
get_box(coord_t y, coord_t x)
{
    return ((y / BOX_Y) * (X / BOX_X) + (x / BOX_X));
}

int 
scan_singles(void)
{
    char found = 0;
    num_t i, j, k, val;

    for (i = 0; i < Y; i++) 
    {
        for (j = 0; j < X; j++) 
        {
            val = 0;
            for (k = 0; k < NR_MAX; k++) 
            {
                if (is_poss(i,j,k)) 
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

            if (val != 0) 
            {
                explain("Single possibility (%d) for cell (%d, %d)", val, j+1, i+1);
                fix(i, j, val);
                ++found;
            }
        }
    }
    return found;
}

int 
scan_singles_rowcol(void)
{
    num_t i, j, k;
    int place[NR_MAX];	/* holds the column or row of the only possible place for numbers */
                        /* 0  => no possibilities (yet)  */
                        /* -1 => two or more possibilities */
    char found = 0;

    /* loop rows */
    for (i = 0; i < Y; i++) 
    {
        for (k = 0; k < NR_MAX; k++)
            place[k] = 0;

        for (j = 0; j < X; j++) 
        {
            /* don't check possibilities if the cell is occupied */
            if (board[i][j] != 0)
                continue;

            for (k = 0; k < NR_MAX; k++) 
            {
                if (is_poss(i,j,k)) 
                {
                    if (place[k] == 0)  /* no possibility yet */
                        place[k] = j + 1; /* because zero has a special meaning */
                    else 
                        place[k] = -1; /* two or more possible places for k */
                }
            }
                
        }
        /* check after each row */
        for (k = 0; k < NR_MAX; k++) 
        {
            if (place[k] > 0 ) 
            {
                explain("Single possible place (col %d) for %d on row %d", place[k]-1+1, k+1, i+1);
                /* 1) because place-array has special meaning for zero
                 * 2) because k is zero-offset */
                fix(i, place[k]-1, k + 1); 
                ++found;
            }
        }
    }

    /* loop cols */
    for (j = 0; j < X; j++) 
    {
        for (k = 0; k < NR_MAX; k++)
            place[k] = 0;

        for (i = 0; i < Y; i++) 
        {
            /* don't check possibilities if the cell is occupied */
            if (board[i][j] != 0)
                continue;

            for (k = 0; k < NR_MAX; k++) 
            {
                if (is_poss(i,j,k)) 
                {
                    if (place[k] == 0)  /* no possibility yet */
                        place[k] = i + 1; /* because zero has a special meaning */
                    else 
                        place[k] = -1; /* two or more possible places for k */
                }
            }
        }
        /* check for singles after each row */
        for (k = 0; k < NR_MAX; k++) 
        {
            if (place[k] > 0 ) 
            {
                explain("Single possible place (row %d) for %d on col %d\n", place[k]-1+1, k+1, j+1);
                /* 1) because place-array has special meaning for zero
                 * 2) because k is zero-offset */
                fix(place[k]-1, j, k + 1); 
                ++found;
            }
        }
    }
    return found;
}

/* 
 * Finds singles in boxes and does box-line and box-col reduction
 *
 */
int 
scan_boxes(void)
{
    num_t i, j, k, bi, bj, tmpx, tmpy;
    int found = 0;
    int boxes_x;
    int boxes_y;
    struct point place[NR_MAX];	/* holds the place (y,x) of the only possible place for numbers */
                                /* 0,0  => no possibilities (yet)  */
                                /* -1   => two or more possibilities */

    /* loop over 3x3 boxes */
    /* loop over each number, checking if it has only one possible place */
    /* if there's more than one possibility, check if we can do box-line reduction */

    boxes_y = Y / BOX_Y;
    boxes_x = X / BOX_X;

    for (bi = 0; bi < boxes_y; bi++) 
    {
        for (bj = 0; bj < boxes_x; bj++) 
        {
            /* clear place array */
            for (k = 0; k < NR_MAX; k++) 
            {
                place[k].y = 0;
                place[k].x = 0;
            }
            /* debug("Scanning box (%d, %d)", bj+1, bi+1); */
            /* loop over the nine cells */
            for (i = 0; i < BOX_Y; i++) 
            {
                tmpy = bi*BOX_Y + i;
                for (j = 0; j < BOX_X; j++) 
                {
                    /* tricky */
                    tmpx = bj*BOX_X + j;
                    /* don't check possibilities if the cell is occupied */
                    if (board[tmpy][tmpx] != 0)
                        continue;

                    for (k = 0; k < NR_MAX; k++) 
                    {
                        if (is_poss(tmpy,tmpx,k)) 
                        {
                            /* debug("%d poss in (%d, %d)", k+1, j+1, i+1); */
                            if (place[k].x == 0) 
                            {  
                                /* no possibility yet */
                                place[k].y = 1 + tmpy; /* because zero has a special meaning */
                                place[k].x = 1 + tmpx;
                            }
                            else 
                            {
                                /* two or more possible places for k */

                                /* if the possibilities are not in the same col, 
                                 * mark as -1 */
                                if (place[k].x != 1 + tmpx)
                                    place[k].x = -1;

                                /* not same row => mark as - 1 */
                                if (place[k].y != 1 + tmpy)
                                    place[k].y = -1;
                            }
                        }
                    }
                }
            }
            /* check after each box */
            for (k = 0; k < NR_MAX; k++) 
            {
                if (place[k].x > 0 && place[k].y > 0) 
                {
                    explain("Single possible place (%d, %d) for %d in box (%d, %d)", place[k].x, place[k].y, k+1, bj+1, bi+1);
                    /* 1) because place-array has special meaning for zero
                     * 2) because k is zero-offset */
                    fix(place[k].y - 1, place[k].x - 1, k + 1); 
                    ++found;
                }
                else if (place[k].x > 0) 
                {
                    /* k possible only on this col */
                    /* eliminate k's other possibilities from other boxes on current col */
                    for (tmpy = 0; tmpy < boxes_y; tmpy++) 
                    {
                        if (tmpy == bi) /* don't delete possibilities from current box */
                            continue;

                        for (i = 0; i < BOX_Y; i++) 
                        {
                            if (is_poss(tmpy * BOX_Y + i,(place[k].x - 1),k)) 
                            {
                                /* explain("%d possible only on col %d in box (%d, %d)", k+1, place[k].x, bj+1, bi+1); */
                                debug("Eliminating %d from (%d, %d)", k+1, place[k].x, tmpy * BOX_Y + i + 1);
                                set_poss(tmpy * BOX_Y + i,(place[k].x - 1),k, P_FALSE);
                                ++found;
                            }
                        }
                    }

                }
                else if (place[k].y > 0) 
                {
                    /* k possible only on this row */
                    /* eliminate k's other possibilities from other boxes on current row */
                    for (tmpx = 0; tmpx < boxes_x; tmpx++) 
                    {
                        if (tmpx == bj) /* don't delete possibilities from current box */
                            continue;

                        for (j = 0; j < BOX_X; j++) 
                        {
                            if (is_poss(place[k].y - 1, tmpx * BOX_X + j, k)) 
                            {
                                /* explain("%d possible only on row %d in box (%d, %d)", k+1, place[k].y, bj+1, bi+1); */
                                debug("Eliminating %d from (%d, %d)\n", k+1, tmpx * BOX_X + j + 1, place[k].y);
                                set_poss(place[k].y-1, tmpx * BOX_X + j, k, P_FALSE);
                                ++found;
                            }
                        }
                    }
                }
            }
        }
    }
    return found;
}

int 
scan_naked_pairs_group(struct point cells[], int n_cells)
{
    num_t k;
    int found = 0;
    num_t subset[2];
    num_t subset_comp[2];
    struct point place;
    struct point place_comp;	/* holds the places (x,y) of the pair */
    int cellno, cellno2, cellno3;
    struct point *cell = NULL;
    struct point *cell2 = NULL;
    struct point *cell3 = NULL;

    if (NULL == cells || n_cells < 1)
        return 0;

    /* walk through all given cells */
    for (cellno = 0; cellno < n_cells - 1; ++cellno)
    {
        cell = & (cells[cellno]);

        if (board[cell->y][cell->x] != 0)
            continue;

        /* clear things */
        subset[0] = 0;
        subset[1] = 0;

        /* check # of candidates for this cell */
        for (k = 0; k < NR_MAX; ++k) 
        {
            if (is_poss(cell->y, cell->x,k)) 
            {
                if (subset[0] == 0) 
                {
                    subset[0] = k + 1;
                }
                else if (subset[1] == 0) 
                {
                    subset[1] = k + 1;
                    place.y = cell->y;
                    place.x = cell->x; 
                }
                else 
                {
                    subset[0] = 0;
                    subset[1] = 0;
                    break;
                }
            }
        }

        /* if the cell has only two candidates */
        if (subset[0] != 0 && subset[1] != 0) 
        {
            subset_comp[0] = 0;
            subset_comp[1] = 0;
            place_comp.y = 0;
            place_comp.x = 0;

            for (cellno2 = cellno + 1; cellno2 < n_cells; ++cellno2)
            {
                cell2 = & (cells[cellno2]);

                if (board[cell2->y][cell2->x] != 0)
                    continue;

                /* clear things */
                subset_comp[0] = 0;
                subset_comp[1] = 0;
                place_comp.x = cell2->x;

                if (poss_equals(place.y, place.x, cell2->y, cell2->x)) 
                {
                    subset_comp[0] = subset[0];
                    subset_comp[1] = subset[1];
                    place_comp.x = cell2->x;
                    place_comp.y = cell2->y;
                    break;
                    /* TODO: this ignores naked triples */
                }
            }

            if (subset_comp[0] != 0 && subset_comp[1] != 0) 
            {
                /* eliminate candidates from other cells in the group */
                for (cellno3 = 0; cellno3 < n_cells; ++cellno3)
                {
                    cell3 = & (cells[cellno3]);
                    if (cell3 == cell || cell3 == cell2)
                        continue;

                    if (set_poss(cell3->y, cell3->x, subset[0]-1, P_FALSE)) 
                    {

                        explain("Naked pair {%d, %d} found in cells (%d, %d) and (%d, %d)",subset[0], subset[1], place.x+1, place.y+1, place_comp.x+1,place_comp.y+1);
                        debug("Eliminating %d from (%d, %d)\n", subset[0], cell3->x+1, cell3->y+1);
                        ++found;
                    }
                    if (set_poss(cell3->y, cell3->x, subset[1]-1, P_FALSE)) 
                    {
                        explain("Naked pair {%d, %d} found in cells (%d, %d) and (%d, %d)", subset[0], subset[1], place.x+1, place.y+1, place_comp.x+1,place_comp.y+1);
                        debug("Eliminating %d from (%d, %d)", subset[1], cell3->x+1, cell3->y+1);
                        ++found;
                    }
                }
            }
        }
    }

    return found;
}

static void 
find_possible_cells(struct point cells[], int n_cells, struct ptr_array poss_cells[])
{
    int cellno, nr;
    struct point **tmp = NULL;
    struct point *cell = NULL;

    /* walk through all given cells to find possible cells for all numbers */
    for (cellno = 0; cellno < n_cells; ++cellno)
    {
        cell = & (cells[cellno]);
      
        /* don't check possibilities if the cell is occupied */
        if (board[cell->y][cell->x] != 0)
            continue;

        for (nr = 0; nr < NR_MAX; ++nr)
        {
            if (is_poss(cell->y, cell->x, nr))
            {
                /* uh, hope i got this right this time ! */
                /* TODO: this whole pointer arithmetic is so error-prone that
                 * some other data structure would be better */
                tmp = (struct point **) (poss_cells[nr].ptr + (poss_cells[nr].n));
                *tmp = cell;
                (poss_cells[nr].n)++;
            }
        }
    }
}

int 
scan_rows_cols(group_scan_func scan, const char *name)
{
    int found = 0;
    num_t i, j;
    struct point cells[NR_MAX];

    /* rows */
    for (j = 0; j < Y; ++j)
    {
        for (i = 0; i < X; ++i)
        {
            cells[i].y = j;
            cells[i].x = i;
        }
        debug("Performing scan `%s' on row %d", name, j + 1);
        if (scan(cells, NR_MAX))
            ++found;
    }

    /* cols */
    for (i = 0; i < X; ++i)
    {
        for (j = 0; j < Y; ++j)
        {
            cells[j].y = j;
            cells[j].x = i;
        }
        debug("Performing scan `%s' on col %d", name, i + 1);
        if (scan(cells, NR_MAX))
            ++found;
    }

    return found;
}

int 
scan_all_groups(group_scan_func scan, const char *name)
{
    int found = 0, boxes_x, boxes_y, tmpy, tmpx;
    num_t i, j, bj, bi, n;
    struct point cells[NR_MAX];

    found += scan_rows_cols(scan, name);

    /* and boxes */
    boxes_y = Y / BOX_Y;
    boxes_x = X / BOX_X;

    for (bj = 0; bj < boxes_y; bj++) 
    {
        for (bi = 0; bi < boxes_x; bi++) 
        {
            /* debug("Scanning box (%d, %d)", bi+1, bj+1); */
            /* loop over the nine cells */
            n = 0;
            for (j = 0; j < BOX_Y; j++) 
            {
                tmpy = bj*BOX_Y + j;
                for (i = 0; i < BOX_X; i++) 
                {
                    tmpx = bi*BOX_X + i;
                    cells[n].y = tmpy;
                    cells[n].x = tmpx;
                    n++;
                }
            }
            if (scan(cells, NR_MAX))
                ++found;
        }
    }

    return found;
}

int 
scan_hidden_pairs_group(struct point cells[], int n_cells)
{
    int found = 0;
    struct point *cell = cells;
    void * voidptr = NULL;
    num_t nr = 0, first = 0, second = 0, nr2, i;
    struct ptr_array poss_cells[NR_MAX];

    if (NULL == cells || n_cells < 1)
        return 0;

    /* initialize the poss_cells array */
    for (nr = 0; nr < NR_MAX; ++nr)
    {
        /* poss_cells[nr].ptr = calloc(n_cells, sizeof(struct point *)); */
        voidptr = calloc(n_cells, sizeof(struct point *));
        /* debug("poss_cells[%d].ptr %p", nr, voidptr); */
        poss_cells[nr].ptr = voidptr;
        if (NULL == poss_cells[nr].ptr)
            eprintf("calloc() failed:");

        poss_cells[nr].n = 0; /* n means in this context the number of possible cells */
    }

    /* must find exactly two cells that contain the same two candidates.
     * The candidates must not appear anywhere else in the group.
     */

    /*
     * - create a mapping for all numbers:
     *   number => possible cells
     */
    find_possible_cells(cells, n_cells, poss_cells);

    /*
     * - if number has exactly two possible cells, store as candidate and go on
     * - if another number has exactly two possible cells and the cells are the same
     *   => hidden pair. eliminate all other candidates (if there are any) in these two cells
     *
     */

    for (nr = 0; nr < NR_MAX - 1; ++nr)
    {
        if (poss_cells[nr].n == 2)
        {
            first = nr + 1;
            /* search if there's another number having the same possible cells */
            for (nr2 = nr + 1; nr2 < NR_MAX; ++nr2)
            {
                if (poss_cells[nr2].n == 2)
                {
                    int same = 1;
                    int slot;

                    second = nr2 + 1;
                    /* the cells are traversed in the same order */
                    for (slot = 0; slot < 2; slot++)
                    {
                        /* compare if the possible cells point to the same place */
                        if ((struct point *) *(poss_cells[nr].ptr + slot) !=
                                (struct point *) *(poss_cells[nr2].ptr + slot))
                        {
                            same = 0;
                            break;
                        }
                    }
                    if (same)
                    {
                        debug("Hidden pair {%d, %d} found", first, second);
                        for (slot = 0; slot < 2; slot++)
                        {
                            cell = (struct point *) *(poss_cells[nr].ptr + slot);
                            /* eliminate all other possibilities except the pair */
                            for (i = 0; i < NR_MAX; ++i)
                            {
                                if (i + 1 == first || i + 1 == second)
                                {
                                    continue;
                                }

                                if (set_poss(cell->y, cell->x, i, P_FALSE))
                                {
                                    explain("Eliminating %d from (%d, %d)", i+1, cell->x+1, cell->y+1);
                                    ++found;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* free the allocated stuff */
    for (nr = 0; nr < NR_MAX; ++nr)
    {
        /*debug("freeing mem at %p", poss_cells[nr].ptr); */
        free(poss_cells[nr].ptr);
    }

    return found;
}

int
scan_box_line_group(struct point cells[], int n_cells)
{
    int found = 0;
    struct point *cell = cells;
    void * voidptr = NULL;
    num_t nr = 0;
    struct ptr_array poss_cells[NR_MAX];
    int box = -1, slot, i;

    if (NULL == cells || n_cells < 1)
        return 0;

    /* initialize the poss_cells array */
    for (nr = 0; nr < NR_MAX; ++nr)
    {
        /* poss_cells[nr].ptr = calloc(n_cells, sizeof(struct point *)); */
        voidptr = calloc(n_cells, sizeof(struct point *));
        /* debug("poss_cells[%d].ptr %p", nr, voidptr); */
        poss_cells[nr].ptr = voidptr;
        if (NULL == poss_cells[nr].ptr)
            eprintf("calloc() failed:");

        poss_cells[nr].n = 0; /* n means in this context the number of possible cells */
    }

    /*
     * - create a mapping for all numbers:
     *   number => possible cells
     */
    find_possible_cells(cells, n_cells, poss_cells);

    for (nr = 0; nr < NR_MAX; ++nr)
    {
        if (poss_cells[nr].n > BOX_X)
            continue;

        box = -1;

        for (slot = 0 ; slot < poss_cells[nr].n; ++slot)
        {
            cell = (struct point *) *(poss_cells[nr].ptr + slot);
            i = get_box(cell->y, cell->x);
            if (box == -1)
            {
                box = i;
            }
            else if (box != i)
            {
                /* nr is possible in two different boxes */
                box = -1;
                break;
            }
        }

        if (box >= 0)
        {
            if (eliminate_from_box_excluding(nr, box, &poss_cells[nr]))
            {
                explain("%d possible only in box %d in row or col", nr+1, box+1);
                ++found;
            }
        }

    }

    return found;
}

static int
eliminate_from_box_excluding(num_t nr, int box, struct ptr_array *skip)
{
    int found = 0;
    struct point *cell = NULL;
    int i, j, slot, i_beg, j_beg;
    char check = 1;

    j_beg = (box / BOX_Y) * BOX_Y;
    i_beg = (box % BOX_X) * BOX_X;

    /* debug("Box %d, i_beg %d, j_beg %d", box + 1, i_beg+1, j_beg+1);  */
    for (j = j_beg; j < j_beg + BOX_Y; ++j)
    {
        for (i = i_beg; i < i_beg + BOX_X; ++i)
        {
            check = 1;
            for (slot = 0 ; slot < skip->n; ++slot)
            {
                    cell = (struct point *) *(skip->ptr + slot);
                    if (cell->y == j && cell->x == i)
                    {
                        /* debug("Skipping cell (%d, %d)", i+1,j+1); */
                        check = 0;
                        break;
                    }
            }
            if (check == 0)
                continue;

            if (set_poss(j, i, nr, P_FALSE)) 
            {
                debug("Eliminating %d from (%d, %d) in box %d\n", nr+1, i+1, j+1, box+1);
                ++found;
            }
        }
    }
    return found;
}
