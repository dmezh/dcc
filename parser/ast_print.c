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
           print_number(&n->astn_num.number, stderr);
            fprintf(stderr, "n");
            break;

    case ASTN_ASSIGN:
            fprintf(stderr, "ASSIGNMENTn");
            tabs++;
                print_ast(n->astn_assign.left);
                print_ast(n->astn_assign.right);
            tabs--; break;

    case ASTN_IDENT:
            //fprintf(stderr, "DBG: I ident am %pn", (void*)n);
            fprintf(stderr, "IDENT: %sn", n->astn_ident.ident);
            break;

    case ASTN_STRLIT:
            fprintf(stderr, "STRING: \"");
            for (size_t i=0; i<n->astn_strlit.strlit.len; i++) {
                emit_char(n->astn_strlit.strlit.str[i], stderr);
            }
            fprintf(stderr, "\"\n");
            break;

    case ASTN_BINOP:
            fprintf(stderr, "BINARY OP ");
            switch (n->astn_binop.op) {
                case SHL:   fprintf(stderr, "<<n"); break;
                case SHR:   fprintf(stderr, ">>n"); break;
                case LTEQ:  fprintf(stderr, "<=n"); break;
                case GTEQ:  fprintf(stderr, ">=n"); break;
                case EQEQ:  fprintf(stderr, "==n"); break;
                case NOTEQ: fprintf(stderr, "!=n"); break;
                case LOGAND:fprintf(stderr, "&&n"); break;
                case LOGOR: fprintf(stderr, "||n"); break;
                default:    fprintf(stderr, "%cn", n->astn_binop.op); break;
            }
            tabs++;
                print_ast(n->astn_binop.left);
                print_ast(n->astn_binop.right);
            tabs--; break;

     case ASTN_FNCALL: // wip
            fprintf(stderr, "FNCALL w/ %d argsn", n->astn_fncall.argcount);
            astn *arg = n->astn_fncall.args;
            tabs++;
                print_ast(n->astn_fncall.fn);
                for (int i=0; i<n->astn_fncall.argcount; i++) {
                    print_ast(0); fprintf(stderr, "ARG %dn", i);
                    tabs++;
                        print_ast(arg->astn_list.me);
                    tabs--;
                    arg=arg->astn_list.next;
                }
            tabs--; break;

    case ASTN_SELECT:
            fprintf(stderr, "SELECTn");
            tabs++;
                print_ast(n->astn_select.parent);
                print_ast(n->astn_select.member);
            tabs--; break;

    case ASTN_UNOP:
            fprintf(stderr, "UNOP ");
            switch (n->astn_unop.op) {
                case PLUSPLUS:      fprintf(stderr, "POSTINCn");                break;
                case MINUSMINUS:    fprintf(stderr, "POSTDECn");                break;
                case '*':           fprintf(stderr, "DEREFn");                  break;
                case '&':           fprintf(stderr, "ADDRESSOFn");              break;
                default:            fprintf(stderr, "%cn", n->astn_unop.op);    break;
            }
            tabs++;
                print_ast(n->astn_unop.target);
            tabs--; break;

    case ASTN_SIZEOF:
            fprintf(stderr, "SIZEOFn");
            tabs++;
                print_ast(n->astn_sizeof.target);
            tabs--; break;

    case ASTN_TERN:
            fprintf(stderr, "TERNARYn");
            tabs++;
                print_ast(0); fprintf(stderr, "IF:n");
                tabs++; print_ast(n->astn_tern.cond); tabs--;

                print_ast(0); fprintf(stderr, "THEN:n");
                tabs++; print_ast(n->astn_tern.t_then); tabs--;

                print_ast(0); fprintf(stderr, "ELSE:n");
                tabs++; print_ast(n->astn_tern.t_else); tabs--;
            tabs--; break;

    case ASTN_LIST:
            //fprintf(stderr, "LIST:n");
            tabs++;
                while (n) {
                    //print_ast(NULL);
                    //fprintf(stderr, "LIST ELEMENT:n");
                    tabs++;
                        print_ast(n->astn_list.me);
                    tabs--;
                    n = n->astn_list.next;
                    fprintf(stderr, "n");
                }
            tabs--;
            break;
            
    case ASTN_TYPESPEC:
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
            fprintf(stderr, "n");
            if (n->astn_typespec.next) {
                tabs++;
                    print_ast(n->astn_typespec.next);
                tabs--;
            }
            break;

    case ASTN_TYPEQUAL:
            fprintf(stderr, "TYPEQUAL ");
            switch (n->astn_typequal.qual) {
                case TQ_CONST:      fprintf(stderr, "CONST");        break;
                case TQ_RESTRICT:   fprintf(stderr, "RESTRICT");     break;
                case TQ_VOLATILE:   fprintf(stderr, "VOLATILE");     break;
                default:            die("invalid typequal");
            }
            fprintf(stderr, "n");
            if (n->astn_typequal.next) {
                tabs++;
                    print_ast(n->astn_typequal.next);
                tabs--;
            }
            break;

    case ASTN_STORSPEC:
            fprintf(stderr, "STORAGESPEC ");
            switch (n->astn_storspec.spec) {
                case SS_EXTERN:     fprintf(stderr, "EXTERN");       break;
                case SS_STATIC:     fprintf(stderr, "STATIC");       break;
                case SS_AUTO:       fprintf(stderr, "AUTO");         break;
                case SS_REGISTER:   fprintf(stderr, "REGISTER");     break;
                default:            die("invalid storspec");
            }
            fprintf(stderr, "n");
            if (n->astn_storspec.next) {
                tabs++;
                    print_ast(n->astn_storspec.next);
                tabs--;
            }
            break;

    case ASTN_TYPE:
            // if if if if if if if if if if
            if (n->astn_type.is_const) fprintf(stderr, "CONST ");
            if (n->astn_type.is_restrict) fprintf(stderr, "RESTRICT ");
            if (n->astn_type.is_volatile) fprintf(stderr, "VOLATILE ");
            if (n->astn_type.is_atomic)   fprintf(stderr, "ATOMIC ");
            if (n->astn_type.is_derived) {
                //fprintf(stderr, "DBG: target is %pn", (void*)n->astn_type.derived.target);
                if (n->astn_type.derived.type == t_ARRAY) {
                    fprintf(stderr, "ARRAYn");
                    tabs++;
                        print_ast(NULL);
                        fprintf(stderr, "SIZE:");
                        if (n->astn_type.derived.size) {
                            fprintf(stderr, "n");
                            tabs++;
                                print_ast(n->astn_type.derived.size);
                            tabs--;
                        } else {
                            fprintf(stderr, " (NONE)n");
                        }
                        print_ast(NULL);
                        fprintf(stderr, "OF:n");
                        tabs++;
                            print_ast(n->astn_type.derived.target);
                        tabs--;
                    tabs--;
                    fprintf(stderr, "n");
                } else {
                    switch (n->astn_type.derived.type) {
                        case t_PTR:     fprintf(stderr, "PTR TO");       break;
                        case t_FN:      fprintf(stderr, "FN RETURNING"); break; // dead code at the moment
                        default:        die("invalid derived type");
                    }
                    fprintf(stderr, "n");
                    tabs++;
                        if (n->astn_type.derived.target)
                            print_ast(n->astn_type.derived.target);
                        else
                            fprintf(stderr, "NULLn");
                    tabs--;
                }
                //if (n->astn_type.derived.type == t_ARRAY) tabs--; // kludge!
            } else if (n->astn_type.is_tagtype) {
                if (n->astn_type.tagtype.symbol->ident) // I really wanted to use the GNU :? here but I felt bad
                    fprintf(stderr, "struct %sn", n->astn_type.tagtype.symbol->ident);
                else
                    fprintf(stderr, "struct (unnamed)n");
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
                fprintf(stderr, "n");
            }
            break;

    case ASTN_DECL:
            fprintf(stderr, "DECL:n");
            tabs++;
                print_ast(NULL);
                fprintf(stderr, "SPECS:n");
                tabs++;
                    print_ast(n->astn_decl.specs);
                tabs--;
                print_ast(NULL);
                fprintf(stderr, "TYPE (with ident):n");
                tabs++;
                    print_ast(n->astn_decl.type);
                tabs--;
            tabs--;
            break;

    case ASTN_FNDEF:
            fprintf(stderr, "FNDEF ofn");
            tabs++;
                print_ast(n->astn_fndef.decl); // should just be the name!
            tabs--;
            if (n->astn_fndef.param_list) {
                fprintf(stderr, " with param list:n");
                tabs++;
                    print_ast(n->astn_fndef.param_list);
                tabs--;
            } else {
                fprintf(stderr, "with no paramsn");
            }
            break;
    case ASTN_DECLREC:
            fprintf(stderr, "(declaration of symbol <%s>)n", n->astn_declrec.e->ident);
            break;

    case ASTN_SYMPTR:
            fprintf(stderr, "symbol ");
            st_dump_entry(n->astn_symptr.e);
            break;

    case ASTN_IFELSE:
            fprintf(stderr, "IF:n");
            tabs++;
                print_ast(n->astn_ifelse.condition_s);
            tabs--;
            print_ast(NULL); fprintf(stderr, "THEN:n");
            tabs++;
                print_ast(n->astn_ifelse.then_s);
            tabs--;
            if (n->astn_ifelse.else_s) {
                print_ast(NULL); fprintf(stderr, "ELSE:n");
                tabs++;
                    print_ast(n->astn_ifelse.else_s);
                tabs--;
            }
            break;

    case ASTN_SWITCH:
            fprintf(stderr, "SWITCH:n");
            tabs++;
                print_ast(NULL); fprintf(stderr, "COND:n");
                tabs++;
                    print_ast(n->astn_switch.condition);
                tabs--;
                print_ast(NULL); fprintf(stderr, "BODY:n");
                tabs++;
                    print_ast(n->astn_switch.body);
                tabs--;
            tabs--;
            break;

    case ASTN_WHILELOOP:
            if (n->astn_whileloop.is_dowhile) fprintf(stderr, "DO ");
            fprintf(stderr, "WHILE:n");
            tabs++;
                print_ast(n->astn_whileloop.condition);
                print_ast(NULL); fprintf(stderr, "BODY:n");
                print_ast(n->astn_whileloop.body);
            tabs--;
            break;

    case ASTN_FORLOOP:
            fprintf(stderr, "FOR:n");
            tabs++;
                print_ast(NULL); fprintf(stderr, "INIT:n");
                tabs++;
                    print_ast(n->astn_forloop.init);
                tabs--;
                print_ast(NULL); fprintf(stderr, "CONDITION:n");
                tabs++;
                    print_ast(n->astn_forloop.condition);
                tabs--;
                print_ast(NULL); fprintf(stderr, "ONEACH:n");
                tabs++;
                    print_ast(n->astn_forloop.oneach);
                tabs--;
                print_ast(NULL); fprintf(stderr, "BODY:n");
                tabs++;
                    print_ast(n->astn_forloop.body);
                tabs--;
            tabs--;
            break;

    case ASTN_GOTO:
            fprintf(stderr, "GOTO %sn", n->astn_goto.ident->astn_ident.ident);
            break;
    case ASTN_CONTINUE:
            fprintf(stderr, "CONTINUEn");
            break;
    case ASTN_BREAK:
            fprintf(stderr, "BREAKn");
            break;
    case ASTN_RETURN:
            if (n->astn_return.ret) {
                fprintf(stderr, "RETURN");
                tabs++; print_ast(n->astn_return.ret); tabs--;
            } else
                fprintf(stderr, "RETURN;");
            break;
    case ASTN_LABEL:
            fprintf(stderr, "LABEL %s:n", n->astn_label.ident->astn_ident.ident);
            print_ast(n->astn_label.statement);
            break;
    case ASTN_CASE:
            if (n->astn_case.case_expr) {
                fprintf(stderr, "CASE:"); print_ast(n->astn_case.case_expr);
            } else
                fprintf(stderr, "DEFAULT:n");
            print_ast(n->astn_case.statement);
            break;
    case ASTN_QTEMP:
            print_node(n, stderr);
            break;
        default:
            die("Unhandled AST node type");
    }
}
