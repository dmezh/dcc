/* parser */

%{
    #include <stdio.h>
    #include "semval.h"
    #include "ast.h"
    int yylex(void);
    void yyerror (const char *s) { fprintf(stderr, "o! %s\n", s);}
%}

%define parse.trace

%union
{
    struct number number;
    struct strlit strlit;
    unsigned char charlit;
    char* ident;
    astn* astn_p;
}

%token INDSEL PLUSPLUS MINUSMINUS SHL SHR LTEQ GTEQ EQEQ
%token NOTEQ LOGAND LOGOR ELLIPSIS TIMESEQ DIVEQ MODEQ PLUSEQ MINUSEQ SHLEQ SHREQ ANDEQ
%token OREQ XOREQ AUTO BREAK CASE CHAR CONST CONTINUE DEFAULT DO DOUBLE ELSE ENUM EXTERN
%token FLOAT FOR GOTO IF INLINE INT LONG REGISTER RESTRICT RETURN SHORT SIGNED SIZEOF
%token STATIC STRUCT SWITCH TYPEDEF UNION UNSIGNED VOID VOLATILE WHILE _BOOL _COMPLEX _IMAGINARY

%token<number> NUMBER
%token<strlit> STRING
%token<charlit> CHARLIT
%token<ident> IDENT
%type<astn_p> constant
%type<astn_p> ident
%type<astn_p> stringlit
%type<astn_p> expr
%type<astn_p> array_subscript
%type<astn_p> fncall
%type<astn_p> select
%type<astn_p> indsel
%type<astn_p> primary_expr
%type<astn_p> postfix_expr

%left '.'
%%

expr:
    postfix_expr                {   print_ast($1);  }
;
// 6.5.1 Primary expressions

primary_expr:
    ident
|   constant
|   stringlit
|   '(' expr ')'                {   $$=$2;   }
// the fuck is a generic selection?
;

// 6.5.2 Postfix operators
postfix_expr:
    primary_expr
|   array_subscript
|   fncall
|   select
|   indsel
// DOING NOW: indsel
;

array_subscript:
    postfix_expr '[' expr ']'   {   $$=astn_alloc(ASTN_DEREF);
                                    $$->astn_deref.target=astn_alloc(ASTN_BINOP);
                                    $$->astn_deref.target->astn_binop.op='+';
                                    $$->astn_deref.target->astn_binop.left=$1;
                                    $$->astn_deref.target->astn_binop.right=$3;
                                }
;

fncall:
    postfix_expr '(' ')'        {   $$=astn_alloc(ASTN_FNCALL);
                                    $$->astn_fncall.fn=$1;
                                    $$->astn_fncall.args=NULL;
                                }
    // todo: with args

select:
    postfix_expr '.' ident      {
                                    $$=astn_alloc(ASTN_SELECT);
                                    $$->astn_select.parent = $1;
                                    $$->astn_select.member = $3;
                                }

indsel:
    postfix_expr INDSEL ident   {   $$=astn_alloc(ASTN_SELECT);
                                    $$->astn_select.parent=astn_alloc(ASTN_DEREF);
                                    $$->astn_select.parent->astn_deref.target=$1;
                                    $$->astn_select.member=$3;
                                }

ident:
    IDENT                       {   $$=astn_alloc(ASTN_IDENT);
                                    $$->astn_ident.ident=$1;
                                }
;

constant:
    NUMBER                      {   $$=astn_alloc(ASTN_NUM);
                                    $$->astn_num.number=$1;
                                }
;

stringlit:
    STRING                      {   $$=astn_alloc(ASTN_STRLIT);
                                    $$->astn_strlit.strlit=$1;
                                }
;   

%%

int main() {
    yydebug = 0;
    yyparse();
}