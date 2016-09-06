/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Utilities header
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef UTILS_H
#define UTILS_H


/* Function prototypes */
bool is_empty_to_eof(FILE *stream);
char *read_to_eoln(FILE *stream, int *err);
bool str_start_match(char *base, char *cmp);
bool str_match(char *base, char *cmp);
char *int_to_str(int num);

#endif
