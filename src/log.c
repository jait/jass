/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005, 2006 Jari Tenhunen <jait@iki.fi>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "jass.h"

static int loglevel = LOG_NORMAL;

void set_loglevel(int level)
{
    loglevel = level;
}

int get_loglevel(void)
{
    return loglevel;
}

void eprintf(const char *s, ...)
{
    va_list args;
    fflush(stdout);

    va_start(args, s);
    vfprintf(stderr, s, args);
    va_end(args);

    if (s[0] != '\0' && s[strlen(s)-1] == ':')
        fprintf(stderr, " %s", strerror(errno));

    fprintf(stderr, "\n");
    
    exit(EXIT_FAILURE);
}

void wprintf(const char *s, ...)
{
    va_list args;
    fflush(stdout);

    va_start(args, s);
    vfprintf(stderr, s, args);
    va_end(args);

    if (s[0] != '\0' && s[strlen(s)-1] == ':')
        fprintf(stderr, " %s", strerror(errno));

    fprintf(stderr, "\n");
}

void debug(const char *s, ...)
{
    va_list args;
    if (loglevel < LOG_DEBUG)
        return;

    fflush(stdout);

    va_start(args, s);
    vfprintf(stdout, s, args);
    va_end(args);
    fprintf(stdout, "\n");
}

void explain(const char *s, ...)
{
    va_list args;

    if (get_mode() != MODE_STEP && loglevel < LOG_DEBUG)
        return;

    fflush(stdout);

    va_start(args, s);
    vfprintf(stdout, s, args);
    va_end(args);
    fprintf(stdout, "\n");
}

void info(const char *s, ...)
{
    va_list args;
    fflush(stdout);

    va_start(args, s);
    vfprintf(stdout, s, args);
    va_end(args);
    fprintf(stdout, "\n");
}
