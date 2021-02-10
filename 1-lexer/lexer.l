/* tidy lexer */

%option noyywrap
%{
#include "lexer.h"
YYSTYPE yylval;
%}

HEXSET  [0-9A-Fa-f]
FLOAT   ((0x){HEXSET}+(\.)({HEXSET}+)?)|([0-9]+(\.)([0-9]+)?)
INT     ((0x){HEXSET}+|[0-9]+)
STRING  \"(?:[^"\\]|\\.)*\"
CHAR    \'(?:[^'\\]|\\.)*\'

%%

[ \t]+          /* ignore whitespace */

"auto"			{ return AUTO;      }
"break"			{ return BREAK;     }
"case"			{ return CASE;      }
"char"			{ return CHAR;      }   
"const"			{ return CONST;     }
"continue"		{ return CONTINUE;  }
"default"		{ return DEFAULT;   }
"do"			{ return DO;        }
"double"		{ return DOUBLE;    }
"else"			{ return ELSE;      }
"enum"			{ return ENUM;      }
"extern"		{ return EXTERN;    }
"float"			{ return FLOAT;     }
"for"			{ return FOR;       }
"goto"			{ return GOTO;      }
"if"			{ return IF;        }
"inline"		{ return INLINE;    }
"int"			{ return INT;       }
"long"			{ return LONG;      }
"register"		{ return REGISTER;  }
"restrict"		{ return RESTRICT;  }
"return"		{ return RETURN;    }
"short"			{ return SHORT;     }
"signed"		{ return SIGNED;    }
"sizeof"		{ return SIZEOF;    }
"static"		{ return STATIC;    }
"struct"		{ return STRUCT;    }
"switch"		{ return SWITCH;    }
"typedef"		{ return TYPEDEF;   }
"union"			{ return UNION;     }
"unsigned"		{ return UNSIGNED;  }
"void"			{ return VOID;      }
"volatile"		{ return VOLATILE;  }
"while"			{ return WHILE;     }
"_Bool"			{ return _BOOL;     }
"_Complex"		{ return _COMPLEX;  }
"_Imaginary"	{ return _IMAGINARY;}

{FLOAT}                         {   
                                    fprintf(stderr, "gotcha\n");
                                    return NUMBER;
                                }

{INT}                           {   
                                    uint_parse();
                                    yylval.number.integer = (int)yylval.number.integer;
                                    yylval.aux_type = s_INT;
                                    yylval.is_signed = 1;
                                    return NUMBER;
                                }
{INT}[Ll]                       {
                                    uint_parse();
                                    yylval.number.integer = (long)yylval.number.integer;
                                    yylval.aux_type = s_LONG;
                                    yylval.is_signed = 1;
                                    return NUMBER;
                                }
{INT}(LL|ll)                    {
                                    uint_parse();
                                    yylval.number.integer = (long long)yylval.number.integer;
                                    yylval.aux_type = s_LONGLONG;
                                    yylval.is_signed = 1;
                                    return NUMBER;
                                }
{INT}[Uu]                       {
                                    uint_parse();
                                    yylval.number.integer = (unsigned int)yylval.number.integer;
                                    yylval.aux_type = s_INT;
                                    yylval.is_signed = 0;
                                    return NUMBER;
                                }
{INT}([Uu][Ll]|[Ll][Uu])        {   
                                    uint_parse();
                                    yylval.number.integer = (unsigned long)yylval.number.integer;
                                    yylval.aux_type = s_LONG;
                                    yylval.is_signed = 0;
                                    return NUMBER;
                                }
{INT}([Uu](LL|ll)|(LL|ll)[Uu])  {   
                                    uint_parse();
                                    yylval.number.integer = (unsigned long long)yylval.number.integer;
                                    yylval.aux_type = s_LONGLONG;
                                    yylval.is_signed = 0;
                                    return NUMBER;
                                }

{STRING}                        {
                                    /* thanks to https://stackoverflow.com/questions/249791/regex-for-quoted-string-with-escaping-quotes */
                                    yylval.str_lit = (char*)calloc(strlen(yytext+1), sizeof(char)); /* can't be longer than this */
                                    yylval.str_len = 0;
                                    for (int i = 1; i<strlen(yytext)-1; i++) { /* skipping the first and last (") */
                                        i = char_parse(i);
                                    }
                                    return STRING;
                                }
{CHAR}                          {
                                    yylval.str_lit = (char*)calloc(1, sizeof(char));
                                    yylval.str_len = 0;
                                    int chars_read = char_parse(1);
                                    if (strlen(yytext) - 2 != chars_read) { // minus the single quotes
                                        fprintf(stderr, "warning: character constant too long for its type\n");
                                    }
                                    return CHARLIT;
                                }

\n                              {yylval.lineno++;}
\+                              {return '+';}
-                               {return '-';}
.                               {fprintf(stderr,"Error: unknown token %s\n",yytext);}
%%

void uint_parse() {
    if (strlen(yytext) > 2 && yytext[0] == '0' && yytext[1] == 'x') {
        yylval.number.integer=(unsigned long long)strtoull(yytext, NULL, 16);
    } else if (yytext[0] == '0') {
        yylval.number.integer=(unsigned long long)strtoull(yytext, NULL, 8);
    } else {
        yylval.number.integer=(unsigned long long)strtoull(yytext, NULL, 10);
    }
}

/* parse one char from yytext and write it to yylval */
int char_parse(int i)
{
    unsigned long long newchar; // helps us detect+warn about extremely long out-of-range hex escapes
    if (yytext[i] == '\\') {
        switch (yytext[++i])
        {
            case '\'': yylval.str_lit[yylval.str_len++] = '\''; break;
            case '\"': yylval.str_lit[yylval.str_len++] = '\"'; break;
            case '\?': yylval.str_lit[yylval.str_len++] = '\?'; break;
            case '\\': yylval.str_lit[yylval.str_len++] = '\\'; break;
            case 'a':  yylval.str_lit[yylval.str_len++] = '\a'; break;
            case 'b':  yylval.str_lit[yylval.str_len++] = '\b'; break;
            case 'f':  yylval.str_lit[yylval.str_len++] = '\f'; break;
            case 'n':  yylval.str_lit[yylval.str_len++] = '\n'; break;
            case 'r':  yylval.str_lit[yylval.str_len++] = '\r'; break;
            case 't':  yylval.str_lit[yylval.str_len++] = '\t'; break;
            case 'v':  yylval.str_lit[yylval.str_len++] = '\v'; break;
            case 'x':
                ; // empty
                char* newyytext;
                newchar = strtol(yytext+(++i), &newyytext, 16);
                if (newchar > 0xFF) fprintf(stderr, "warning: hex escape sequence out of range\n");
                yylval.str_lit[yylval.str_len++] = (char)newchar;
                i = newyytext - yytext - 1;
                break;
            default: // we'll give octal a try
                ; // empty
                int chars_read = 0;
                chars_read = sscanf(yytext+i, "%3o[0-7]", &newchar);
                i = i + chars_read + 1;
                if (newchar > 0xFF) fprintf(stderr, "warning: octal escape sequence out of range\n");
                yylval.str_lit[yylval.str_len++] = (char)newchar;
                if (chars_read) break;

                fprintf(stderr, "Error: Unrecognized escape sequence\n");
                exit(5);       
        }   
    } else {
        yylval.str_lit[yylval.str_len++] = yytext[i];
    }
    return i;
}
