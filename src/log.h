/* vim: set sts=4 sw=4 et: */
/**
 * jass - just another sudoku solver
 * (C) 2005, 2006 Jari Tenhunen <jait@iki.fi>
 */

#ifndef LOG_H_
#define LOG_H_

enum
{
    LOG_NONE = 0,
    LOG_NORMAL = 1,
    LOG_DEBUG = 2
};

void set_loglevel(int level);
int get_loglevel(void);
void eprintf(const char *s, ...);
void wprintf(const char *s, ...);
void debug(const char *s, ...);
void explain(const char *s, ...);
void info(const char *s, ...);

#endif
