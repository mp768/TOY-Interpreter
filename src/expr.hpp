#pragma once 
#include <iostream> 
#include "token.hpp" 

enum Expr_Type {
    none_t,

    binary_t,
    group_t,
    literal_t,
    unary_t,
    variable_t,
    assign_t,
    logical_t,
    call_t,
};

enum Lit_type {
    Lit_type_none,

    Lit_type_var,

    Lit_type_number,
    Lit_type_string,
    Lit_type_bool,
    Lit_type_null,
    Lit_type_callee,
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
        type = none_t;
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

    void Variable(Token v_name) {
        name = v_name;

        lit_type = Lit_type_var;
        type = literal_t;
    }

    void Assign(Token v_name, Expr* value) {
        name = v_name;

        lhs = value;
        type = assign_t;
    }

    void Logical(Expr* v_lhs, Token v_op, Expr* v_rhs) {
        lhs = v_lhs;
        op = v_op;
        rhs = v_rhs;

        type = logical_t;
    }

    void Call(Expr* callee, Token paren, std::vector<Expr*> list_arguments) {
        lhs = callee;
        name = paren;
        arguments = list_arguments;

        type = call_t;
        lit_type = Lit_type_callee;
    }

    std::vector<Expr*> arguments;

    Expr* rhs;
    Token op;
    Expr* lhs;

    Token name;

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

Expr Expr_new_variable(Token name) {
    Expr t; t.Variable(name); return t;
}

Expr Expr_new_assign(Token name, Expr* v) {
    Expr t; t.Assign(name, v); return t;
}

Expr Expr_new_logical(Expr* lhs, Token op, Expr* rhs) {
    Expr t; t.Logical(lhs, op, rhs); return t;
}

Expr Expr_new_Call(Expr* callee, Token paren, std::vector<Expr*> arguments) {
    Expr t; t.Call(callee, paren, arguments); return t;
}

enum Stmt_type {
    stmt_none_t,

    stmt_expression_t,
    stmt_print_t,
    stmt_println_t,
    stmt_var_t,
    stmt_block_t,
    stmt_if_t,
    stmt_if_else_t,
    stmt_while_t,
    stmt_break_t,
    stmt_function_t,
    stmt_return_t,
};

enum Stmt_has_value {
    val_none_t,

    has_val_t,
    has_no_val_t,
};

struct Stmt {
    Stmt() {
        expr = nullptr;
        type = stmt_none_t;
        val = val_none_t;
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
        val = has_val_t;
        type = stmt_println_t;
    }

    void PrintLine() {
        val = has_no_val_t;
        type = stmt_println_t;
    }

    void Var(Token v_name, Expr* value) {
        name = v_name;
        expr = value;

        type = stmt_var_t;
        val = has_val_t;
    }

    void Var(Token v_name) {
        name = v_name;
        expr = nullptr;

        type = stmt_var_t;
        val = has_no_val_t;
    }

    void Block(std::vector<Stmt> stmts) {
        block_stmts = stmts;

        type = stmt_block_t;
    }

    void If_Else(Expr* ex, Stmt t_branch, Stmt e_branch) {
        expr = ex;
        branches.push_back(t_branch);
        branches.push_back(e_branch);

        type = stmt_if_else_t;
    }

    void If(Expr* ex, Stmt t_branch) {
        expr = ex;
        branches.push_back(t_branch);

        type = stmt_if_t;
    }

    void While(Expr* condition, Stmt body) {
        expr = condition;
        branches.push_back(body);

        type = stmt_while_t;
    }

    void Break() {
        type = stmt_break_t;
    }

    void Function(Token v_name, std::vector<Token> v_params, std::vector<Stmt> stmts) {
        name = v_name;
        params = v_params;
        block_stmts = stmts;

        type = stmt_function_t;
    }

    void Return(Token keyword, Expr* value) {
        name = keyword;
        expr = value;

        type = stmt_return_t;
    }   

    std::vector<Stmt> branches;

    std::vector<Stmt> block_stmts;
    std::vector<Token> params;
    Token name;

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

Stmt Stmt_new_Var(Token name, Expr* value) {
    Stmt t; t.Var(name, value); return t;
}

Stmt Stmt_new_Var(Token name) {
    Stmt t; t.Var(name); return t;
}

Stmt Stmt_new_Block(std::vector<Stmt> stmts) {
    Stmt t; t.Block(stmts); return t;
}

Stmt Stmt_new_If_Else(Expr* expr, Stmt then_branch, Stmt else_branch) {
    Stmt t; t.If_Else(expr, then_branch, else_branch); return t;
}

Stmt Stmt_new_If(Expr* expr, Stmt then_branch) {
    Stmt t; t.If(expr, then_branch); return t;
}

Stmt Stmt_new_While(Expr* condition, Stmt body) {
    Stmt t; t.While(condition, body); return t;
}

Stmt Stmt_new_Break() {
    Stmt t; t.Break(); return t;
}

Stmt Stmt_new_Function(Token name, std::vector<Token> params, std::vector<Stmt> body) {
    Stmt t; t.Function(name, params, body); return t;
}

Stmt Stmt_new_Return(Token keyword, Expr* value) {
    Stmt t; t.Return(keyword, value); return t;
}