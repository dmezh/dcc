%option noyywrap
%{
#define NUMBER 1
#define PLUS 2
#define MINUS 3
int yylval;
%}

%%
[0-9]+      {yylval=atoi(yytext);return NUMBER;}
\+          {return PLUS;}
-           {return MINUS;}
.           {fprintf(stderr,"Error: unknown token %s\n",yytext);}
%%
