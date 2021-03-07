#ifndef CHARUTIL_H
#define CHARUTIL_H

#include <stdlib.h>
#include <stdio.h>

// returns -1 if invalid esc sequence
// does not cast to char; handle over-length esc seqs at caller
// pointer to i - pointer to new current pos in string
long long int parse_char(char* str, int* i) {
    long long int newchar = 0; // for obscenely long hex escapes
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
                if (newchar > 0xFF) newchar = 0xFF;
                return newchar;
            default: // we'll give octal a try
                ; // empty
                int chars_read = 0;
                sscanf(str+(*i), "%3o%n", (unsigned int*)&newchar, &chars_read);
                if (!chars_read) return -1;
                *i = *i + chars_read;
                if (newchar > 0xFF) newchar = 0xFF;
                return newchar;
        }
    } else {
        return str[(*i)++];
    }
}

void emit_char(unsigned char c) {
    switch (c)
    {
            case '\'': printf("\\\'"); break;
            case '\"': printf("\\\""); break;
            case '\?': printf("?");    break;
            case '\\': printf("\\\\"); break;
            case '\0': printf("\\0");  break;
            case '\a': printf("\\a");  break;
            case '\b': printf("\\b");  break;
            case '\f': printf("\\f");  break;
            case '\n': printf("\\n");  break;
            case '\r': printf("\\r");  break;
            case '\t': printf("\\t");  break;
            case '\v': printf("\\v");  break;
            default:
                if (c >= 33 && c <= 126) {
                    printf("%c", c);
                } else {
                    printf("\\%03o", c);
                }
    }
}

#endif
