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
    char* ident;
    astn* astn_p;
}

%token INDSEL PLUSPLUS MINUSMINUS SHL SHR LTEQ GTEQ EQEQ
%token NOTEQ LOGAND LOGOR ELLIPSIS TIMESEQ DIVEQ MODEQ PLUSEQ MINUSEQ SHLEQ SHREQ ANDEQ
%token OREQ XOREQ AUTO BREAK CASE CHAR CONST CONTINUE DEFAULT DO DOUBLE ELSE ENUM EXTERN
%token FLOAT FOR GOTO IF INLINE INT LONG REGISTER RESTRICT RETURN SHORT SIGNED SIZEOF
%token STATIC STRUCT SWITCH TYPEDEF UNION UNSIGNED VOID VOLATILE WHILE _BOOL _COMPLEX _IMAGINARY

%type<astn_p> statement expr

%token<number> NUMBER
%token<strlit> STRING
%token<ident> IDENT
%type<astn_p> primary_expr constant stringlit ident
%type<astn_p> postfix_expr array_subscript fncall arg_list select indsel postop
%type<astn_p> unary_expr unops sizeof
%type<astn_p> cast_expr
%type<astn_p> mult_expr addit_expr shift_expr
%type<astn_p> relat_expr eqlty_expr
%type<astn_p> bwand_expr bwxor_expr bwor_expr logand_expr logor_expr
%type<astn_p> tern_expr
%type<astn_p> assign

%%

statement:
    expr ';'                    {   print_ast($1); printf("\n");    }
|   statement expr ';'          {   print_ast($2); printf("\n");    }
;

// ----------------------------------------------------------------------------
// 6.5.1 Primary expressions
primary_expr:
    ident
|   constant
|   stringlit
|   '(' expr ')'                {   $$=$2;   }
// generic selections yeah ok
;

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
// ----------------------------------------------------------------------------
// 6.5.2 Postfix operators
postfix_expr:
    primary_expr
|   array_subscript
|   fncall
|   select
|   indsel
|   postop
// todo: typename+init list
;

array_subscript:
    postfix_expr '[' expr ']'   {   $$=unop_alloc('*', astn_alloc(ASTN_BINOP));
                                    $$->astn_unop.target->astn_binop.op='+';
                                    $$->astn_unop.target->astn_binop.left=$1;
                                    $$->astn_unop.target->astn_binop.right=$3;
                                }
;

fncall:
    postfix_expr '(' ')'        {   $$=astn_alloc(ASTN_FNCALL);
                                    $$->astn_fncall.fn=$1;
                                    $$->astn_fncall.argcount=0;
                                    $$->astn_fncall.args=NULL;
                                }
|   postfix_expr '(' arg_list ')'   {   $$=astn_alloc(ASTN_FNCALL);
                                    $$->astn_fncall.fn=$1;
                                    $$->astn_fncall.args=$3;
                                    $$->astn_fncall.argcount=list_measure($$->astn_fncall.args);
                                }
;

arg_list:
    assign                      {   $$=list_alloc($1);          }
|   arg_list ',' assign         {   $$=$1; list_append($3, $1); }

select:
    postfix_expr '.' ident      {
                                    $$=astn_alloc(ASTN_SELECT);
                                    $$->astn_select.parent = $1;
                                    $$->astn_select.member = $3;
                                }
;

indsel:
    postfix_expr INDSEL ident   {   $$=astn_alloc(ASTN_SELECT);
                                    $$->astn_select.parent=unop_alloc('*', $1);
                                    $$->astn_select.member=$3;
                                }
;

postop:
    postfix_expr MINUSMINUS     {   $$=unop_alloc(MINUSMINUS, $1);  }
|   postfix_expr PLUSPLUS       {   $$=unop_alloc(PLUSPLUS, $1);    }
;
// ----------------------------------------------------------------------------
// 6.5.3 Unary operators
unary_expr:
    postfix_expr
|   unops
|   PLUSPLUS unary_expr         {   astn *n=astn_alloc(ASTN_NUM);
                                    n->astn_num.number.integer=1;
                                    n->astn_num.number.is_signed=1;
                                    n->astn_num.number.aux_type=s_INT;
                                    $$=cassign_alloc('+', $2, n);
                                }
|   MINUSMINUS unary_expr       {   astn *n=astn_alloc(ASTN_NUM);
                                    n->astn_num.number.integer=1;
                                    n->astn_num.number.is_signed=1;
                                    n->astn_num.number.aux_type=s_INT;
                                    $$=cassign_alloc('-', $2, n);
                                }
// todo: casts
|   sizeof
// todo: _Alignof
;

unops:
    '&' cast_expr               {   $$=unop_alloc('&', $2); }
|   '*' cast_expr               {   $$=unop_alloc('*', $2); }
|   '+' cast_expr               {   $$=unop_alloc('+', $2); }
|   '-' cast_expr               {   $$=unop_alloc('-', $2); }
|   '!' cast_expr               {   $$=unop_alloc('!', $2); }
|   '~' cast_expr               {   $$=unop_alloc('~', $2); }
;

