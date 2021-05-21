#pragma once
#include <iostream>

#include "expr.hpp"

//int __IDENT = 0;
int __INTERATION = 0;

#define PRINT_LOOP()                  \
for (auto i = 0; i < __IDENT; i++) {  \
    std::cout << " ";                 \
};

#define INCREASE_INTERATION __INTERATION++
#define DECREASE_INTERATION __INTERATION--

#define PRINT_OP_TOKEN(op)                                                \
IDENT_INCREASE(2);                                                        \
PRINT_LOOP();                                                             \
std::cout << "op: " << Token_type_to_string(op) << std::endl;             \
IDENT_INCREASE(-2);                 

#define PRINT(x) \
std::cout << x

#define PRINT_LINE(x) \
PRINT(x) << std::endl;

#define PRINT_LINE_BREAK(x) \
PRINT_LINE(x);              \
break;

#define IDENT_INCREASE(x) __IDENT += x;

void ast_print_expr(Expr* expr) {
    switch(expr->type) {
        case literal_t: 
            switch(expr->lit_type) {
                case Lit_type_number:
                    PRINT(expr->num_value);
                    break;
                case Lit_type_string:
                    PRINT(expr->s_value);
                    break;
                case Lit_type_bool: 
                    PRINT((expr->b_value ? "true" : "false"));
                    break;
                case Lit_type_null:
                    PRINT("NULL");
                    break;
            };
            break;

        case unary_t:
            switch(expr->op.type) {
                case Minus:
                    std::cout << "-";
                    break;
                case Not:
                    std::cout << "!";
                    break;
            }
            INCREASE_INTERATION;
            ast_print_expr(expr->lhs);
            DECREASE_INTERATION;

            break;
        
        case binary_t:
            PRINT("(");
            PRINT(expr->op.lexeme);
            //switch (expr->op.type) {
            //    case Plus: 
            //        std::cout << "+";
            //        break;
            //    case Minus: 
            //        std::cout << "-";
            //        break;
            //    case Star: 
            //        std::cout << "*";
            //        break;
            //    case Slash:
            //        std::cout << "/";
            //        break;
            //}

            PRINT(" ");
            INCREASE_INTERATION;
            ast_print_expr(expr->lhs);
            DECREASE_INTERATION;
            PRINT(", ");

            INCREASE_INTERATION;
            ast_print_expr(expr->rhs);
            DECREASE_INTERATION;
            PRINT(")");

            break;

        case group_t:
            PRINT("( group: ");
            INCREASE_INTERATION;
            ast_print_expr(expr->lhs);
            DECREASE_INTERATION;
            PRINT(")");

            break;
    }

    if (__INTERATION == 0)
        PRINT_LINE("");
} 