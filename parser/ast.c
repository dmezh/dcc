/*
 * ast.c
 * 
 * This file contains functions for the construction and manipulation of the AST.
 * x_alloc functions all allocate, potentially populate, and return new AST nodes.
 * All AST node allocation should be done internally with astn_alloc. 
 */

#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "charutil.h"
#include "parser.tab.h"
#include "quads_print.h"
#include "util.h"

/*
 * Recursively dump AST, starting at n and ending when we can't go any deeper.
 */
void print_ast(const astn *n) {
    static int tabs = 0;     //     -> __ <- two spaces
    for (int i=0; i<tabs; i++) fprintf(stderr, "  ");
    if (!n) return; // if we just want to print tabs, pass NULL
    switch (n->type) {

/**/    case ASTN_NUM:
/*
            if (n->astn_num.number.aux_type == s_CHARLIT) {
                printf("CHARLIT: '");
                emit_char(n->astn_num.number.integer);
                fprintf(stderr, "'\n");
                break;
            } else {
                fprintf(stderr, "CONSTANT (");
                if (!n->astn_num.number.is_signed) fprintf(stderr, "UNSIGNED ");
                fprintf(stderr, "%s): ", int_types_str[n->astn_num.number.aux_type]);
                if (n->astn_num.number.aux_type < s_REAL)
                    fprintf(stderr, "%llu\n", n->astn_num.number.integer);
                else
                    fprintf(stderr, "%Lg\n", n->astn_num.number.real);
                break;
            }
*/
            print_number(&n->astn_num.number, stderr);
            fprintf(stderr, "\n");
            break;

/**/    case ASTN_ASSIGN:
            fprintf(stderr, "ASSIGNMENT\n");
            tabs++;
                print_ast(n->astn_assign.left);
                print_ast(n->astn_assign.right);
            tabs--; break;

/**/    case ASTN_IDENT:
            //fprintf(stderr, "DBG: I ident am %p\n", (void*)n);
            fprintf(stderr, "IDENT: %s\n", n->astn_ident.ident);
            break;

/**/    case ASTN_STRLIT:
            fprintf(stderr, "STRING: \"");
            for (size_t i=0; i<n->astn_strlit.strlit.len; i++) {
                emit_char(n->astn_strlit.strlit.str[i], stderr);
            }
            fprintf(stderr, "\"\n");
            break;

/**/    case ASTN_BINOP:
            fprintf(stderr, "BINARY OP ");
            switch (n->astn_binop.op) {
                case SHL:   fprintf(stderr, "<<\n"); break;
                case SHR:   fprintf(stderr, ">>\n"); break;
                case LTEQ:  fprintf(stderr, "<=\n"); break;
                case GTEQ:  fprintf(stderr, ">=\n"); break;
                case EQEQ:  fprintf(stderr, "==\n"); break;
                case NOTEQ: fprintf(stderr, "!=\n"); break;
                case LOGAND:fprintf(stderr, "&&\n"); break;
                case LOGOR: fprintf(stderr, "||\n"); break;
                default:    fprintf(stderr, "%c\n", n->astn_binop.op); break;
            }
            tabs++;
                print_ast(n->astn_binop.left);
                print_ast(n->astn_binop.right);
            tabs--; break;

/**/     case ASTN_FNCALL: // wip
            fprintf(stderr, "FNCALL w/ %d args\n", n->astn_fncall.argcount);
            astn *arg = n->astn_fncall.args;
            tabs++;
                print_ast(n->astn_fncall.fn);
                for (int i=0; i<n->astn_fncall.argcount; i++) {
                    print_ast(0); fprintf(stderr, "ARG %d\n", i);
                    tabs++;
                        print_ast(arg->astn_list.me);
                    tabs--;
                    arg=arg->astn_list.next;
                }
            tabs--; break;

/**/    case ASTN_SELECT:
            fprintf(stderr, "SELECT\n");
            tabs++;
                print_ast(n->astn_select.parent);
                print_ast(n->astn_select.member);
            tabs--; break;

/**/    case ASTN_UNOP:
            fprintf(stderr, "UNOP ");
            switch (n->astn_unop.op) {
                case PLUSPLUS:      fprintf(stderr, "POSTINC\n");                break;
                case MINUSMINUS:    fprintf(stderr, "POSTDEC\n");                break;
                case '*':           fprintf(stderr, "DEREF\n");                  break;
                case '&':           fprintf(stderr, "ADDRESSOF\n");              break;
                default:            fprintf(stderr, "%c\n", n->astn_unop.op);    break;
            }
            tabs++;
                print_ast(n->astn_unop.target);
            tabs--; break;

/**/    case ASTN_SIZEOF:
            fprintf(stderr, "SIZEOF\n");
            tabs++;
                print_ast(n->astn_sizeof.target);
            tabs--; break;

/**/    case ASTN_TERN:
            fprintf(stderr, "TERNARY\n");
            tabs++;
                print_ast(0); fprintf(stderr, "IF:\n");
                tabs++; print_ast(n->astn_tern.cond); tabs--;

                print_ast(0); fprintf(stderr, "THEN:\n");
                tabs++; print_ast(n->astn_tern.t_then); tabs--;

                print_ast(0); fprintf(stderr, "ELSE:\n");
                tabs++; print_ast(n->astn_tern.t_else); tabs--;
            tabs--; break;

/**/    case ASTN_LIST:
            //fprintf(stderr, "LIST:\n");
            tabs++;
                while (n) {
                    //print_ast(NULL);
                    //fprintf(stderr, "LIST ELEMENT:\n");
                    tabs++;
                        print_ast(n->astn_list.me);
                    tabs--;
                    n = n->astn_list.next;
                    fprintf(stderr, "\n");
                }
            tabs--;
            break;
            
/**/    case ASTN_TYPESPEC:
            fprintf(stderr, "TYPESPEC ");
            if (n->astn_typespec.is_tagtype) {
                tabs++;
                    print_ast(NULL);
                    st_dump_single(n->astn_typespec.symbol->members);
                tabs--;
            } else {
                switch (n->astn_typespec.spec) {
                    case TS_VOID:       fprintf(stderr, "VOID");         break;
                    case TS_CHAR:       fprintf(stderr, "CHAR");         break;
                    case TS_SHORT:      fprintf(stderr, "SHORT");        break;
                    case TS_INT:        fprintf(stderr, "INT");          break;
                    case TS_LONG:       fprintf(stderr, "LONG");         break;
                    case TS_FLOAT:      fprintf(stderr, "FLOAT");        break;
                    case TS_DOUBLE:     fprintf(stderr, "DOUBLE");       break;
                    case TS_SIGNED:     fprintf(stderr, "SIGNED");       break;
                    case TS_UNSIGNED:   fprintf(stderr, "UNSIGNED");     break;
                    case TS__BOOL:      fprintf(stderr, "_BOOL");        break;
                    case TS__COMPLEX:   fprintf(stderr, "_COMPLEX");     break;
                    default:            die("invalid typespec");
                }
            }
            fprintf(stderr, "\n");
            if (n->astn_typespec.next) {
                tabs++;
                    print_ast(n->astn_typespec.next);
                tabs--;
            }
            break;

/**/    case ASTN_TYPEQUAL:
            fprintf(stderr, "TYPEQUAL ");
            switch (n->astn_typequal.qual) {
                case TQ_CONST:      fprintf(stderr, "CONST");        break;
                case TQ_RESTRICT:   fprintf(stderr, "RESTRICT");     break;
                case TQ_VOLATILE:   fprintf(stderr, "VOLATILE");     break;
                default:            die("invalid typequal");
            }
            fprintf(stderr, "\n");
            if (n->astn_typequal.next) {
                tabs++;
                    print_ast(n->astn_typequal.next);
                tabs--;
            }
            break;

/**/    case ASTN_STORSPEC:
            fprintf(stderr, "STORAGESPEC ");
            switch (n->astn_storspec.spec) {
                case SS_EXTERN:     fprintf(stderr, "EXTERN");       break;
                case SS_STATIC:     fprintf(stderr, "STATIC");       break;
                case SS_AUTO:       fprintf(stderr, "AUTO");         break;
                case SS_REGISTER:   fprintf(stderr, "REGISTER");     break;
                default:            die("invalid storspec");
            }
            fprintf(stderr, "\n");
            if (n->astn_storspec.next) {
                tabs++;
                    print_ast(n->astn_storspec.next);
                tabs--;
            }
            break;

/**/    case ASTN_TYPE:
            // if if if if if if if if if if
            if (n->astn_type.is_const) fprintf(stderr, "CONST ");
            if (n->astn_type.is_restrict) fprintf(stderr, "RESTRICT ");
            if (n->astn_type.is_volatile) fprintf(stderr, "VOLATILE ");
            if (n->astn_type.is_atomic)   fprintf(stderr, "ATOMIC ");
            if (n->astn_type.is_derived) {
                //fprintf(stderr, "DBG: target is %p\n", (void*)n->astn_type.derived.target);
                if (n->astn_type.derived.type == t_ARRAY) {
                    fprintf(stderr, "ARRAY\n");
                    tabs++;
                        print_ast(NULL);
                        fprintf(stderr, "SIZE:");
                        if (n->astn_type.derived.size) {
                            fprintf(stderr, "\n");
                            tabs++;
                                print_ast(n->astn_type.derived.size);
                            tabs--;
                        } else {
                            fprintf(stderr, " (NONE)\n");
                        }
                        print_ast(NULL);
                        fprintf(stderr, "OF:\n");
                        tabs++;
                            print_ast(n->astn_type.derived.target);
                        tabs--;
                    tabs--;
                    fprintf(stderr, "\n");
                } else {
                    switch (n->astn_type.derived.type) {
                        case t_PTR:     fprintf(stderr, "PTR TO");       break;
                        case t_FN:      fprintf(stderr, "FN RETURNING"); break; // dead code at the moment
                        default:        die("invalid derived type");
                    }
                    fprintf(stderr, "\n");
                    tabs++;
                        if (n->astn_type.derived.target)
                            print_ast(n->astn_type.derived.target);
                        else
                            fprintf(stderr, "NULL\n");
                    tabs--;
                }
                //if (n->astn_type.derived.type == t_ARRAY) tabs--; // kludge!
            } else if (n->astn_type.is_tagtype) {
                if (n->astn_type.tagtype.symbol->ident) // I really wanted to use the GNU :? here but I felt bad
                    fprintf(stderr, "struct %s\n", n->astn_type.tagtype.symbol->ident);
                else
                    fprintf(stderr, "struct (unnamed)\n");
            } else {
                if (n->astn_type.scalar.is_unsigned) fprintf(stderr, "UNSIGNED ");
                switch (n->astn_type.scalar.type) {
                    case t_VOID:            fprintf(stderr, "VOID");             break;
                    case t_CHAR:            fprintf(stderr, "CHAR");             break;
                    case t_SHORT:           fprintf(stderr, "SHORT");            break;
                    case t_INT:             fprintf(stderr, "INT");              break;
                    case t_LONG:            fprintf(stderr, "LONG");             break;
                    case t_LONGLONG:        fprintf(stderr, "LONGLONG");         break;
                    case t_BOOL:            fprintf(stderr, "BOOL");             break;
                    case t_FLOAT:           fprintf(stderr, "FLOAT");            break;
                    case t_DOUBLE:          fprintf(stderr, "DOUBLE");           break;
                    case t_LONGDOUBLE:      fprintf(stderr, "LONGDOUBLE");       break;
                    case t_FLOATCPLX:       fprintf(stderr, "FLOATCPLX");        break;
                    case t_DOUBLECPLX:      fprintf(stderr, "DOUBLECPLX");       break;
                    case t_LONGDOUBLECPLX:  fprintf(stderr, "LONGDOUBLECPLX");   break;
                    default:                die("invalid scalar type");
                }
                fprintf(stderr, "\n");
            }
            break;

 /**/   case ASTN_DECL:
            fprintf(stderr, "DECL:\n");
            tabs++;
                print_ast(NULL);
                fprintf(stderr, "SPECS:\n");
                tabs++;
                    print_ast(n->astn_decl.specs);
                tabs--;
                print_ast(NULL);
                fprintf(stderr, "TYPE (with ident):\n");
                tabs++;
                    print_ast(n->astn_decl.type);
                tabs--;
            tabs--;
            break;

/**/    case ASTN_FNDEF:
            fprintf(stderr, "FNDEF of\n");
            tabs++;
                print_ast(n->astn_fndef.decl); // should just be the name!
            tabs--;
            if (n->astn_fndef.param_list) {
                fprintf(stderr, " with param list:\n");
                tabs++;
                    print_ast(n->astn_fndef.param_list);
                tabs--;
            } else {
                fprintf(stderr, "with no params\n");
            }
            break;
/**/    case ASTN_DECLREC:
            fprintf(stderr, "(declaration of symbol <%s>)\n", n->astn_declrec.e->ident);
            break;

/**/    case ASTN_SYMPTR:
            fprintf(stderr, "symbol ");
            st_dump_entry(n->astn_symptr.e);
            break;

/**/    case ASTN_IFELSE:
            fprintf(stderr, "IF:\n");
            tabs++;
                print_ast(n->astn_ifelse.condition_s);
            tabs--;
            print_ast(NULL); fprintf(stderr, "THEN:\n");
            tabs++;
                print_ast(n->astn_ifelse.then_s);
            tabs--;
            if (n->astn_ifelse.else_s) {
                print_ast(NULL); fprintf(stderr, "ELSE:\n");
                tabs++;
                    print_ast(n->astn_ifelse.else_s);
                tabs--;
            }
            break;

/**/    case ASTN_SWITCH:
            fprintf(stderr, "SWITCH:\n");
            tabs++;
                print_ast(NULL); fprintf(stderr, "COND:\n");
                tabs++;
                    print_ast(n->astn_switch.condition);
                tabs--;
                print_ast(NULL); fprintf(stderr, "BODY:\n");
                tabs++;
                    print_ast(n->astn_switch.body);
                tabs--;
            tabs--;
            break;

/**/    case ASTN_WHILELOOP:
            if (n->astn_whileloop.is_dowhile) fprintf(stderr, "DO ");
            fprintf(stderr, "WHILE:\n");
            tabs++;
                print_ast(n->astn_whileloop.condition);
                print_ast(NULL); fprintf(stderr, "BODY:\n");
                print_ast(n->astn_whileloop.body);
            tabs--;
            break;

/**/    case ASTN_FORLOOP:
            fprintf(stderr, "FOR:\n");
            tabs++;
                print_ast(NULL); fprintf(stderr, "INIT:\n");
                tabs++;
                    print_ast(n->astn_forloop.init);
                tabs--;
                print_ast(NULL); fprintf(stderr, "CONDITION:\n");
                tabs++;
                    print_ast(n->astn_forloop.condition);
                tabs--;
                print_ast(NULL); fprintf(stderr, "ONEACH:\n");
                tabs++;
                    print_ast(n->astn_forloop.oneach);
                tabs--;
                print_ast(NULL); fprintf(stderr, "BODY:\n");
                tabs++;
                    print_ast(n->astn_forloop.body);
                tabs--;
            tabs--;
            break;

/**/    case ASTN_GOTO:
            fprintf(stderr, "GOTO %s\n", n->astn_goto.ident->astn_ident.ident);
            break;
/**/    case ASTN_CONTINUE:
            fprintf(stderr, "CONTINUE\n");
            break;
/**/    case ASTN_BREAK:
            fprintf(stderr, "BREAK\n");
            break;
/**/    case ASTN_RETURN:
            if (n->astn_return.ret) {
                fprintf(stderr, "RETURN");
                tabs++; print_ast(n->astn_return.ret); tabs--;
            } else
                fprintf(stderr, "RETURN;");
            break;
/**/    case ASTN_LABEL:
            fprintf(stderr, "LABEL %s:\n", n->astn_label.ident->astn_ident.ident);
            print_ast(n->astn_label.statement);
            break;
/**/    case ASTN_CASE:
            if (n->astn_case.case_expr) {
                fprintf(stderr, "CASE:"); print_ast(n->astn_case.case_expr);
            } else
                fprintf(stderr, "DEFAULT:\n");
            print_ast(n->astn_case.statement);
            break;
/**/    case ASTN_QTEMP:
            print_node(n, stderr);
            break;
        default:
            die("Unhandled AST node type");
    }
}

