/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005-2007 Jari Tenhunen <jait@iki.fi>
 *
 * TODO:
 * - use a bit vector for storing possibles?
 *
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "jass.h"
#include "poss.h"
#include "log.h"
#include "scan.h"

num_t board[Y][X];
static char mode = MODE_NORMAL;

void init(void)
{
    num_t i, j, k;
    for (i = 0; i < Y; i++) 
    {
        for (j = 0; j < X; j++) 
        {
            for (k = 0; k < NR_MAX; ++k) 
            {
                set_poss(i, j, k, P_TRUE);
            }
            board[i][j] = 0;
        }
    }
}

void print_board(void)
{
    num_t i, j;

    for (i = 0; i < X; i++) 
    {
        if (i % BOX_Y == 0)
            printf("+-------+-------+-------+\n");

        for (j = 0; j < Y; j++) 
        {
            if (j % BOX_X == 0)
                printf("| ");

            if (board[i][j] != 0)
                printf("%d ", board[i][j]);
            else
                printf(". ");
                
        }
        printf("|\n");
    }
    printf("+-------+-------+-------+\n");

}

void fix(num_t y, num_t x, num_t val)
{
    num_t i, j, k;

    explain("Placing %d into (%d, %d)", val, x+1, y+1);
    if (board[y][x] != 0) 
    {
        eprintf("Error: cell (%d,%d) already contains value %d", x+1, y+1, board[y][x]);
    }

    board[y][x] = val;

    /* no other possibilities for this cell */
    for (k = 0; k < NR_MAX; k++) 
    {
        set_poss(y, x, k, P_FALSE);
    }

    /* eliminate all occurrences of val from this col */
    for (i = 0; i < Y; i++) 
    {
        /*
        explain("Eliminating %d from (%d, %d)", val, x+1, i+1);
        */
        set_poss(i, x, val - 1, P_FALSE);
    }
    /* and row */
    for (i = 0; i < X; i++) 
    {
        /*
        explain("Eliminating %d from (%d, %d)", val, i+1, y+1);
        */
        set_poss(y, i, val - 1, P_FALSE);
    }

    /* eliminate all occurrences of val from current box */
    y = (y / BOX_X) * BOX_X;
    x = (x / BOX_Y) * BOX_Y;

    for (i = y; i < y + BOX_X; i++) 
    {
        for (j = x; j < x + BOX_Y; j++) 
        {
            /*
            explain("Eliminating %d from (%d, %d)", val, j+1, i+1);
            */
            set_poss(i, j, val - 1, P_FALSE);
        }
    }

    if (mode == MODE_STEP)
    {
        getchar();
    }
}

void init_fix(void) 
{
    num_t i, j, val;
    for (i = 0; i < X; i++) 
    {
        for (j = 0; j < Y; j++) 
        {
            if (board[i][j] != 0) 
            {
                val = board[i][j];
                board[i][j] = 0; /* to avoid warning */
                fix(i, j, val); 
            }
        }
    }
}

/**
 * Returns number of unsolved cells 
 * 
 */
int check_unsolved(void)
{
    num_t i, j;
    int unsolved = 0;

    for (i = 0; i < X; i++) 
    {
        for (j = 0; j < Y; j++) 
        {
            if (board[i][j] == 0)
                ++unsolved;
        }
    }
    return unsolved;
}

/**
 *
 *
 */
int parse_board(const char *str)
{
    int i = 0;
    char c = 0;
    num_t *bptr = (num_t *) board;

    while ((c = str[i]) != '\0' && i < X*Y) 
    {
        if (c == '0' || c == '.')
            bptr[i] = 0;
        else if (isdigit(c)) /* or isxdigit() ? */
        {  
            fix((i / X), i % X, c - '0'); 
            /* bptr[i] = c - '0'; */
        }
        else 
            return -1;

        ++i;
    }
    return i;
}

void dump_board(FILE *out) 
{
    num_t i, j;

    for (i = 0; i < Y; i++) 
    {
        for (j = 0; j < X; j++) 
        {
            if (board[i][j] == 0)
                fprintf(out, ".");
            else 
                fprintf(out, "%d", board[i][j]);
        }
    }
    fprintf(out, "\n");
}

