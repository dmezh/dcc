#include "ast_print.h"

#include <stdio.h>

#include "ast.h"
#include "charutil.h"
#include "parser.tab.h"
#include "quads_print.h"
#include "symtab.h"

/*
 * Recursively dump AST, starting at n and ending when we can't go any deeper.
 */
void print_ast(const astn *n) {
    static int tabs = 0;     //     -> __ <- two spaces
    for (int i=0; i<tabs; i++) fprintf(stderr, "  ");
    if (!n) return; // if we just want to print tabs, pass NULL
    switch (n->type) {
    case ASTN_NUM:
        print_number(&n->Num.number, stderr);
        fprintf(stderr, "\n");
        break;

    case ASTN_ASSIGN:
        fprintf(stderr, "ASSIGNMENT\n");
        tabs++;
            print_ast(n->Assign.left);
            print_ast(n->Assign.right);
        tabs--; break;

    case ASTN_IDENT:
        //fprintf(stderr, "DBG: I ident am %p\n", (void*)n);
        fprintf(stderr, "IDENT: %s\n", n->Ident.ident);
        break;

    case ASTN_STRLIT:
        fprintf(stderr, "STRING: \"");
        for (size_t i=0; i<n->Strlit.strlit.len; i++) {
            emit_char(n->Strlit.strlit.str[i], stderr);
        }
        fprintf(stderr, "\"\\n");
        break;

    case ASTN_BINOP:
        fprintf(stderr, "BINARY OP ");
        switch (n->Binop.op) {
            case SHL:   fprintf(stderr, "<<\n"); break;
            case SHR:   fprintf(stderr, ">>\n"); break;
            case LTEQ:  fprintf(stderr, "<=\n"); break;
            case GTEQ:  fprintf(stderr, ">=\n"); break;
            case EQEQ:  fprintf(stderr, "==\n"); break;
            case NOTEQ: fprintf(stderr, "!=\n"); break;
            case LOGAND:fprintf(stderr, "&&\n"); break;
            case LOGOR: fprintf(stderr, "||\n"); break;
            default:    fprintf(stderr, "%c\n", n->Binop.op); break;
        }
        tabs++;
            print_ast(n->Binop.left);
            print_ast(n->Binop.right);
        tabs--; break;

    case ASTN_FNCALL: // wip
        fprintf(stderr, "FNCALL w/ %d args\n", n->Fncall.argcount);
        astn *arg = n->Fncall.args;
        tabs++;
            print_ast(n->Fncall.fn);
            for (int i=0; i<n->Fncall.argcount; i++) {
                print_ast(0); fprintf(stderr, "ARG %d\n", i);
                tabs++;
                    print_ast(arg->List.me);
                tabs--;
                arg=arg->List.next;
            }
        tabs--; break;

    case ASTN_SELECT:
        fprintf(stderr, "SELECT\n");
        tabs++;
            print_ast(n->Select.parent);
            print_ast(n->Select.member);
        tabs--; break;

    case ASTN_UNOP:
        fprintf(stderr, "UNOP ");
        switch (n->Unop.op) {
            case PLUSPLUS:      fprintf(stderr, "POSTINC\n");                break;
            case MINUSMINUS:    fprintf(stderr, "POSTDEC\n");                break;
            case '*':           fprintf(stderr, "DEREF\n");                  break;
            case '&':           fprintf(stderr, "ADDRESSOF\n");              break;
            default:            fprintf(stderr, "%c\n", n->Unop.op);    break;
        }
        tabs++;
            print_ast(n->Unop.target);
        tabs--; break;

    case ASTN_SIZEOF:
        fprintf(stderr, "SIZEOF\n");
        tabs++;
            print_ast(n->Sizeof.target);
        tabs--; break;

    case ASTN_TERN:
        fprintf(stderr, "TERNARY\n");
        tabs++;
            print_ast(0); fprintf(stderr, "IF:\n");
            tabs++; print_ast(n->Tern.cond); tabs--;

                print_ast(0); fprintf(stderr, "THEN:\n");
                tabs++; print_ast(n->Tern.t_then); tabs--;

                print_ast(0); fprintf(stderr, "ELSE:\n");
                tabs++; print_ast(n->Tern.t_else); tabs--;
            tabs--; break;

    case ASTN_LIST:
        //fprintf(stderr, "LIST:\n");
        tabs++;
            while (n) {
                //print_ast(NULL);
                //fprintf(stderr, "LIST ELEMENT:\n");
                tabs++;
                    print_ast(n->List.me);
                tabs--;
                n = n->List.next;
                fprintf(stderr, "\n");
            }
        tabs--;
        break;

    case ASTN_TYPESPEC:
        fprintf(stderr, "TYPESPEC ");
        if (n->Typespec.is_tagtype) {
            tabs++;
                print_ast(NULL);
                st_dump_single_given(n->Typespec.symbol->members);
            tabs--;
        } else {
            switch (n->Typespec.spec) {
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
        if (n->Typespec.next) {
            tabs++;
                print_ast(n->Typespec.next);
            tabs--;
        }
        break;

    case ASTN_TYPEQUAL:
        fprintf(stderr, "TYPEQUAL ");
        switch (n->Typequal.qual) {
            case TQ_CONST:      fprintf(stderr, "CONST");        break;
            case TQ_RESTRICT:   fprintf(stderr, "RESTRICT");     break;
            case TQ_VOLATILE:   fprintf(stderr, "VOLATILE");     break;
            default:            die("invalid typequal");
        }
        fprintf(stderr, "\n");
        if (n->Typequal.next) {
            tabs++;
                print_ast(n->Typequal.next);
            tabs--;
        }
        break;

    case ASTN_STORSPEC:
        fprintf(stderr, "STORAGESPEC ");
        switch (n->Storspec.spec) {
            case SS_EXTERN:     fprintf(stderr, "EXTERN");       break;
            case SS_STATIC:     fprintf(stderr, "STATIC");       break;
            case SS_AUTO:       fprintf(stderr, "AUTO");         break;
            case SS_REGISTER:   fprintf(stderr, "REGISTER");     break;
            default:            die("invalid storspec");
        }
        fprintf(stderr, "\n");
        if (n->Storspec.next) {
            tabs++;
                print_ast(n->Storspec.next);
            tabs--;
        }
        break;

    case ASTN_TYPE:
        // if if if if if if if if if if
        if (n->Type.is_const) fprintf(stderr, "CONST ");
        if (n->Type.is_restrict) fprintf(stderr, "RESTRICT ");
        if (n->Type.is_volatile) fprintf(stderr, "VOLATILE ");
        if (n->Type.is_atomic)   fprintf(stderr, "ATOMIC ");
        if (n->Type.is_derived) {
            //fprintf(stderr, "DBG: target is %p\n", (void*)n->Type.derived.target);
            if (n->Type.derived.type == t_ARRAY) {
                fprintf(stderr, "ARRAY\n");
                tabs++;
                    print_ast(NULL);
                    fprintf(stderr, "SIZE:");
                    if (n->Type.derived.size) {
                        fprintf(stderr, "\n");
                        tabs++;
                            print_ast(n->Type.derived.size);
                        tabs--;
                    } else {
                        fprintf(stderr, " (NONE)\n");
                    }
                    print_ast(NULL);
                    fprintf(stderr, "OF:\n");
                    tabs++;
                        print_ast(n->Type.derived.target);
                    tabs--;
                tabs--;
                fprintf(stderr, "\n");
            } else {
                switch (n->Type.derived.type) {
                    case t_PTR:     fprintf(stderr, "PTR TO");       break;
                    case t_FN:      fprintf(stderr, "FN RETURNING"); break; // dead code at the moment
                    default:        die("invalid derived type");
                }
                fprintf(stderr, "\n");
                tabs++;
                    if (n->Type.derived.target)
                        print_ast(n->Type.derived.target);
                    else
                        fprintf(stderr, "NULL\n");
                tabs--;
            }
            //if (n->Type.derived.type == t_ARRAY) tabs--; // kludge!
        } else if (n->Type.is_tagtype) {
            if (n->Type.tagtype.symbol->ident) // I really wanted to use the GNU :? here but I felt bad
                fprintf(stderr, "struct %s\n", n->Type.tagtype.symbol->ident);
            else
                fprintf(stderr, "struct (unnamed)\n");
        } else {
            if (n->Type.scalar.is_unsigned) fprintf(stderr, "UNSIGNED ");
            switch (n->Type.scalar.type) {
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

    case ASTN_DECL:
        fprintf(stderr, "DECL:\n");
        tabs++;
            print_ast(NULL);
            fprintf(stderr, "SPECS:\n");
            tabs++;
                print_ast(n->Decl.specs);
            tabs--;
            print_ast(NULL);
            fprintf(stderr, "TYPE (with ident):\n");
            tabs++;
                print_ast(n->Decl.type);
            tabs--;
        tabs--;
        break;

    case ASTN_FNDEF:
        fprintf(stderr, "FNDEF of\n");
        tabs++;
            print_ast(n->Fndef.decl); // should just be the name!
        tabs--;
        if (n->Fndef.param_list) {
            tabs++;
                print_ast(NULL);
                fprintf(stderr, "> with param list:\n");
                tabs++;
                    struct astn *a = n->Fndef.param_list;
                    while (a) {
                        print_ast(a->List.me);
                        tabs++;
                            print_ast(NULL);
                            fprintf(stderr, "> with type:\n");
                            print_ast(a->List.me->Declrec.e->type);
                        tabs--;
                        a = a->List.next;
                    }
                tabs--;
            tabs--;
        } else {
            fprintf(stderr, "with no params\n");
        }
        break;

    case ASTN_DECLREC:
        fprintf(stderr, "(declaration of symbol <%s>)\n", n->Declrec.e->ident);
        break;

    case ASTN_SYMPTR:
        fprintf(stderr, "symbol ");
        st_dump_entry(n->Symptr.e);
        break;

    case ASTN_IFELSE:
        fprintf(stderr, "IF:\n");
        tabs++;
            print_ast(n->Ifelse.condition_s);
        tabs--;
        print_ast(NULL); fprintf(stderr, "THEN:\n");
        tabs++;
            print_ast(n->Ifelse.then_s);
        tabs--;
        if (n->Ifelse.else_s) {
            print_ast(NULL); fprintf(stderr, "ELSE:\n");
            tabs++;
                print_ast(n->Ifelse.else_s);
            tabs--;
        }
        break;

    case ASTN_SWITCH:
        fprintf(stderr, "SWITCH:\n");
        tabs++;
            print_ast(NULL); fprintf(stderr, "COND:\n");
            tabs++;
                print_ast(n->Switch.condition);
            tabs--;
            print_ast(NULL); fprintf(stderr, "BODY:\n");
            tabs++;
                print_ast(n->Switch.body);
            tabs--;
        tabs--;
        break;

    case ASTN_WHILELOOP:
        if (n->Whileloop.is_dowhile) fprintf(stderr, "DO ");
        fprintf(stderr, "WHILE:\n");
        tabs++;
            print_ast(n->Whileloop.condition);
            print_ast(NULL); fprintf(stderr, "BODY:\n");
            print_ast(n->Whileloop.body);
        tabs--;
        break;

    case ASTN_FORLOOP:
        fprintf(stderr, "FOR:\n");
        tabs++;
            print_ast(NULL); fprintf(stderr, "INIT:\n");
                tabs++;
                print_ast(n->Forloop.init);
            tabs--;
            print_ast(NULL); fprintf(stderr, "CONDITION:\n");
            tabs++;
                print_ast(n->Forloop.condition);
            tabs--;
            print_ast(NULL); fprintf(stderr, "ONEACH:\n");
            tabs++;
                print_ast(n->Forloop.oneach);
            tabs--;
            print_ast(NULL); fprintf(stderr, "BODY:\n");
            tabs++;
                print_ast(n->Forloop.body);
            tabs--;
        tabs--;
        break;

    case ASTN_GOTO:
        fprintf(stderr, "GOTO %s\n", n->Goto.ident->Ident.ident);
        break;
    case ASTN_CONTINUE:
        fprintf(stderr, "CONTINUE\n");
        break;
    case ASTN_BREAK:
        fprintf(stderr, "BREAK\n");
        break;
    case ASTN_RETURN:
        if (n->Return.ret) {
            fprintf(stderr, "RETURN");
            tabs++; print_ast(n->Return.ret); tabs--;
        } else
            fprintf(stderr, "RETURN;");
        break;
    case ASTN_LABEL:
        fprintf(stderr, "LABEL %s:\n", n->Label.ident->Ident.ident);
        print_ast(n->Label.statement);
        break;
    case ASTN_CASE:
        if (n->Case.case_expr) {
            fprintf(stderr, "CASE:"); print_ast(n->Case.case_expr);
        } else
            fprintf(stderr, "DEFAULT:\n");
        print_ast(n->Case.statement);
        break;
    case ASTN_QTEMP:
        print_node(n, stderr);
        break;
    case ASTN_NOOP:
        fprintf(stderr, "(NOOP)\n");
        break;
    default:
        die("Unhandled AST node type");
}
}