/*
 * Allocate single astn safely.
 * The entire compiler relies on the astn type being set, so we must do that.
 */
astn* astn_alloc(enum astn_types type) {
    astn *n = safe_calloc(1, sizeof(astn));
    n->type = type;
    return n;
}

/*
 * allocate complex assignment (*=, /=, etc) - 6.5.16
 */
astn *cassign_alloc(int op, astn *left, astn *right) {
    astn *n=astn_alloc(ASTN_ASSIGN);
    n->astn_assign.left=left;
    n->astn_assign.right=binop_alloc(op, left, right);
    return n;
}

/*
 * allocate binary operation
 */
astn *binop_alloc(int op, astn *left, astn *right) {
    astn *n=astn_alloc(ASTN_BINOP);
    n->astn_binop.op=op;
    n->astn_binop.left=left;
    n->astn_binop.right=right;
    return n;
}

/*
 * allocate unary operation
 */
astn *unop_alloc(int op, astn *target) {
    astn *n=astn_alloc(ASTN_UNOP);
    n->astn_unop.op=op;
    n->astn_unop.target=target;
    return n;
}

/*
 * allocate a list node
 */
astn *list_alloc(astn *me) {
    astn *l=astn_alloc(ASTN_LIST);
    l->astn_list.me=me;
    l->astn_list.next=NULL;
    return l;
}

