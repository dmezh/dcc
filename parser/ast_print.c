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
    for (int i=0; i<tabs; i++) eprintf("  ");
    if (!n) return; // if we just want to print tabs, pass NULL
    switch (n->type) {
    case ASTN_NUM:
        print_number(&n->Num.number, stderr);
        eprintf("\n");
        break;

    case ASTN_ASSIGN:
        eprintf("ASSIGNMENT\n");
        tabs++;
            print_ast(n->Assign.left);
            print_ast(n->Assign.right);
        tabs--; break;

    case ASTN_IDENT:
        //eprintf("DBG: I ident am %p\n", (void*)n);
        eprintf("IDENT: %s\n", n->Ident.ident);
        break;

    case ASTN_STRLIT:
        eprintf("STRING: \"");
        for (size_t i=0; i<n->Strlit.strlit.len; i++) {
            emit_char(n->Strlit.strlit.str[i], stderr);
        }
        eprintf("\"\n");
        break;

    case ASTN_BINOP:
        eprintf("BINARY OP ");
        switch (n->Binop.op) {
            case SHL:   eprintf("<<\n"); break;
            case SHR:   eprintf(">>\n"); break;
            case LTEQ:  eprintf("<=\n"); break;
            case GTEQ:  eprintf(">=\n"); break;
            case EQEQ:  eprintf("==\n"); break;
            case NOTEQ: eprintf("!=\n"); break;
            case LOGAND:eprintf("&&\n"); break;
            case LOGOR: eprintf("||\n"); break;
            default:    eprintf("%c\n", n->Binop.op); break;
        }
        tabs++;
            print_ast(n->Binop.left);
            print_ast(n->Binop.right);
        tabs--; break;

    case ASTN_FNCALL: // wip
        eprintf("FNCALL w/ %d args\n", n->Fncall.argcount);
        astn *arg = n->Fncall.args;
        tabs++;
            print_ast(n->Fncall.fn);
            for (int i=0; i<n->Fncall.argcount; i++) {
                print_ast(0); eprintf("ARG %d\n", i);
                tabs++;
                    print_ast(arg->List.me);
                tabs--;
                arg=arg->List.next;
            }
        tabs--; break;

    case ASTN_SELECT:
        eprintf("SELECT\n");
        tabs++;
            print_ast(n->Select.parent);
            print_ast(n->Select.member);
        tabs--; break;

    case ASTN_UNOP:
        eprintf("UNOP ");
        switch (n->Unop.op) {
            case PLUSPLUS:      eprintf("POSTINC\n");                break;
            case MINUSMINUS:    eprintf("POSTDEC\n");                break;
            case '*':           eprintf("DEREF\n");                  break;
            case '&':           eprintf("ADDRESSOF\n");              break;
            default:            eprintf("%c\n", n->Unop.op);    break;
        }
        tabs++;
            print_ast(n->Unop.target);
        tabs--; break;

    case ASTN_SIZEOF:
        eprintf("SIZEOF\n");
        tabs++;
            print_ast(n->Sizeof.target);
        tabs--; break;

    case ASTN_TERN:
        eprintf("TERNARY\n");
        tabs++;
            print_ast(0); eprintf("IF:\n");
            tabs++; print_ast(n->Tern.cond); tabs--;

                print_ast(0); eprintf("THEN:\n");
                tabs++; print_ast(n->Tern.t_then); tabs--;

                print_ast(0); eprintf("ELSE:\n");
                tabs++; print_ast(n->Tern.t_else); tabs--;
            tabs--; break;

    case ASTN_LIST:
        //eprintf("LIST:\n");
        tabs++;
            while (n) {
                //print_ast(NULL);
                //eprintf("LIST ELEMENT:\n");
                tabs++;
                    print_ast(n->List.me);
                tabs--;
                n = n->List.next;
                eprintf("\n");
            }
        tabs--;
        break;

    case ASTN_TYPESPEC:
        eprintf("TYPESPEC ");
        if (n->Typespec.is_tagtype) {
            tabs++;
                print_ast(NULL);
                st_dump_single_given(n->Typespec.symbol->members);
            tabs--;
        } else {
            switch (n->Typespec.spec) {
                case TS_VOID:       eprintf("VOID");         break;
                case TS_CHAR:       eprintf("CHAR");         break;
                case TS_SHORT:      eprintf("SHORT");        break;
                case TS_INT:        eprintf("INT");          break;
                case TS_LONG:       eprintf("LONG");         break;
                case TS_FLOAT:      eprintf("FLOAT");        break;
                case TS_DOUBLE:     eprintf("DOUBLE");       break;
                case TS_SIGNED:     eprintf("SIGNED");       break;
                case TS_UNSIGNED:   eprintf("UNSIGNED");     break;
                case TS__BOOL:      eprintf("_BOOL");        break;
                case TS__COMPLEX:   eprintf("_COMPLEX");     break;
                default:            die("invalid typespec");
            }
        }
        eprintf("\n");
        if (n->Typespec.next) {
            tabs++;
                print_ast(n->Typespec.next);
            tabs--;
        }
        break;

    case ASTN_TYPEQUAL:
        eprintf("TYPEQUAL ");
        switch (n->Typequal.qual) {
            case TQ_CONST:      eprintf("CONST");        break;
            case TQ_RESTRICT:   eprintf("RESTRICT");     break;
            case TQ_VOLATILE:   eprintf("VOLATILE");     break;
            default:            die("invalid typequal");
        }
        eprintf("\n");
        if (n->Typequal.next) {
            tabs++;
                print_ast(n->Typequal.next);
            tabs--;
        }
        break;

    case ASTN_STORSPEC:
        eprintf("STORAGESPEC ");
        switch (n->Storspec.spec) {
            case SS_EXTERN:     eprintf("EXTERN");       break;
            case SS_STATIC:     eprintf("STATIC");       break;
            case SS_AUTO:       eprintf("AUTO");         break;
            case SS_REGISTER:   eprintf("REGISTER");     break;
            default:            die("invalid storspec");
        }
        eprintf("\n");
        if (n->Storspec.next) {
            tabs++;
                print_ast(n->Storspec.next);
            tabs--;
        }
        break;

    case ASTN_TYPE:
        // if if if if if if if if if if
        if (n->Type.is_const) eprintf("CONST ");
        if (n->Type.is_restrict) eprintf("RESTRICT ");
        if (n->Type.is_volatile) eprintf("VOLATILE ");
        if (n->Type.is_atomic)   eprintf("ATOMIC ");
        if (n->Type.is_derived) {
            //eprintf("DBG: target is %p\n", (void*)n->Type.derived.target);
            if (n->Type.derived.type == t_ARRAY) {
                eprintf("ARRAY\n");
                tabs++;
                    print_ast(NULL);
                    eprintf("SIZE:");
                    if (n->Type.derived.size) {
                        eprintf("\n");
                        tabs++;
                            print_ast(n->Type.derived.size);
                        tabs--;
                    } else {
                        eprintf(" (NONE)\n");
                    }
                    print_ast(NULL);
                    eprintf("OF:\n");
                    tabs++;
                        print_ast(n->Type.derived.target);
                    tabs--;
                tabs--;
                eprintf("\n");
            } else {
                switch (n->Type.derived.type) {
                    case t_PTR:     eprintf("PTR TO");       break;
                    case t_FN:
                        eprintf("FN RETURNING\n");
                        if (n->Type.derived.param_list) {
                            tabs++;
                                print_ast(NULL);
                                eprintf("> with param list:\n");
                                tabs++;
                                    struct astn *a = n->Type.derived.param_list;
                                    while (a) {
                                        print_ast(a->List.me);
                                        if (a->List.me->type != ASTN_ELLIPSIS) {
                                            tabs++;
                                                print_ast(NULL);
                                                eprintf("> with type:\n");
                                                print_ast(a->List.me->Declrec.e->type);
                                            tabs--;
                                        }
                                        a = a->List.next;
                                    }
                                tabs--;
                            tabs--;
                        } else {
                            print_ast(NULL);
                            eprintf("> with no params\n");
                        }
                        break;
                    default:        die("invalid derived type");
                }
                eprintf("\n");
                tabs++;
                    if (n->Type.derived.target)
                        print_ast(n->Type.derived.target);
                    else
                        eprintf("NULL\n");
                tabs--;
            }
            //if (n->Type.derived.type == t_ARRAY) tabs--; // kludge!
        } else if (n->Type.is_tagtype) {
            if (n->Type.tagtype.symbol->ident) // I really wanted to use the GNU :? here but I felt bad
                eprintf("struct %s\n", n->Type.tagtype.symbol->ident);
            else
                eprintf("struct (unnamed)\n");
        } else {
            if (n->Type.scalar.is_unsigned) eprintf("UNSIGNED ");
            switch (n->Type.scalar.type) {
                case t_VOID:            eprintf("VOID");             break;
                case t_CHAR:            eprintf("CHAR");             break;
                case t_SHORT:           eprintf("SHORT");            break;
                case t_INT:             eprintf("INT");              break;
                case t_LONG:            eprintf("LONG");             break;
                case t_LONGLONG:        eprintf("LONGLONG");         break;
                case t_BOOL:            eprintf("BOOL");             break;
                case t_FLOAT:           eprintf("FLOAT");            break;
                case t_DOUBLE:          eprintf("DOUBLE");           break;
                case t_LONGDOUBLE:      eprintf("LONGDOUBLE");       break;
                case t_FLOATCPLX:       eprintf("FLOATCPLX");        break;
                case t_DOUBLECPLX:      eprintf("DOUBLECPLX");       break;
                case t_LONGDOUBLECPLX:  eprintf("LONGDOUBLECPLX");   break;
                default:                die("invalid scalar type");
            }
            eprintf("\n");
        }
        break;

    case ASTN_DECL:
        eprintf("DECL:\n");
        tabs++;
            print_ast(NULL);
            eprintf("SPECS:\n");
            tabs++;
                print_ast(n->Decl.specs);
            tabs--;
            print_ast(NULL);
            eprintf("TYPE (with ident):\n");
            tabs++;
                print_ast(n->Decl.type);
            tabs--;
        tabs--;
        break;

    case ASTN_FNDEF:
        eprintf("FNDEF of\n");
        tabs++;
            print_ast(n->Fndef.decl); // should just be the name!
        tabs--;
        if (n->Fndef.param_list) {
            tabs++;
                print_ast(NULL);
                eprintf("> with param list:\n");
                tabs++;
                    struct astn *a = n->Fndef.param_list;
                    while (a) {
                        print_ast(a->List.me);
                        if (a->List.me->type != ASTN_ELLIPSIS) {
                            tabs++;
                                print_ast(NULL);
                                eprintf("> with type:\n");
                                print_ast(a->List.me->Declrec.e->type);
                            tabs--;
                        }
                        a = a->List.next;
                    }
                tabs--;
            tabs--;
        } else {
            print_ast(NULL);
            eprintf("> with no params\n");
        }
        break;

    case ASTN_ELLIPSIS:
        eprintf("ELLIPSIS\n");
        break;

    case ASTN_DECLREC:
        eprintf("(declaration of symbol <%s>)\n", n->Declrec.e->ident);
        break;

    case ASTN_SYMPTR:
        eprintf("symbol ");
        st_dump_entry(n->Symptr.e);
        break;

    case ASTN_IFELSE:
        eprintf("IF:\n");
        tabs++;
            print_ast(n->Ifelse.condition_s);
        tabs--;
        print_ast(NULL); eprintf("THEN:\n");
        tabs++;
            print_ast(n->Ifelse.then_s);
        tabs--;
        if (n->Ifelse.else_s) {
            print_ast(NULL); eprintf("ELSE:\n");
            tabs++;
                print_ast(n->Ifelse.else_s);
            tabs--;
        }
        break;

    case ASTN_SWITCH:
        eprintf("SWITCH:\n");
        tabs++;
            print_ast(NULL); eprintf("COND:\n");
            tabs++;
                print_ast(n->Switch.condition);
            tabs--;
            print_ast(NULL); eprintf("BODY:\n");
            tabs++;
                print_ast(n->Switch.body);
            tabs--;
        tabs--;
        break;

    case ASTN_WHILELOOP:
        if (n->Whileloop.is_dowhile) eprintf("DO ");
        eprintf("WHILE:\n");
        tabs++;
            print_ast(n->Whileloop.condition);
            print_ast(NULL); eprintf("BODY:\n");
            print_ast(n->Whileloop.body);
        tabs--;
        break;

    case ASTN_FORLOOP:
        eprintf("FOR:\n");
        tabs++;
            print_ast(NULL); eprintf("INIT:\n");
                tabs++;
                print_ast(n->Forloop.init);
            tabs--;
            print_ast(NULL); eprintf("CONDITION:\n");
            tabs++;
                print_ast(n->Forloop.condition);
            tabs--;
            print_ast(NULL); eprintf("ONEACH:\n");
            tabs++;
                print_ast(n->Forloop.oneach);
            tabs--;
            print_ast(NULL); eprintf("BODY:\n");
            tabs++;
                print_ast(n->Forloop.body);
            tabs--;
        tabs--;
        break;

    case ASTN_GOTO:
        eprintf("GOTO %s\n", n->Goto.ident->Ident.ident);
        break;
    case ASTN_CONTINUE:
        eprintf("CONTINUE\n");
        break;
    case ASTN_BREAK:
        eprintf("BREAK\n");
        break;
    case ASTN_RETURN:
        if (n->Return.ret) {
            eprintf("RETURN");
            tabs++; print_ast(n->Return.ret); tabs--;
        } else
            eprintf("RETURN;");
        break;
    case ASTN_LABEL:
        eprintf("LABEL %s:\n", n->Label.ident->Ident.ident);
        print_ast(n->Label.statement);
        break;
    case ASTN_CASE:
        if (n->Case.case_expr) {
            eprintf("CASE:"); print_ast(n->Case.case_expr);
        } else
            eprintf("DEFAULT:\n");
        print_ast(n->Case.statement);
        break;
    case ASTN_QTEMP:
        print_node(n, stderr);
        break;
    case ASTN_NOOP:
        eprintf("(NOOP)\n");
        break;
    default:
        die("Unhandled AST node type");
}
}
