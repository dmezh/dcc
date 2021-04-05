// basic character utilities
#include "charutil.h"

#include <stdio.h>
#include <stdlib.h>

long long int parse_char(char* str, size_t* i, int* type) {
    long long int newchar = 0; // for obscenely long hex escapes
    if (type) *type = 0; // 0 normal, 1 hex, 2 oct
    if (str[*i] == '\\') {
        switch (str[++(*i)])
        {
            case '\'': (*i)++; return '\'';
            case '\"': (*i)++; return '\"';
            case '\?': (*i)++; return '\?';
            case '\\': (*i)++; return '\\';
            case 'a':  (*i)++; return '\a';
            case 'b':  (*i)++; return '\b';
            case 'f':  (*i)++; return '\f';
            case 'n':  (*i)++; return '\n';
            case 'r':  (*i)++; return '\r';
            case 't':  (*i)++; return '\t';
            case 'v':  (*i)++; return '\v';
            case 'x':
                ; // empty
                char* newstr = NULL;
                newchar = strtol(str+(++(*i)), &newstr, 16);
                if (newstr == str+(*i)) return -1;
                *i = newstr - str;
                if (type) *type = 1;
                return newchar;
            default: // we'll give octal a try
                ; // empty
                int chars_read = 0;
                sscanf(str+(*i), "%3o%n", (unsigned int*)&newchar, &chars_read);
                if (!chars_read) return -1;
                *i = *i + chars_read;
                if (type) *type = 2;
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