/*
 * allocate a list node and append it to end of existing list (from head)
 * return: ptr to new node
 */
//              (arg to add)(head of ll)
astn *list_append(astn* new, astn *head) {
    astn *n=list_alloc(new);
    while (head->astn_list.next) head=head->astn_list.next;
    head->astn_list.next = n;
    return n;
}

/*
 *  get next node of list
 */
astn *list_next(astn* cur) {
    return cur->astn_list.next;
}

/*
 *  get current data element
 */
astn *list_data(astn* n) {
    return n->astn_list.me;
}

// DSA shit
// https://www.geeksforgeeks.org/reverse-a-linked-list/
void list_reverse(astn **l) {
    struct astn *prev=NULL, *current=*l, *next=NULL;
    while (current) {
        next = list_next(current);
        current->astn_list.next = prev;
        prev = current;
        current = next;
    }
    *l = prev;
}

/*
 * return length of AST list starting at head
 */
unsigned list_measure(const astn *head) {
    int c = 0;
    while ((head=head->astn_list.next)) {
        c++;
    }
    return c + 1;
}

/*
 * allocate type specifier node
 */
astn *typespec_alloc(enum typespec spec) {
    astn *n=astn_alloc(ASTN_TYPESPEC);
    n->astn_typespec.spec = spec;
    n->astn_typespec.next = NULL;
    //fprintf(stderr, "ALLOCATED TSPEC\n");
    return n;
}

