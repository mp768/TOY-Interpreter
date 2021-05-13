#pragma once
#include <string>

#include "expr.hpp"
#include "scanner.hpp"
#include "parser.hpp"

#define EXPR_LIT(switch_stmt, arg1, arg2, arg3, arg4, doarg1, doarg2, doarg3, doarg4) \
switch(switch_stmt) {  case arg1: doarg1; break;  case arg2: doarg2; break;  case arg3: doarg3; break;  case arg4: doarg4; break;  default: had_error = true; break;  }                                                                                                           

#define match switch
#define c(x) case x
#define ret(x) return x

#define scenerio(case_type, stmt_, return_type) c(case_type): stmt_; ret(return_type); break;

struct Interpreter {
    Interpreter(Expr* v_expr) {
        i_expr = v_expr;
    }

    void evaluate(Expr* expr) {

        auto type = loop_evaluate(expr);

        #ifdef DEBUG_PRINT
            switch(type) {
                case Lit_type_number:
                    std::cout << "Number" << std::endl;
                    break;

                case Lit_type_string:
                    std::cout << "String" << std::endl;
                    break;

                case Lit_type_bool:
                    std::cout << "Bool" << std::endl;
                    break;

                case Lit_type_none:
                    std::cout << "None" << std::endl;
                    break;
            }
        #endif

        evaluated_lit_type = type;

        /* 
            Clearly don't know how to setup comparison evaluation, simply because it won't run the program because of it.
            Will fix it way later
        */

        //if(evaluate_comparison_expr(expr))
        //    return;

        switch(type) {
            case Lit_type_number:
                evaluated_number = evaluate_expr_number(expr);
                break;

            case Lit_type_string:
                evaluated_string = evaluate_expr_string(expr);
                break;

            case Lit_type_bool:
                evaluated_bool = evaluate_expr_bool(expr);
                break;
        }
    }

    bool evaluate_comparison_expr(Expr* expr) {
        double b1;
        double b2;

        evaluate(expr->lhs);

        switch(evaluated_lit_type) {
            case Lit_type_number:
                b1 = evaluated_number;
                break;

            case Lit_type_bool:
                b1 = evaluated_bool;
                break;

            //case Lit_type_string:
            //    b1 = *evaluated_string.c_str();
            //    break;
        }

        evaluate(expr->rhs);

        switch(evaluated_lit_type) {
            case Lit_type_number:
                b2 = evaluated_number;
                break;

            case Lit_type_bool:
                b2 = evaluated_bool;
                break;

            //case Lit_type_string:
            //    b2 = *evaluated_string.c_str();
            //    break;
        }

        evaluated_lit_type = Lit_type_bool;

        match(expr->op.type) {

            scenerio(Greater, evaluated_bool = b1 > b2, true);
            scenerio(GreaterEqual, evaluated_bool = b1 >= b2, true);
            scenerio(Less, evaluated_bool = b1 < b2, true);
            scenerio(LessEqual, evaluated_bool = b1 <= b2, true);
            scenerio(EqualEqual, evaluated_bool = b1 == b2, true);
            scenerio(NotEqual, evaluated_bool = b1 != b2, true);

            //case GreaterEqual:
            //    evaluated_bool = b1 >= b2;
            //    return true;
            //
            //case Less:
            //    evaluated_bool = b1 < b2;
            //    return true;
//
            //case LessEqual:
            //    evaluated_bool = b1 <= b2;
            //    return true;
//
            //case EqualEqual:
            //    evaluated_bool = b1 == b2;
            //    return true;
//
            //case NotEqual:
            //    evaluated_bool = b1 != b2;
            //    return true;

            default:
                return false;
        }
    }

    Lit_type loop_evaluate(Expr* expr) {
        switch(expr->type) {
            case literal_t:
                switch(expr->lit_type) {
                    case Lit_type_number:
                        return Lit_type_number;
                        break;
                        
                    case Lit_type_string:
                        return Lit_type_string;
                        break;

                    case Lit_type_bool:
                        return Lit_type_bool;
                        break;
                }
                break;

            case unary_t:
            case binary_t:
            case group_t:
                return loop_evaluate(expr->lhs);
                break;
        }

        return Lit_type_none;
    } 

