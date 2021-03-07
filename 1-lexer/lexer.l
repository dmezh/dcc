/* tidy lexer */

%option noyywrap
%x FILENAME
%x ENDL
%{
#include "lexer.h"
#include "charutil.h"
YYSTYPE yylval;
struct context context = {NULL, 1};
%}

HEXSET  [0-9A-Fa-f]
HEXEXP  ([Pp][+-]?[0-9]+)
FLTEXP  ([Ee][+-]?[0-9]+)
/* terrorism, but still readable: each of the | conditions are the different valid formats for a real constant: */
/* hex+.hex*(hexexponent) | hex*.hex+(hexexponent) | hex+(hexexponent) | dec+.dec*(fltexponent)? | dec*.dec+(fltexponent)? | dec(fltexponent) */
FLOAT   0x{HEXSET}+\.{HEXSET}*{HEXEXP}|0x{HEXSET}*\.{HEXSET}+{HEXEXP}|0x{HEXSET}+{HEXEXP}|[0-9]+\.[0-9]*{FLTEXP}?|[0-9]*\.[0-9]+{FLTEXP}?|[0-9]+{FLTEXP}
OCT     (0)[0-7]*
INT     ((0x){HEXSET}+|[1-9][0-9]*)
STRING  \"(?:[^"\\]|\\.)*\"
CHAR    \'(?:[^'\\]|\\.)*\'
PUNCT   [\[\]\(\)\{\}\.\&\*\+\-\~\!\/\%\<\>\^\|\?\:\;\=\,\#]

%%

[ \t]+          /* ignore whitespace */

#[ ][0-9]+[ ]           {
                            context.lineno = atoi(yytext+2);
                            BEGIN(FILENAME);
                        }
<FILENAME>{STRING}      {
                            if (context.filename) free(context.filename);
                            context.filename = strdup(yytext+1); // skip quote
                            context.filename[strlen(yytext)-2] = '\0'; // delete quote
                            BEGIN(ENDL);
                        }
<ENDL>.*\n              {
                            BEGIN(INITIAL);
                        }

"auto"          { return AUTO;      }
"break"         { return BREAK;     }
"case"          { return CASE;      }
"char"          { return CHAR;      }
"const"         { return CONST;     }
"continue"      { return CONTINUE;  }
"default"       { return DEFAULT;   }
"do"            { return DO;        }
"double"        { return DOUBLE;    }
"else"          { return ELSE;      }
"enum"          { return ENUM;      }
"extern"        { return EXTERN;    }
"float"         { return FLOAT;     }
"for"           { return FOR;       }
"goto"          { return GOTO;      }
"if"            { return IF;        }
"inline"        { return INLINE;    }
"int"           { return INT;       }
"long"          { return LONG;      }
"register"      { return REGISTER;  }
"restrict"      { return RESTRICT;  }
"return"        { return RETURN;    }
"short"         { return SHORT;     }
"signed"        { return SIGNED;    }
"sizeof"        { return SIZEOF;    }
"static"        { return STATIC;    }
"struct"        { return STRUCT;    }
"switch"        { return SWITCH;    }
"typedef"       { return TYPEDEF;   }
"union"         { return UNION;     }
"unsigned"      { return UNSIGNED;  }
"void"          { return VOID;      }
"volatile"      { return VOLATILE;  }
"while"         { return WHILE;     }
"_Bool"         { return _BOOL;     }
"_Complex"      { return _COMPLEX;  }
"_Imaginary"    { return _IMAGINARY;}

    /* in the order they appear in ISO 6.4.6 */
->              { return INDSEL;    }
\+\+            { return PLUSPLUS;  }
--              { return MINUSMINUS;}
\<\<            { return SHL;       }
>>              { return SHR;       }
\<=             { return LTEQ;      }
>=              { return GTEQ;      }
==              { return EQEQ;      }
!=              { return NOTEQ;     }
&&              { return LOGAND;    }
\|\|            { return LOGOR;     }
\.\.\.          { return ELLIPSIS;  }
\*=             { return TIMESEQ;   }
\/=             { return DIVEQ;     }
%=              { return MODEQ;     }
\+=             { return PLUSEQ;    }
-=              { return MINUSEQ;   }
\<\<=           { return SHLEQ;     }
>>=             { return SHREQ;     }
&=              { return ANDEQ;     }
\^=             { return XOREQ;     }
\|=             { return OREQ;      }

    /* digraphs */
