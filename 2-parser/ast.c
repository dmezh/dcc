#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parser.tab.h"
#include "charutil.h"

astn* astn_alloc(enum astn_type type) {
    astn *n = (astn*)malloc(sizeof(astn));
    n->type = type;
    return n;
}

void print_ast(astn *n) {
    static int tabs = 0;     //     -> __ <- two spaces
    for (int i=0; i<tabs; i++) printf("  ");
    if (!n) return; // if we just want to print tabs, pass NULL
    switch (n->type) {

/**/    case ASTN_NUM:
            if (n->astn_num.number.aux_type == s_CHARLIT) {
                printf("CHARLIT: '");
                emit_char(n->astn_num.number.integer);
                printf("'\n");
                break;
            } else {
                printf("CONSTANT (");
                if (!n->astn_num.number.is_signed) printf("UNSIGNED ");
                printf("%s): ", int_types_str[n->astn_num.number.aux_type]);
                if (n->astn_num.number.aux_type < s_REAL)
                    printf("%llu\n", n->astn_num.number.integer);
                else
                    printf("%Lg\n", n->astn_num.number.real);
                break;
            }

/**/    case ASTN_ASSIGN:
            printf("ASSIGNMENT\n");
            tabs++;
                print_ast(n->astn_assign.left);
                print_ast(n->astn_assign.right);
            tabs--; break;

/**/    case ASTN_IDENT:
            printf("IDENT: %s\n", n->astn_ident.ident);
            break;

/**/    case ASTN_STRLIT:
            printf("STRING: \"");
            for (int i=0; i<n->astn_strlit.strlit.len; i++) {
                emit_char(n->astn_strlit.strlit.str[i]);
            }
            printf("\"\n");
            break;

/**/    case ASTN_BINOP:
            printf("BINARY OP ");
            switch (n->astn_binop.op) {
                case SHL:   printf("<<\n"); break;
                case SHR:   printf(">>\n"); break;
                case LTEQ:  printf("<=\n"); break;
                case GTEQ:  printf(">=\n"); break;
                case EQEQ:  printf("==\n"); break;
                case NOTEQ: printf("!=\n"); break;
                case LOGAND:printf("&&\n"); break;
                case LOGOR: printf("||\n"); break;
                default:    printf("%c\n", n->astn_binop.op); break;
            }
            tabs++;
                print_ast(n->astn_binop.left);
                print_ast(n->astn_binop.right);
            tabs--; break;

/**/     case ASTN_FNCALL: // wip
            printf("FNCALL w/ %d args\n", n->astn_fncall.argcount);
            astn *arg = n->astn_fncall.args;
            tabs++; 
                print_ast(n->astn_fncall.fn);
                for (int i=0; i<n->astn_fncall.argcount; i++) {
                    print_ast(0); printf("ARG %d\n", i);
                    tabs++;
                        print_ast(arg->astn_list.me);
                    tabs--;
                    arg=arg->astn_list.next;
                }
            tabs--; break;

/**/    case ASTN_SELECT:
            printf("SELECT\n");
            tabs++;
                print_ast(n->astn_select.parent);
                print_ast(n->astn_select.member);
            tabs--; break;

/**/    case ASTN_UNOP:
            printf("UNOP ");
            switch (n->astn_unop.op) {
                case PLUSPLUS:      printf("POSTINC\n");                break;
                case MINUSMINUS:    printf("POSTDEC\n");                break;
                case '*':           printf("DEREF\n");                  break;
                case '&':           printf("ADDRESSOF\n");              break;
                default:            printf("%c\n", n->astn_unop.op);    break;
            }
            tabs++;
                print_ast(n->astn_unop.target);
            tabs--; break;

/**/    case ASTN_SIZEOF:
            printf("SIZEOF\n");
            tabs++;
                print_ast(n->astn_sizeof.target);
            tabs--; break;

/**/    case ASTN_TERN:
            printf("TERNARY\n");
            tabs++;
                print_ast(0); printf("IF:\n");
                tabs++; print_ast(n->astn_tern.cond); tabs--;

                print_ast(0); printf("THEN:\n");
                tabs++; print_ast(n->astn_tern.t_then); tabs--;

                print_ast(0); printf("ELSE:\n");
                tabs++; print_ast(n->astn_tern.t_else); tabs--;
            tabs--; break;

/**/    case ASTN_LIST: // unused
            print_ast(n->astn_list.me);
    }
}

astn *cassign_alloc(int op, astn* left, astn* right) {
    astn *n=astn_alloc(ASTN_ASSIGN);
    n->astn_assign.left=left;
    n->astn_assign.right=binop_alloc(op, left, right);
    return n;
}

astn *binop_alloc(int op, astn* left, astn* right) {
    astn *n=astn_alloc(ASTN_BINOP);
    n->astn_binop.op=op;
    n->astn_binop.left=left;
    n->astn_binop.right=right;
    return n;
}

astn *unop_alloc(int op, astn* target) {
    astn *n=astn_alloc(ASTN_UNOP);
    n->astn_unop.op=op;
    n->astn_unop.target=target;
    return n;
}

astn *list_alloc(astn* me) {
    astn *l=astn_alloc(ASTN_LIST);
    l->astn_list.me=me;
    l->astn_list.next=NULL;
    return l;
}
//              (arg to add)(head of ll)
astn *list_append(astn* new, astn* head) {
    astn *n=list_alloc(new);
    while (head->astn_list.next) head=head->astn_list.next;
    head->astn_list.next = n;
    return n;
}

int list_measure(astn* head) {
    int c = 0;
    while ((head=head->astn_list.next)) {
        c++;
    }
    return c + 1;
}
