/* tidy lexer */

%option noyywrap
/* we don't use these, disable or else -Wpedantic is upset */
%option nounput
%option noinput
%x FILENAME
%x ENDL
%{
#include "lexer.h"

#include <stdbool.h>

#include "ast.h"
#include "parser.tab.h"
#include "location.h"
#include "charutil.h"
#include "semval.h"
#include "util.h"

static int process_uint(bool is_signed, enum int_types type);
static int process_oct(bool is_signed, enum int_types type);
static int process_real(enum int_types type);
static unsigned char parse_char_safe(char* str, size_t* i);

YYLTYPE context = {.lineno = 1, .filename = NULL};
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

    /* Internal actions */
"#pragma dbg-info"      { return SET_DEBUG_INFO;    }
"#pragma dbg-verbose"   { return SET_DEBUG_VERBOSE; }
"#pragma dbg-debug"     { return SET_DEBUG_DEBUG;   }
"#pragma dbg-none"      { return SET_DEBUG_NONE;    }
"#pragma do-perish"     { return _PERISH;   }
"#pragma do-examine"    { return _EXAMINE;  }
"#pragma do-dumpsymtab" { return _DUMPSYMTAB; }

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
                                    yylval.strlit.str = (char*)calloc(strlen(yytext+1), sizeof(char)); /* can't be longer than this */
                                    yylval.strlit.len = 0;
                                    for (size_t i = 1; i<strlen(yytext)-1; ) /* skipping the first and last (") */
                                        yylval.strlit.str[yylval.strlit.len++] = (unsigned char)parse_char_safe(yytext, &i);
                                    return STRING;
                                }
{CHAR}                          {
                                    size_t chars_read = 0;
                                    yylval.number.integer = (unsigned char)parse_char_safe(yytext+1, &chars_read);
                                    yylval.number.aux_type = s_CHARLIT;
                                    if (strlen(yytext) - 2 != chars_read) { // minus the single quotes
                                        print_context(1);
                                        fprintf(stderr, "character constant too long for its type\n");
                                    }
                                    return NUMBER;
                                }

{PUNCT}                         { return yytext[0]; }
[A-Za-z_][A-Za-z0-9_]*          {
                                    yylval.ident = strdup(yytext);
                                    return IDENT;
                                }
\n                              { context.lineno++; }
.                               { print_context(0); fprintf(stderr,"unknown token %s\n",yytext);   }
%%

static int process_uint(bool is_signed, enum int_types type) {
    if (strlen(yytext) > 2 && yytext[0] == '0' && yytext[1] == 'x') {
        yylval.number.integer = strtoull(yytext, NULL, 16);
    } else {
        yylval.number.integer = strtoull(yytext, NULL, 10);
    }
    yylval.number.aux_type = type;
    yylval.number.is_signed = is_signed;
    return NUMBER;
}

static int process_oct(bool is_signed, enum int_types type) {
    yylval.number.integer = strtoull(yytext, NULL, 8);
    yylval.number.aux_type = type;
    yylval.number.is_signed = is_signed;
    return NUMBER;
}

static int process_real(enum int_types type) {
    yylval.number.real = strtold(yytext, NULL);
    yylval.number.aux_type = type;
    return NUMBER;
}

void print_context(bool warn) {
    fprintf(stderr, "%s:%d %s: ", FRIENDLYFN, context.lineno, (warn ? "(warning)" : "Error"));
}

static unsigned char parse_char_safe(char* str, size_t* i) {
    int type;
    long long int c = parse_char(str, i, &type);
    if (c < 0) {
        print_context(0);
        fprintf(stderr, "unrecognized escape sequence\n");
        exit(5);
    } else if (c > 0xFF) {
        print_context(1);
        fprintf(stderr, "%s escape sequence out of range\n", type==1?"hex":"octal");
    }
    if (c > 0xFF) c = 0xFF;
    return (unsigned char)c;
}
