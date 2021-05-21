#pragma once
#include <string>
#include <optional>
#include <sstream>
#include <iomanip>

#include "expr.hpp"
#include "scanner.hpp"
#include "parser.hpp"

#define EXPR_LIT(switch_stmt, arg1, arg2, arg3, arg4, doarg1, doarg2, doarg3, doarg4) \
switch(switch_stmt) {  case arg1: doarg1; break;  case arg2: doarg2; break;  case arg3: doarg3; break;  case arg4: doarg4; break;  default: had_error = true; break;  }                                                                                                           

#define match switch
#define c(x) case x
#define ret(x) return x

#define scenerio_r(case_type, stmts_, return_type) c(case_type): stmts_; ret(return_type); break;
#define scenerio_s(case_type, stmts_) c(case_type): stmts_; break;
#define match_s(match_a, stmts_) match(match_a) { stmts_ };
#define op_match(err, lit, stmt_) if (expr_lit_1 != lit || expr_lit_2 != lit) { had_error = true; error(err); return; } evaluated_lit_type = lit; stmt_;

#ifdef DEBUG_PRINT
    int __IDENT = 0;
#endif

struct Interpreter {
    Interpreter() {
    }   

    void evaluate_stmts(std::vector<Stmt> stmts) {
        try {
            for (auto stmt : stmts) {
                evaluate_stmt(stmt);
            }
        } catch (std::exception e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void evaluate_stmt(Stmt stmt) {
        switch(stmt.type) {
            case stmt_print_t:
                evaluate_expr(stmt.expr);
                switch(evaluated_lit_type) {
                    case Lit_type_bool:
                        std::cout << std::boolalpha << evaluated_bool.value();
                        break;

                    case Lit_type_number:
                        std::cout << evaluated_number;
                        break;

                    case Lit_type_string:
                        std::cout << evaluated_string;
                        break;

                    default:
                        had_error = true;
                        error("Cannot use evaluated type to print!");
                        break;
                }

                break;

            case stmt_println_t:
                if (stmt.val == stmt_has_val_t) {
                    evaluate_expr(stmt.expr);
                    switch(evaluated_lit_type) {
                        case Lit_type_bool:
                            std::cout << std::boolalpha << evaluated_bool.value() << std::endl;
                            break;

                        case Lit_type_number:
                            std::cout << evaluated_number << std::endl;
                            break;

                        case Lit_type_string:
                            std::cout << evaluated_string << std::endl;
                            break;

                        default:
                            had_error = true;
                            error("Cannot use evaluated type to print!");
                            break;
                    }
                }
                else if (stmt.val == stmt_has_no_val_t) {
                    std::cout << std::endl;
                }
                break;

            case stmt_expression_t:
                evaluate_expr(stmt.expr);
                break;

            case stmt_none_t:
                break;

            default:
                had_error = true;
                error("Unknown statement type used!");
                break;
        }
    }

    void evaluate_expr(Expr* expr) {
        expr_lit_type = Lit_type_none;
        evaluated_lit_type = Lit_type_none;
        evaluated_bool.reset();


        expr_lit_type = loop_evaluate_type(expr);

        #ifdef DEBUG_PRINT
            std::cout << std::endl;
            ast_print_expr(expr);
        #endif

        #ifdef DEBUG_PRINT
            std::cout << std::endl;
            std::cout << "Original type:    " << Lit_type_to_string(expr_lit_type) << std::endl;
        #endif

        if(expr_lit_type == Lit_type_null)
            expr_lit_type = Lit_type_number;

        #ifdef DEBUG_PRINT
            std::cout << "Transformed type: " << Lit_type_to_string(expr_lit_type) << " (Only changed when null value!)" << std::endl;
        #endif

        #ifdef DEBUG_PRINT
            std::cout << std::endl;
        #endif

        //ast_print_expr(expr);
        evaluate_expr_loop(expr);

        #ifdef DEBUG_PRINT_INTERPRETER
            if (evaluated_lit_type == Lit_type_bool)
                std::cout << std::boolalpha << evaluated_bool.value() << std::endl;
    
            if (evaluated_lit_type == Lit_type_number)
                std::cout << evaluated_number << std::endl;
    
            if (evaluated_lit_type == Lit_type_string)
                std::cout << evaluated_string << std::endl;
        #endif
    }

    //private:
//
    //    Lit_type     expr_lit_1;
    //    Lit_type     expr_lit_2;
//
    //    double       expr_num_1;    
    //    double       expr_num_2;    
//
    //    std::string  expr_str_1;
    //    std::string  expr_str_2;
//
    //    bool         expr_b_1;
    //    bool         expr_b_2;
//
    //public:

    void evaluate_expr_loop(Expr* expr) {
        if (had_error)
            return;

        std::ostringstream oss;

        Lit_type expr_lit_1;
        Lit_type expr_lit_2;

        double expr_num_1;
        double expr_num_2;

        std::string expr_str_1;
        std::string expr_str_2;

        bool expr_b_1;
        bool expr_b_2;

        switch(expr->type) {
            case literal_t:
                switch(expr->lit_type) {
                    case Lit_type_number:
                        evaluated_lit_type = Lit_type_number;
                        evaluated_number = expr->num_value;
                        break;

                    case Lit_type_string:
                        evaluated_lit_type = Lit_type_string;
                        evaluated_string = expr->s_value;
                        break;

                    case Lit_type_bool:
                        evaluated_lit_type = Lit_type_bool;
                        evaluated_bool = expr->b_value;
                        break;

                    case Lit_type_null:
                        switch(expr_lit_type) {
                            case Lit_type_number:
                                evaluated_lit_type = Lit_type_number;
                                evaluated_number = 0;
                                break;

                            case Lit_type_string:
                                evaluated_lit_type = Lit_type_string;
                                evaluated_string = "";
                                break;

                            case Lit_type_bool:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = false;
                                break;
                                
                            default:
                                had_error = true;
                                // this error as like others should never happen unless implementation details change and / or forgetting to update type checks.
                                error("Null value passed but cannot be evaluated to a type.");
                                break;

                        };
                        break;

                    default:
                        had_error = true;
                        error("No type was evalulated that are of the usable types.");
                        break;
                };
                break;

            case group_t:
                evaluate_expr_loop(expr->lhs);
                break;

            case unary_t:
                switch(evaluated_lit_type) {
                    case Lit_type_number:
                        // Handled somewhere else for efficieny???????!?!?!??!

                        evaluated_lit_type = Lit_type_number;
                        evaluate_expr_loop(expr->lhs);
                        switch(expr->op.type) {
                            case Minus:
                                evaluated_number = -evaluated_number;
                                break;

                            default:
                                had_error = true;
                                error("Unexpected unary operation used on number!");
                                break;
                        }
                        break;

                    //case Lit_type_string:
                    //    evaluate_expr_loop(expr->lhs);
                    //    had_error = true;
                    //    error("Type string doesn't have any unary operations!");
                    //    break;

                    case Lit_type_bool:
                        evaluated_lit_type = Lit_type_bool;
                        evaluate_expr_loop(expr->lhs);
                        switch(expr->op.type) {
                            case Not:
                                evaluated_bool = !evaluated_bool.value();
                                break;

                            default:
                                had_error = true;
                                error("Unexpected unary operation used on bool!");
                                break;
                        }
                        break;

                }

            case binary_t:
                switch(expr_lit_type) {
                    case Lit_type_number:
                        #ifdef DEBUG_PRINT
                            __IDENT += 1;
                        #endif

                        evaluate_expr_loop(expr->lhs);

                        if (evaluated_lit_type != Lit_type_number && evaluated_lit_type != Lit_type_bool) {
                            had_error = true;
                            error("Type does not match expr type. (Numbers only)");
                            return;
                        }

                        switch(evaluated_lit_type) {
                            case Lit_type_number:
                                expr_lit_1 = Lit_type_number;
                                expr_num_1 = evaluated_number;
                                break;

                            case Lit_type_bool:
                                expr_lit_1 = Lit_type_bool;
                                if (evaluated_bool.has_value())
                                    expr_b_1 = evaluated_bool.value();
                                break;
                        };

                        #ifdef DEBUG_PRINT
                            for (auto i = 0; i < __IDENT * 4; i++) { std::cout << " "; }
                            std::cout << "Number 1:\t" << expr_num_1 << std::endl;

                            for (auto i = 0; i < __IDENT * 4; i++) { std::cout << " "; }
                            if (evaluated_bool.has_value())
                                std::cout << "Bool 1:  \t" << expr_b_1 << std::endl;
                        #endif

                        if (expr->type == unary_t && evaluated_lit_type == Lit_type_bool) {
                            evaluated_lit_type = Lit_type_bool;
                            switch(expr->op.type) {
                                case Not:
                                    evaluated_bool = !evaluated_bool.value();
                                    break;

                                default:
                                    had_error = true;
                                    error("Unexpected unary operation used on bool!");
                                    break;
                            }   
                            return;
                        } 

                        else if (expr->type == unary_t) {
                            evaluated_lit_type = Lit_type_number;
                            switch(expr->op.type) {
                                case Minus:
                                    evaluated_number = -evaluated_number;
                                    break;

                                default:
                                    had_error = true;
                                    error("Unexpected unary operation used on number!");
                                    break;
                            }   
                            return;
                        }

                        evaluate_expr_loop(expr->rhs);

                        if (evaluated_lit_type != Lit_type_number && evaluated_lit_type != Lit_type_bool) {
                            had_error = true;
                            error("Type does not match expr type. (Numbers only)");
                            return;
                        }

                        switch(evaluated_lit_type) {
                            case Lit_type_number:
                                expr_lit_2 = Lit_type_number;
                                expr_num_2 = evaluated_number;
                                break;

                            case Lit_type_bool:
                                expr_lit_2 = Lit_type_bool;
                                if (evaluated_bool.has_value())
                                    expr_b_2 = evaluated_bool.value();
                                break;
                        };

                        #ifdef DEBUG_PRINT
                            for (auto i = 0; i < __IDENT * 4; i++) { std::cout << " "; }
                            std::cout << "Number 2:\t" << expr_num_2 << std::endl;

                            for (auto i = 0; i < __IDENT * 4; i++) { std::cout << " "; }
                            if(evaluated_bool.has_value())
                                std::cout << "Bool 2:  \t" << expr_b_2 << std::endl;
                            std::cout << std::endl;

                            __IDENT -= 1;
                        #endif

                        switch(expr->op.type) {
                            case Plus:
                                op_match("Cannot add values that are not numbers!", Lit_type_number, evaluated_number = expr_num_1 + expr_num_2);
                                break;

                            case Minus:
                                op_match("Cannot subtract values that are not numbers!", Lit_type_number, evaluated_number = expr_num_1 - expr_num_2);
                                break;

                            case Star:
                                op_match("Cannot multiply values that are not numbers!", Lit_type_number, evaluated_number = expr_num_1 * expr_num_2);
                                break;

                            case Slash:
                                op_match("Cannot divide values that are not numbers!", Lit_type_number, evaluated_number = expr_num_1 / expr_num_2);
                                break;

                            case Less:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_num_1 < expr_num_2;
                                break;

                            case LessEqual:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_num_1 <= expr_num_2;
                                break;

                            case Greater:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = (expr_num_1 > expr_num_2);
                                break;

                            case GreaterEqual:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_num_1 >= expr_num_2;
                                break;

                            case NotEqual:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_num_1 != expr_num_2;
                                break;

                            case EqualEqual:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_num_1 == expr_num_2;
                                break;

                            default:
                                had_error = true;
                                error("Numbers cannot do that binary operation!");
                                break;
                        }


                        break;

                    case Lit_type_string:
                        evaluate_expr_loop(expr->lhs);

                        if (evaluated_lit_type != Lit_type_number && evaluated_lit_type != Lit_type_bool && evaluated_lit_type != Lit_type_string) {
                            had_error = true;
                            error("Type does not match expr type. (String Types only)");
                            return;
                        }

                        switch(evaluated_lit_type) {
                            case Lit_type_number:
                                expr_lit_1 = Lit_type_string;
                                oss << std::setprecision(16) << std::noshowpoint << evaluated_number;
                                expr_str_1 = oss.str();
                                break;

                            case Lit_type_bool:
                                expr_lit_1 = Lit_type_string;
                                if (evaluated_bool.has_value()) {
                                    if (evaluated_bool.value())
                                        expr_str_1 = "true";
                                    if (!evaluated_bool.value())
                                        expr_str_1 = "false";
                                }
                                break;

                            case Lit_type_string:
                                expr_lit_1 = Lit_type_string;
                                expr_str_1 = evaluated_string;
                        };

                        if (expr->type == unary_t && evaluated_lit_type == Lit_type_bool) {
                            evaluated_lit_type = Lit_type_bool;
                            switch(expr->op.type) {
                                case Not:
                                    evaluated_bool = !evaluated_bool.value();
                                    break;

                                default:
                                    had_error = true;
                                    error("Unexpected unary operation used on bool!");
                                    break;
                            }   
                            return;
                        } 

                        else if (expr->type == unary_t) {
                            had_error = true;
                            error("No unary operations can be applied to strings");
                            return;
                        }

                        evaluate_expr_loop(expr->rhs);

                        if (evaluated_lit_type != Lit_type_number && evaluated_lit_type != Lit_type_bool && evaluated_lit_type != Lit_type_string) {
                            had_error = true;
                            error("Type does not match expr type. (String Types only)");
                            return;
                        }

                        switch(evaluated_lit_type) {
                            case Lit_type_number:
                                expr_lit_2 = Lit_type_string;
                                oss << std::setprecision(16) << std::noshowpoint << evaluated_number;
                                expr_str_2 = oss.str();
                                break;

                            case Lit_type_bool:
                                expr_lit_2 = Lit_type_string;
                                if (evaluated_bool.has_value()) {
                                    if (evaluated_bool.value())
                                        expr_str_2 = "true";
                                    if (!evaluated_bool.value())
                                        expr_str_2 = "false";
                                }
                                break;

                            case Lit_type_string:
                                expr_lit_2 = Lit_type_string;
                                expr_str_2 = evaluated_string;
                        };

                        switch(expr->op.type) {
                            case Plus:
                                evaluated_string = expr_str_1 + expr_str_2;
                                evaluated_lit_type = Lit_type_string;
                                break;

                            case EqualEqual:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_str_1 == expr_str_2;
                                break;

                            case NotEqual:
                                evaluated_lit_type = Lit_type_bool;
                                evaluated_bool = expr_str_1 != expr_str_2;
                                break;

                            default:
                                had_error = true;
                                error("Strings cannot do that binary operation!");
                                break;
                        }

                        break;

                    case Lit_type_bool:
                        evaluate_expr_loop(expr->lhs);

                        if (evaluated_lit_type != Lit_type_bool) {
                            had_error = true;
                            error("Type does not match expr type. (Bools only)");
                            return;
                        }

                        expr_lit_1 = Lit_type_bool;
                        if (evaluated_bool.has_value())
                            expr_b_1 = evaluated_bool.value();

                        evaluate_expr_loop(expr->rhs);

                        if (evaluated_lit_type != Lit_type_bool) {
                            had_error = true;
                            error("Type does not match expr type. (Bools only)");
                            return;
                        }

                        if (expr->type == unary_t && evaluated_lit_type == Lit_type_bool) {
                            evaluated_lit_type = Lit_type_bool;
                            switch(expr->op.type) {
                                case Not:
                                    evaluated_bool = !evaluated_bool.value();
                                    break;

                                default:
                                    had_error = true;
                                    error("Unexpected unary operation used on bool!");
                                    break;
                            }   
                            return;
                        } 

                        expr_lit_2 = Lit_type_bool;
                        if (evaluated_bool.has_value())
                            expr_b_2 = evaluated_bool.value();

                        switch(expr->op.type) {
                            default:
                                had_error = true;
                                error("There is no binary operations for bools!");
                                break;
                        }


                        
                        break;
                }
                break;
        }

        return;
    }

    Lit_type loop_evaluate_type(Expr* expr) {
        switch(expr->type) {
            case literal_t:
                switch(expr->lit_type) {
                    case Lit_type_null:
                        return Lit_type_null;
                        break;

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
                return loop_evaluate_type(expr->lhs);
                break;
        }

        return Lit_type_none;
    } 

    double evaluated_number;
    std::string evaluated_string;
    std::optional<bool> evaluated_bool;
    bool null_active = false;

    Lit_type expr_lit_type;
    Lit_type evaluated_lit_type;
};
