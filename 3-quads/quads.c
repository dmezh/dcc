#include "quads.h"

#include "ast.h"
#include "parser.tab.h" // for token binop values
#include "quads_cf.h"
#include "quads_print.h"
#include "util.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>

unsigned temp_count = 0;

// allocate temporary
astn* qtemp_alloc(unsigned size) {
    astn* n = astn_alloc(ASTN_QTEMP);
    n->astn_qtemp.tempno = ++temp_count;
    n->astn_qtemp.size = size;
    return n;
}

enum quad_op binop_to_quad_op(int binop) {
    switch (binop) {
        case '*': return Q_MUL;
        case '/': return Q_DIV;
        case '%': return Q_MOD;
        case '+': return Q_ADD;
        case '-': return Q_SUB;
        case SHL: return Q_SHL;
        case SHR: return Q_SHR;
        case '&': return Q_BWAND;
        case '^': return Q_BWXOR;
        case '|': return Q_BWOR;
        // condexpr stuff
        default: die("unhandled binop");
    }
    return -1;
}

// definitiely leaky
astn* decay_array(astn* a) {
    if (a->type == ASTN_SYMPTR) return decay_array(a->astn_symptr.e->type);
    if (a->type == ASTN_TYPE && a->astn_type.is_derived && a->astn_type.derived.type == t_ARRAY) {
        astn *n = astn_alloc(ASTN_TYPE);
        n->astn_type.is_derived = true;
        n->astn_type.derived.type = t_PTR;
        n->astn_type.derived.target = a->astn_type.derived.target;
        return n;
    }
    return a;
}

// 
/*
astn* check_ptr_binop(astn* n) {
    if (n->type == ASTN_SYMPTR) return check_ptr_binop(n->astn_symptr.e->type);
    if (n->type == ASTN_TYPE && n->astn_type.is_derived) {
        if (n->astn_type.derived.type == t_ARRAY) return decay_array(n);
        else if (n->astn_type.derived.type == t_PTR) return n;
        else die("undefined derived type");
    } else {
        return n;
    }
    return NULL;
}
*/

void quad_error(char* msg) {
    fprintf(stderr, "quad generation error: %s\n", msg);
    exit(-1);
}

// triggers on either ptr or array
bool isptr(astn* n) {
    if (n->type == ASTN_SYMPTR) return isptr(n->astn_symptr.e->type);
    return (n->type == ASTN_TYPE && n->astn_type.is_derived);
}

bool isarr(astn* n) {
    if (n->type == ASTN_SYMPTR) return isarr(n->astn_symptr.e->type);
    return (n->type == ASTN_TYPE && n->astn_type.is_derived && n->astn_type.derived.type == t_ARRAY);
}

bool isfn(astn *n) {
    return (n->type == ASTN_SYMPTR && n->astn_symptr.e->entry_type == STE_FN);
}

static astn* const_alloc(int num) {
    astn* n=astn_alloc(ASTN_NUM);
    n->astn_num.number.integer = num;
    n->astn_num.number.is_signed = true;
    n->astn_num.number.aux_type = s_INT;
    return n;
}

// works on arr or ptr
static astn* ptr_target(astn *n) {
    if (n->type == ASTN_SYMPTR) return ptr_target(n->astn_symptr.e->type);
    return n->astn_type.derived.target;
}

void todo(const char* msg) {
    fprintf(stderr, "TODO: %s\n", msg);
    exit(-1);
}

void cond_rvalue(astn *n, astn* left, astn* right, astn* target);

// emit args in reverse
void put_args(astn *head) {
    if (!head) return;
    put_args(list_next(head));
    emit(Q_ARG, gen_rvalue(list_data(head), NULL), NULL, NULL);
}

