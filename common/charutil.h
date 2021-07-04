#ifndef CHARUTIL_H
#define CHARUTIL_H

#include <stddef.h>
#include <stdio.h>

// returns -1 if invalid esc sequence
// does not cast to char; handle over-length esc seqs at caller
// pointer to i - pointer to new current pos in string
long long int parse_char(char* str, size_t* i, int* type);

// print single char in escape-seq format
void emit_char(unsigned char c, FILE* f);

#endif
