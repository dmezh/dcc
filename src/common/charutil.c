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

void emit_char(unsigned char c, FILE *f) {
    fprintf(f, "%s", get_char_esc(c));
}

char* get_char_esc(unsigned char c) {
    char *ret;
    switch (c)
    {
            case '\'': asprintf(&ret, "\\\'"); break;
            case '\"': asprintf(&ret, "\\\""); break;
            case '\?': asprintf(&ret, "?");    break;
            case '\\': asprintf(&ret, "\\\\"); break;
            case '\0': asprintf(&ret, "\\0");  break;
            case '\a': asprintf(&ret, "\\a");  break;
            case '\b': asprintf(&ret, "\\b");  break;
            case '\f': asprintf(&ret, "\\f");  break;
            case '\n': asprintf(&ret, "\\n");  break;
            case '\r': asprintf(&ret, "\\r");  break;
            case '\t': asprintf(&ret, "\\t");  break;
            case '\v': asprintf(&ret, "\\v");  break;
            default:
                if (c >= 33 && c <= 126) {
                    asprintf(&ret, "%c", c);
                } else {
                    asprintf(&ret, "\\%03o", c);
                }
    }

    return ret;
}

char* get_char_hexesc(unsigned char c) {
    char *ret;

    if (c != '"' && c >= 32 && c <= 126) {
        asprintf(&ret, "%c", c);
    } else {
        asprintf(&ret, "\\%02X", c);
    }

    return ret;
}