    bool evaluate_expr_bool(Expr* expr) {
        bool b1;
        bool b2;

        switch(expr->type) {
            case literal_t:
                switch(expr->lit_type) {
                    case Lit_type_string:
                        had_error = true;
                        report(expr->op.line, " at number binary operations", "cannot add string to bool (but can add boolean to string or number).");
                        break;

                    case Lit_type_number:
                        return (bool)expr->num_value;
                }
                return expr->b_value;
                break;

            case binary_t:
                b1 = evaluate_expr_bool(expr->lhs);
                b2 = evaluate_expr_bool(expr->rhs);

                switch(expr->op.type) {
                    default:
                        had_error = true;
                        report(expr->op.line, " with evaluating bool binary operator type", "operator type is not apart of the current list of usable operators ");
                        break;
                }
                break;

            case group_t:
                return evaluate_expr_bool(expr->lhs);
                break;

            case unary_t:
                b1 = evaluate_expr_bool(expr->lhs);
                switch(expr->op.type) {
                    case Not:
                        return !b1;
                        break;

                    default:
                        had_error = true;
                        report(expr->op.line, " with evaluating bool unary operator type", "operator type is not apart of the current list of usable operators (ie. !)");
                        break;
                }
                break;
        }

        return false;
    }

    std::string evaluate_expr_string(Expr* expr) {
        std::string str1;
        std::string str2;

        switch(expr->type) {
            case literal_t:
                switch(expr->lit_type) {
                    case Lit_type_number:
                        return std::to_string(expr->num_value);
                        break;

                    case Lit_type_bool:
                        return std::to_string(expr->b_value);
                        break;
                }

                return expr->s_value;
                break;

            case binary_t:
                str1 = evaluate_expr_string(expr->lhs);
                str2 = evaluate_expr_string(expr->rhs);

                switch(expr->op.type) {
                    case Plus:
                        return str1 + str2;
                        break;
                        
                    default:
                        had_error = true;
                        report(expr->op.line, " with evaluating string binary operator type", "operator type is not apart of the current list of usable operators (ie. +)");
                        break;
                }
                break;

            case group_t:
                return evaluate_expr_string(expr->lhs);
                break;

            case unary_t:
                str1 = evaluate_expr_string(expr->lhs);
                switch(expr->op.type) {
                    default:
                        had_error = true;
                        report(expr->op.line, " with evaluating string unary operator type", "operator type is not apart of the current list of usable operators");
                        break;
                }
                break;
        }

        return "";
    }

    double evaluate_expr_number(Expr* expr) {
        double num1;
        double num2;

        switch(expr->type) {
            case literal_t:
                switch(expr->lit_type) {
                    case Lit_type_string:
                        had_error = true;
                        report(expr->op.line, " at number binary operations", "cannot add string to number (but can add number to string or boolean).");
                        break;

                    case Lit_type_bool:
                        return expr->b_value;
                }
                return expr->num_value;
                break;

            case binary_t:
                num1 = evaluate_expr_number(expr->lhs);
                num2 = evaluate_expr_number(expr->rhs);

                switch(expr->op.type) {
                    case Star:
                        return num1 * num2;
                        break;

                    case Slash:
                        return num1 / num2;
                        break;

                    case Plus:
                        return num1 + num2;
                        break;

                    case Minus:
                        return num1 - num2;
                        break;
                        
                    default:
                        had_error = true;
                        report(expr->op.line, " with evaluating number binary operator type", "operator type is not apart of the current list of usable operators (ie. +, -, *, /)");
                        break;
                }
                break;

            case group_t:
                return evaluate_expr_number(expr->lhs);
                break;

            case unary_t:
                num1 = evaluate_expr_number(expr->lhs);
                switch(expr->op.type) {
                    case Minus:
                        return -num1;
                        break;
                        
                    default:
                        had_error = true;
                        report(expr->op.line, " with evaluating number unary operator type", "operator type is not apart of the current list of usable operators (ie. -)");
                        break;
                }
                break;
        }

        return 0;
    }

    double evaluated_number;
    std::string evaluated_string;
    bool evaluated_bool;
    bool null_active = false;

    Lit_type evaluated_lit_type;

    Expr* i_expr = nullptr;
};