/*
 * allocate type qualifier node
 */
astn *typequal_alloc(enum typequal qual) {
    astn *n=astn_alloc(ASTN_TYPEQUAL);
    n->astn_typequal.qual = qual;
    n->astn_typespec.next = NULL;
    return n;
}

/*
 * allocate storage specifier node
 */
astn *storspec_alloc(enum storspec spec) {
    astn *n=astn_alloc(ASTN_STORSPEC);
    n->astn_storspec.spec = spec;
    n->astn_storspec.next = NULL;
    return n;
}

/*
 * allocate derived type node
 */
astn *dtype_alloc(astn *target, enum der_types type) {
    astn *n=astn_alloc(ASTN_TYPE);
    n->astn_type.is_derived = true;
    n->astn_type.derived.type = type;
    n->astn_type.derived.target = target;
    //fprintf(stderr, "ALLOCATED DTYPE %p OF TYPE %s WITH TARGET %p\n",
    //        (void*)n, der_types_str[n->astn_type.derived.type], (void*)n->astn_type.derived.target);
    return n;
}

/*
 * allocate decl type node
 */
astn *decl_alloc(astn *specs, astn *type, astn *init, YYLTYPE context) {
    astn *n=astn_alloc(ASTN_DECL);
    n->astn_decl.specs=specs;
    n->astn_decl.type=type;
    n->astn_decl.context=context;
    n->astn_decl.init = init;
    return n;
}

