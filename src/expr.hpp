#pragma once 
#include <iostream> 
#include "token.hpp" 

///////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: Expr needs to be represented in another way to retain data between functions and such.
// DO NOT FORGET TO DO THIS AS IT IS REALLY REALLY IMPORTANT TO THE INTERPRETER WORKING!!!!!!
///////////////////////////////////////////////////////////////////////////////////////////////

enum Expr_Type {
    binary_t,
    group_t,
    literal_t,
    unary_t
};

enum Lit_type {
    Lit_type_none,

    Lit_type_number,
    Lit_type_string,
    Lit_type_bool,
    Lit_type_null,
};

std::string Lit_type_to_string(Lit_type type) {
    switch(type) {
        case Lit_type_number:  return "Number"; break;
        case Lit_type_string:  return "String"; break;
        case Lit_type_bool:    return "Bool"; break;
        case Lit_type_null:    return "Null"; break;
    
        case Lit_type_none:    return "None"; break;
    
        default:
            return "Error"; break;
    }
}

struct Expr {
    Expr() {
        rhs = nullptr;
        op = Token_new(NONE, "", "", 0);
        lhs = nullptr;

        num_value = 0;
        s_value = "";
        b_value = false;

        lit_type = Lit_type_none;
    }

    void binary(Expr* v_lhs, Token v_op, Expr* v_rhs) {
        lhs = v_lhs;
        op = v_op;
        rhs = v_rhs;
        type = binary_t;
    }

    void unary(Token v_op, Expr* v_lhs) {
        op = v_op;
        lhs = v_lhs;

        type = unary_t;
    }

    void grouping(Expr* v_lhs) {
        lhs = v_lhs;
        type = group_t;
    }

    void literal(double num) {
        num_value = num;
        lit_type = Lit_type_number;
        type = literal_t;
    }

    void literal(std::string s) {
        s_value = s;
        lit_type = Lit_type_string;
        type = literal_t;
    }

    void literal(bool b) {
        b_value = b;
        lit_type = Lit_type_bool;
        type = literal_t;
    }

    void literal() {
        lit_type = Lit_type_null;
        type = literal_t;
    }

    Expr* rhs;
    Token op;
    Expr* lhs;

    double num_value;
    std::string s_value;
    bool b_value;

    Lit_type lit_type;
    Expr_Type type;
};

#define EXPR_FUNCTION_LITERAL(x) \
Expr t;                          \
t.literal(x);                    \
return t;

Expr Expr_new_unary(Token op, Expr* expr) {
    Expr t; t.unary(op, expr); return t;
}

Expr Expr_new_group(Expr* expr) {
    Expr t; t.grouping(expr); return t;
}

Expr Expr_new_literal(double num) {
    EXPR_FUNCTION_LITERAL(num);
}

Expr Expr_new_literal(std::string s) {
    EXPR_FUNCTION_LITERAL(s);
}

Expr Expr_new_literal(bool b) {
    EXPR_FUNCTION_LITERAL(b);
}

Expr Expr_new_literal() {
    Expr t; t.literal(); return t;
}

Expr Expr_new_binary(Expr* lhs, Token op, Expr* rhs) {
    Expr t;
    t.binary(lhs, op, rhs);

    return t;
}

enum Stmt_type {
    stmt_none_t,

    stmt_expression_t,
    stmt_print_t,
    stmt_println_t,
};

enum Stmt_has_value {
    stmt_val_none_t,

    stmt_has_val_t,
    stmt_has_no_val_t,
};

struct Stmt {
    Stmt() {
        expr = nullptr;
        type = stmt_none_t;
        val = stmt_val_none_t;
    }

    void Expression(Expr* v_expr) {
        expr = v_expr;
        type = stmt_expression_t;
    }

    void Print(Expr* v_expr) {
        expr = v_expr;
        type = stmt_print_t;
    }

    void PrintLine(Expr* v_expr) {
        expr = v_expr;
        val = stmt_has_val_t;
        type = stmt_println_t;
    }

    void PrintLine() {
        val = stmt_has_no_val_t;
        type = stmt_println_t;
    }


    Stmt_type type;
    Stmt_has_value val;
    Expr* expr;
};

Stmt Stmt_new_expression(Expr* val) {
    Stmt t; t.Expression(val); return t;
}

Stmt Stmt_new_print(Expr* val) {
    Stmt t; t.Print(val); return t;
}

Stmt Stmt_new_printline(Expr* val) {
    Stmt t; t.PrintLine(val); return t;
}

Stmt Stmt_new_printline() {
    Stmt t; t.PrintLine(); return t;
}

