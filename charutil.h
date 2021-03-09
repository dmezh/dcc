#ifndef CHARUTIL_H
#define CHARUTIL_H

// returns -1 if invalid esc sequence
// does not cast to char; handle over-length esc seqs at caller
// pointer to i - pointer to new current pos in string
long long int parse_char(char* str, int* i, int* type);

// print single char in escape-seq format
void emit_char(unsigned char c);

#endif