astn* gen_rvalue(astn* node, astn* target) {
    astn* temp;
    if (node->type == ASTN_FNCALL) {
        astn *fn = gen_rvalue(node->astn_fncall.fn, NULL);

        emit(Q_ARGBEGIN, NULL, NULL, NULL);
        astn *arg = node->astn_fncall.args;

        put_args(arg);
        /*
        while (arg) {
            emit(Q_ARG, gen_rvalue(list_data(arg), NULL), NULL, NULL);
            arg = list_next(arg);
        }
        */

        if (!target) target = qtemp_alloc(4);
        emit(Q_CALL, fn, NULL, target);
        return target;
    }
    if (node->type == ASTN_SYMPTR) {
        //struct astn_type *type = &node->astn_symptr.e->type->astn_type;
        //if (type->is_derived && type->derived.type == t_ARRAY) {
        if (isarr(node)) {
            astn *temp = qtemp_alloc(4); // address type
            emit(Q_LEA, node, NULL, temp);
            return temp;
        }
        else if (isfn(node)) {
            astn *temp = astn_alloc(ASTN_IDENT);
            temp->astn_ident.ident = node->astn_symptr.e->ident;
            return temp;
        }
        else {
            target = qtemp_alloc(4); // hardcoded
            emit(Q_MOV, node, NULL, target);
            return target;
        }
    }
    if (node->type == ASTN_NUM) return node;
    if (node->type == ASTN_STRLIT) return node;
    if (node->type == ASTN_BINOP) {
        astn* left = node->astn_binop.left;
        astn* right = node->astn_binop.right;

        bool l_isptr = isptr(left);
        bool r_isptr = isptr(right);       

        //decay_array(left);
        //decay_array(right);

        if (l_isptr && r_isptr)
            quad_error("can't add two pointers!\n"); // check for subtraction!!

        if (r_isptr) { // swap them if needed
            temp = right;
            right = left;
            left = temp;
            l_isptr = true; r_isptr = false;
        }

        astn *rval_left = gen_rvalue(left, NULL);
        astn *rval_right = gen_rvalue(right, NULL);

        if (node->astn_binop.op == '+') {
            if (l_isptr) { // ptr + int
                if (rval_right->type == ASTN_NUM) { // otherwise we'd emit a constant as an lvalue
                    temp = qtemp_alloc(4);
                    emit(Q_MUL, rval_right, const_alloc(get_sizeof(ptr_target(left))), temp);
                    rval_right = temp;
                }
                else
                    emit(Q_MUL, rval_right, const_alloc(get_sizeof(ptr_target(left))), rval_right);
            }
        }

        if (!target) target=qtemp_alloc(4); // hardcoded int!

        if (node->astn_binop.op == '<') {
            cond_rvalue(node, rval_left, rval_right, target);
        } else
            emit(binop_to_quad_op(node->astn_binop.op), rval_left, rval_right, target);
        return target;
    }
    if (node->type == ASTN_UNOP) {
        astn *utarget = node->astn_unop.target;

        switch (node->astn_unop.op) {
            case '*':
                printf("dumping utarget\n"); print_ast(utarget);

                // multidim arrays: needs fixing
                if (isarr(utarget))
                {   printf("yup\n");
                    return gen_rvalue(ptr_target(utarget), target);
                }
                ;
                astn* addr = gen_rvalue(utarget, NULL);

                if (!target) target = qtemp_alloc(4);
                emit(Q_LOAD, addr, NULL, target);
                return target;

            case MINUSMINUS:    todo("minusminus");
            case PLUSPLUS:      todo("plusplus");
            case '&':           todo("address-of");
            case '-':           todo("+pos");
            case '!':           todo("-neg");
            case '~':           todo("bwnot");
            default:
                die("unhandled unop");
        }
    }
    die("FUCK!");
    return NULL;
}

astn* gen_lvalue(astn *node, enum addr_modes *mode) {
    if (node->type == ASTN_SYMPTR) { *mode = MODE_DIRECT; return node; }
    if (node->type == ASTN_NUM) return NULL;
    if (node->type == ASTN_UNOP && node->astn_unop.op == '*') {
        *mode = MODE_INDIRECT;
        return gen_rvalue(node->astn_unop.target, NULL);
    }
    return NULL;
}