\<:             { return '[';       }
\:>             { return ']';       }
\<%             { return '{';       }
\%>             { return '}';       }

{FLOAT}                         {   return process_real(s_DOUBLE);          }
{FLOAT}[Ff]                     {   return process_real(s_FLOAT);           }
{FLOAT}[Ll]                     {   return process_real(s_LONGDOUBLE);      }

{OCT}                           {   return process_oct (1, s_INT);          }
{OCT}[Ll]                       {   return process_oct (1, s_LONG);         }
{OCT}(LL|ll)                    {   return process_oct (1, s_LONGLONG);     }
{OCT}[Uu]                       {   return process_oct (0, s_INT);          }
{OCT}([Uu][Ll]|[Ll][Uu])        {   return process_oct (0, s_LONG);         }
{OCT}([Uu](LL|ll)|(LL|ll)[Uu])  {   return process_oct (0, s_LONGLONG);     }

{INT}                           {   return process_uint(1, s_INT);          }
{INT}[Ll]                       {   return process_uint(1, s_LONG);         }
{INT}(LL|ll)                    {   return process_uint(1, s_LONGLONG);     }
{INT}[Uu]                       {   return process_uint(0, s_INT);          }
{INT}([Uu][Ll]|[Ll][Uu])        {   return process_uint(0, s_LONG);         }
{INT}([Uu](LL|ll)|(LL|ll)[Uu])  {   return process_uint(0, s_LONGLONG);     }

{STRING}                        {
                                    /* thanks to https://stackoverflow.com/questions/249791/regex-for-quoted-string-with-escaping-quotes */
                                    yylval.textlit.str = (char*)calloc(strlen(yytext+1), sizeof(char)); /* can't be longer than this */
                                    yylval.textlit.len = 0;
                                    for (int i = 1; i<strlen(yytext)-1; ) /* skipping the first and last (") */
                                        yylval.textlit.str[yylval.textlit.len++] = (unsigned char)parse_char_safe(yytext, &i);
                                    return STRING;
                                }
{CHAR}                          {
                                    int chars_read = 0;
                                    yylval.charlit = (unsigned char)parse_char_safe(yytext+1, &chars_read);
                                    if (strlen(yytext) - 2 != chars_read) { // minus the single quotes
                                        fprintf(stderr, "warning: character constant too long for its type\n");
                                    }
                                    return CHARLIT;
                                }

{PUNCT}                         { return yytext[0]; }
[A-Za-z_][A-Za-z0-9_]*          {
                                    yylval.ident = strdup(yytext);
                                    return IDENT;
                                }
\n                              { context.lineno++; }
.                               { print_context(0); fprintf(stderr,"unknown token %s\n",yytext);   }
%%

int process_uint(int is_signed, int type) {
    if (strlen(yytext) > 2 && yytext[0] == '0' && yytext[1] == 'x') {
        yylval.number.integer=(unsigned long long)strtoull(yytext, NULL, 16);
    } else {
        yylval.number.integer=(unsigned long long)strtoull(yytext, NULL, 10);
    }
    yylval.number.aux_type = type;
    yylval.number.is_signed = is_signed;
    return NUMBER;
}

int process_oct(int is_signed, int type) {
    yylval.number.integer=(unsigned long long)strtoull(yytext, NULL, 8);
    yylval.number.aux_type = type;
    yylval.number.is_signed = is_signed;
    return NUMBER;
}

int process_real(int type) {
    yylval.number.real = (long double)strtold(yytext, NULL);
    yylval.number.aux_type = type;
    return NUMBER;
}

void print_context(int warn) {
    fprintf(stderr, "%s - in %s at line %d: ", (warn ? "warning" : "Error"), FRIENDLYFN, context.lineno);
}

unsigned char parse_char_safe(char* str, int* i) {
    long long int c = parse_char(str, i);
    if (c < 0) {
        print_context(0);
        fprintf(stderr, "unrecognized escape sequence\n");
        exit(5);
    } else if (c > 0xFF) {
        print_context(1);
        fprintf(stderr, "numeric escape sequence out of range\n");
    }
    if (c > 0xFF) c = 0xFF;
    return (unsigned char)c;
}