/*
 * allocate strunion variant of astn_typespec
 */
astn *strunion_alloc(struct st_entry* symbol) {
    astn *n=astn_alloc(ASTN_TYPESPEC);
    n->astn_typespec.is_tagtype = true;
    n->astn_typespec.symbol = symbol;
    n->astn_typespec.next = NULL;
    return n;
}

astn *fndef_alloc(astn* decl, astn* param_list, symtab* scope) {
    astn *n=astn_alloc(ASTN_FNDEF);
    n->astn_fndef.decl = decl;
    n->astn_fndef.param_list = param_list;
    n->astn_fndef.scope = scope;
    return n;
}

astn *declrec_alloc(st_entry* e, astn* init) {
    astn *n=astn_alloc(ASTN_DECLREC);
    n->astn_declrec.e = e;
    n->astn_declrec.init = init;
    return n;
}

astn *symptr_alloc(st_entry* e) {
    astn *n=astn_alloc(ASTN_SYMPTR);
    n->astn_symptr.e = e;
    return n;
}

astn *ifelse_alloc(astn *cond_s, astn *then_s, astn *else_s) {
    astn *n=astn_alloc(ASTN_IFELSE);
    n->astn_ifelse.condition_s = cond_s;
    n->astn_ifelse.then_s = then_s;
    n->astn_ifelse.else_s = else_s;
    return n;
}

