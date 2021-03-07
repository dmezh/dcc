#ifndef CHARUTIL_H
#define CHARUTIL_H

#include <stdlib.h>
#include <stdio.h>

// returns -1 if invalid esc sequence
// does not cast to char; handle over-length esc seqs at caller
// pointer to i == pointer to new current pos in string
int parse_char(char* str, int* i) {
    int newchar = 0;
    if (str[*i] == '\\') {
        switch (str[++(*i)])
        {
            case '\'': (*i)++; return '\''; break;
            case '\"': (*i)++; return '\"'; break;
            case '\?': (*i)++; return '\?'; break;
            case '\\': (*i)++; return '\\'; break;
            case 'a':  (*i)++; return '\a'; break;
            case 'b':  (*i)++; return '\b'; break;
            case 'f':  (*i)++; return '\f'; break;
            case 'n':  (*i)++; return '\n'; break;
            case 'r':  (*i)++; return '\r'; break;
            case 't':  (*i)++; return '\t'; break;
            case 'v':  (*i)++; return '\v'; break;
            case 'x':
                ; // empty
                char* newstr = NULL;
                newchar = strtol(str+(++(*i)), &newstr, 16);
                if (newstr == str+(*i)) return -1;
                *i = newstr - str;
                return newchar;
            default: // we'll give octal a try
                ; // empty
                int chars_read = 0;
                sscanf(str+(*i), "%3o%n", (unsigned int*)&newchar, &chars_read);
                if (!chars_read) return -1;
                *i = *i + chars_read;
                return newchar;
        }
    } else {
        return str[(*i)++];
    }
}

#endif