int main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind, optopt;
    char *fname = NULL;
    char step = 0;
    int c;
    init();

    /* check args
     * -f: read sudokus from file (- for stdin)
     */
    while ((c = getopt(argc, argv, "f:vsh")) != EOF) 
    {
        switch (c) 
        {
            case 'f':
                fname = (char *) malloc(strlen(optarg)+1);
                if (fname == NULL)
                    eprintf("malloc() failed:");

                strncpy(fname, optarg, strlen(optarg));
                fname[strlen(optarg)] = '\0';
                break;
            case 'v':
                set_loglevel(get_loglevel() + 1);
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 's':
                step = 1;
                break;
            default:
                break;
        }
    }

    /*
    if (step && verbose == 0)
        ++verbose;
    */

    if (fname != NULL) 
    {
        FILE *file;
        char *line;
        int bufsize = X*Y + 2;
        if (strcmp(fname, "-") == 0)
            file = stdin;
        else 
        {
            file = fopen(fname, "r");
            if (file == NULL)
                eprintf("Couldn't open file '%s':", fname);

        }
        line = (char *) malloc(bufsize);
        if (NULL == line)
            eprintf("malloc() failed:");

        while (fgets(line, bufsize, file) != NULL) 
        {
            if (line[0] == '#')
                continue;

            init();
            parse_board(line);
            mode = MODE_NORMAL;
            if (step)
                mode = MODE_STEP;

            printf("%s", line);
            solve();
        }
        if (fileno(file) != STDIN_FILENO)
            fclose(file);

        free(line);
        free(fname);
    }
    else 
    {
        /* try if there was a puzzle as argument */
        if (argc > optind) 
        {
            printf("Initializing the board ...\n");
            if (parse_board(argv[optind]) != X*Y) 
            {
                eprintf("Error parsing the puzzle");
            }
            print_board();
            mode = MODE_NORMAL;
            if (step)
                mode = MODE_STEP;

            solve();
        }
        else 
        {
            wprintf("Error: no puzzle(s) given");
            usage();
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}

void usage(void)
{
    printf("Usage: jass [options] [<puzzle>]\n");
    printf("  -f <file>        instead looking for the puzzle string in the arguments,\n");
    printf("                   read puzzles from file (\"-\" for stdin), one per line\n");
    printf("  -s               step mode, pause after each solved number\n");
    printf("  -v               verbose debug output\n");
    printf("  -h               print usage and exit\n");
}

/**
 * Tries to solve the puzzle
 *
 * Returns true if the puzzle was fully solved
 */
int solve(void) 
{
    int nr = 1;
    /* loop as long as there is some progress */
    while (nr) 
    {
        nr = 0;

        debug("Scanning for singles...");
        if ((nr = scan_singles())) 
        {
            /* print_board(); */
            continue;
        }
        debug("Scanning boxes for singles and pointing pairs/triples...");
        if ((nr = scan_boxes())) 
        {
            /* print_board(); */
            continue;
        }
        debug("Scanning for singles on rows and cols...");
        if ((nr = scan_singles_rowcol())) 
        {
            /* print_board(); */
            continue;
        }
        debug("Scanning for naked pairs...");
        if ((nr = scan_all_groups(&scan_naked_pairs_group, "naked pairs"))) 
        {
            /* print_board(); */
            continue;
        }
        debug("Scanning for hidden pairs...");
        if ((nr = scan_all_groups(&scan_hidden_pairs_group, "hidden pairs"))) 
        {
            /*print_board(); */
            continue;
        }
        debug("Doing box/line reduction...");
        if ((nr = scan_rows_cols(&scan_box_line_group, "box/line"))) 
        {
            /*print_board(); */
            continue;
        }
    }

    print_board();
    if ((nr = check_unsolved()) == 0)
        info("Sudoku solved!");
    else
        info("Sudoku not solved, %d numbers left =(", nr);

    dump_board(stdout);

    return (nr == 0);
}

int get_mode(void)
{
    return mode;
}