void gen_assign(astn *node) {
    enum addr_modes destmode;
    astn *dest = gen_lvalue(node->astn_assign.left, &destmode);
    if (!dest) {
        fprintf(stderr, "Error: not an lvalue: "); die("test");
        print_ast(node->astn_assign.left);
        exit(-1);
    }
    if (destmode == MODE_DIRECT) {
        astn *temp = gen_rvalue(node->astn_assign.right, NULL);
        emit(Q_MOV, temp, NULL, dest);
    } else {
        astn *temp = gen_rvalue(node->astn_assign.right, NULL);
        emit(Q_STORE, temp, NULL, dest);
    }
}

void gen_mov_bb(astn* target, int val) {
    astn *aval = const_alloc(val);
    emit(Q_MOV, aval, NULL, target);
}

void cond_rvalue(astn *n, astn* left, astn* right, astn* target) {
    BB* one = bb_alloc();
    BB* zero = bb_alloc();

    emit(Q_CMP, left, right, NULL);
    emit_branch(n->astn_binop.op, one, zero);
    
    BB *future = bb_alloc();

    current_bb = one;
    gen_mov_bb(target, 1);
    uncond_branch(future);
    current_bb = zero;
    gen_mov_bb(target, 0);
    uncond_branch(future);

    //astn *cond = astn_alloc(ASTN_BINOP);
    current_bb = future;
}

void emit(enum quad_op op, astn* src1, astn* src2, astn* target) {
    quad *q = safe_calloc(1, sizeof(quad));
    q->op = op;
    q->src1 = src1;
    q->src2 = src2;
    q->target = target;
    if (!current_bb->start) {
        current_bb->start = q;
        current_bb->cur = q;
    } else {
        current_bb->cur->next = q;
        q->prev = current_bb->cur;
        current_bb->cur = q;
    }
    //print_quad(q);
}

void gen_fn(st_entry *e) {
    bb_count = 0;
    cursor.fn = e->ident;

    BB *f = bb_alloc();
    current_bb = f;

    astn* s = e->body;
    gen_quads(s);

    print_bbs();
}

void gen_quads(astn *n) {
    switch (n->type) {
        case ASTN_LIST:
            while (n) {
                gen_quads(list_data(n));
                n = list_next(n);
            }
            break;
        case ASTN_ASSIGN:
            gen_assign(n);
            break;
        case ASTN_IFELSE:
            gen_if(n);
            break;
        case ASTN_DECLREC:
            break;
        case ASTN_WHILELOOP:
            gen_while(n);
            break;
        case ASTN_CONTINUE:
            uncond_branch(cursor.cont); // may leave behind unreachable code, optimizer would get rid of it
            break;
        case ASTN_BREAK:
            uncond_branch(cursor.brk); // may leave behind unreachable code, optimizer would get rid of it
            break;
        case ASTN_SWITCH:
            todo("switch statements");
            break;
        case ASTN_RETURN:
            ;
            st_entry *e = st_lookup(cursor.fn, NS_MISC);
            bool non_void = (e->type->astn_type.is_derived || e->type->astn_type.scalar.type != t_VOID);
            if (n->astn_return.ret) {
                if (!non_void)
                    printf("Warning: 'return' with a value in a void function\n");
                emit(Q_RET, gen_rvalue(n->astn_return.ret, NULL), NULL, NULL);
            }
            else {
                if (non_void)
                    printf("Warning: 'return' without a value in a nonvoid function\n");
                emit(Q_RET, NULL, NULL, NULL);
            }
            break;
        case ASTN_FNCALL:
            gen_rvalue(n, NULL);
            break;
        default:
            printf("skipping unknown astn for quads %d\n", n->type);
    }
}