astn *whileloop_alloc(astn* cond_s, astn* body_s, bool is_dowhile) {
    astn *n=astn_alloc(ASTN_WHILELOOP);
    n->astn_whileloop.is_dowhile = is_dowhile;
    n->astn_whileloop.condition = cond_s;
    n->astn_whileloop.body = body_s;
    return n;
}

astn *forloop_alloc(astn *init, astn* condition, astn* oneach, astn* body) {
    astn *n=astn_alloc(ASTN_FORLOOP);
    n->astn_forloop.init = init;
    n->astn_forloop.condition = condition;
    n->astn_forloop.oneach = oneach;
    n->astn_forloop.body = body;
    return n;
}

// this shouldn't be here >:(
astn *do_decl(astn *decl) {
    astn *n = NULL;
    if (decl->type == ASTN_DECL) {
        n = declrec_alloc(begin_st_entry(decl, NS_MISC, decl->astn_decl.context), decl->astn_decl.init);
        st_reserve_stack(n->astn_declrec.e);
    }
    return n;
}

/*
 * set the final derived.target of a chain of derived type nodes, eg:
 * (ptr to)->(array of)->...->target
 */
void set_dtypechain_target(astn *top, astn *target) {
    while (top->astn_type.derived.target) {
        top = top->astn_type.derived.target;
    }
    //ffprintf(stderr, stderr, "setting target to %p, I arrived at %p\n", (void*)target, (void*)top);
    top->astn_type.derived.target = target;
}

/*
 * same as above, but replace the existing final target, not append past it
 */
void reset_dtypechain_target(astn *top, astn *target) {
    astn* last = top;
    while (top->type == ASTN_TYPE && top->astn_type.derived.target) {
        last = top;
        top = top->astn_type.derived.target;
    }
    //ffprintf(stderr, stderr, "resetting target to %p, I arrived at %p\n\n", (void*)target, (void*)last);
    last->astn_type.derived.target = target;
}

/*
 * takes the final node of the parent (the IDENT), makes it the final node of the child,
 * and connects the child to the parent
 */
void merge_dtypechains(astn *parent, astn *child) {
    set_dtypechain_target(child, get_dtypechain_target(parent));
    reset_dtypechain_target(parent, child);
}

/*
 * get last target of chain of derived types
 */
astn* get_dtypechain_target(astn* top) {
    while (top->type == ASTN_TYPE && top->astn_type.derived.target) {
        top = top->astn_type.derived.target;
    }
    return top;
}