sizeof:
    SIZEOF unary_expr           {   $$=astn_alloc(ASTN_SIZEOF);
                                    $$->astn_sizeof.target=$2;
                                }
// todo: sizeof abstract types
;
// ----------------------------------------------------------------------------
// 6.5.4 Cast operators
cast_expr:
    unary_expr
// todo: casts
;
// ----------------------------------------------------------------------------
// 6.5.5-14 Binary (two-arg) operators
mult_expr:
    cast_expr
|   mult_expr '*' cast_expr     {   $$=binop_alloc('*', $1, $3);    }
|   mult_expr '/' cast_expr     {   $$=binop_alloc('/', $1, $3);    }
|   mult_expr '%' cast_expr     {   $$=binop_alloc('%', $1, $3);    }
;
addit_expr:
    mult_expr
|   addit_expr '+' mult_expr    {   $$=binop_alloc('+', $1, $3);    }
|   addit_expr '-' mult_expr    {   $$=binop_alloc('-', $1, $3);    }
;
shift_expr:
    addit_expr
|   shift_expr SHL addit_expr   {   $$=binop_alloc(SHL, $1, $3);    }
|   shift_expr SHR addit_expr   {   $$=binop_alloc(SHR, $1, $3);    }
;
relat_expr:
    shift_expr
|   relat_expr '<' shift_expr   {   $$=binop_alloc('<', $1, $3);    }
|   relat_expr '>' shift_expr   {   $$=binop_alloc('>', $1, $3);    }
|   relat_expr LTEQ shift_expr  {   $$=binop_alloc(LTEQ, $1, $3);   }
|   relat_expr GTEQ shift_expr  {   $$=binop_alloc(GTEQ, $1, $3);   }
;
eqlty_expr:
    relat_expr
|   eqlty_expr EQEQ relat_expr  {   $$=binop_alloc(EQEQ, $1, $3);   }
|   eqlty_expr NOTEQ relat_expr {   $$=binop_alloc(NOTEQ, $1, $3);  }
;
bwand_expr:
    eqlty_expr
|   bwand_expr '&' eqlty_expr   {   $$=binop_alloc('&', $1, $3);    }
;
bwxor_expr:
    bwand_expr
|   bwxor_expr '^' bwand_expr   {   $$=binop_alloc('^', $1, $3);    }
;
bwor_expr:
    bwxor_expr
|   bwor_expr '|' bwxor_expr    {   $$=binop_alloc('|', $1, $3);    }
;
logand_expr:
    bwor_expr
|   logand_expr LOGAND bwor_expr    {   $$=binop_alloc(LOGAND, $1, $3); }
;
logor_expr:
    logand_expr
|   logor_expr LOGOR logand_expr    {   $$=binop_alloc(LOGOR, $1, $3);  }
;
// ----------------------------------------------------------------------------
// 6.5.15 Conditional (ternary) operator
tern_expr:
    logor_expr
|   logor_expr '?' expr ':' tern_expr   {   $$=astn_alloc(ASTN_TERN);
                                            $$->astn_tern.cond=$1;
                                            $$->astn_tern.t_then=$3;
                                            $$->astn_tern.t_else=$5;
                                        }
;
// ----------------------------------------------------------------------------
// 6.5.16 Assignment
assign:
    tern_expr
|   unary_expr '=' assign       {   $$=astn_alloc(ASTN_ASSIGN);
                                    $$->astn_assign.left=$1;
                                    $$->astn_assign.right=$3;
                                }
|   unary_expr TIMESEQ assign   {   $$=cassign_alloc('*', $1, $3);  }
|   unary_expr DIVEQ assign     {   $$=cassign_alloc('/', $1, $3);  }
|   unary_expr MODEQ assign     {   $$=cassign_alloc('%', $1, $3);  }
|   unary_expr PLUSEQ assign    {   $$=cassign_alloc('+', $1, $3);  }
|   unary_expr MINUSEQ assign   {   $$=cassign_alloc('-', $1, $3);  }
|   unary_expr SHLEQ assign     {   $$=cassign_alloc(SHL, $1, $3);  }
|   unary_expr SHREQ assign     {   $$=cassign_alloc(SHR, $1, $3);  }
|   unary_expr ANDEQ assign     {   $$=cassign_alloc('&', $1, $3);  }
|   unary_expr XOREQ assign     {   $$=cassign_alloc('^', $1, $3);  }
|   unary_expr OREQ assign      {   $$=cassign_alloc('|', $1, $3);  }
;
// ----------------------------------------------------------------------------
// 6.5.17 Comma operator
expr:
    assign
|   expr ',' assign             {   $$=binop_alloc(',', $1, $3);    }

%%

int main() {
    yydebug = 0;
    yyparse();
}
