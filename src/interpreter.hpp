#pragma once
#include <string>
#include <optional>
#include <sstream>
#include <iomanip>
#include <string>
#include <any>

#include "expr.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "environment.hpp"
//#include "ToyCallable.hpp"

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

Expr empty_expression = Expr_new_literal();

namespace toy {

struct ReturnException : std::runtime_error {
    ReturnException(std::any value) : val(value), runtime_error("") {}; 

    std::any val;
};

Expr number_literal = Expr_new_literal(static_cast<double>(6));

struct Interpreter;

std::any clock(Interpreter& interpreter, const std::vector<std::any>& arguments);

std::any toy_sqrt(Interpreter& interpreter, const std::vector<std::any>& arguments) {
    auto v1 = std::any_cast<double>(arguments[0]);

    return static_cast<double>(v1 * v1);
}

std::any toy_hello(Interpreter& interpreter, const std::vector<std::any>& arguments) {
    auto v1 = std::any_cast<std::string>(arguments[0]);
    auto v2 = std::any_cast<double>(arguments[1]);

    std::ostringstream o;

    o << "Hello, " << v1 << "! And you are " << v2 << " Years old!";
    //std::cout << o.str() << std::endl;

    return static_cast<std::string>(o.str());
}

struct Interpreter {
    Environment environment;
    Environment globals;

    Interpreter() {
        globals.define("clock", ToyCallable(1, &clock));
        globals.define("sqrt", ToyCallable(1, &toy_sqrt));
        globals.define("hello", ToyCallable(2, &toy_hello));

        environment = globals;
    }   


    void evaluate_stmts(std::vector<Stmt> stmts) {
        try {
            for (auto stmt : stmts) {
                evaluate_stmt(stmt);
            }
        } catch (std::exception e) {
            std::cout << e.what() << " at evaluating" << std::endl;
        }
    }

    bool block_while_loop_on = false;
    bool calling_function = false;

    void execute_block(std::vector<Stmt> stmts, Environment en) {
        //Environment previous = environment;

        if (while_loop_on)
            block_while_loop_on = true;



        try {
            environment = en;

            for (const auto& stmt : stmts) {
                if (block_while_loop_on) {
                    evaluate_stmt(stmt);
                    if (!while_loop_on) {
                        block_while_loop_on = false;

                        environment.values = environment.get_values();
                        environment.enclosing = nullptr;
                        return;
                    }   
                }
                else
                    evaluate_stmt(stmt);
            }

        } catch (ReturnException& e) {
            block_while_loop_on = false;

            environment.values = environment.get_values();
            environment.enclosing = nullptr;            
            return;
        }

        block_while_loop_on = false;

        environment.values = environment.get_values();
        environment.enclosing = nullptr;
    }

    bool while_loop_on = false;

    void evaluate_stmt(Stmt stmt) {
        bool b_value = false;
        ToyCallable func;
        std::any val;

        switch(stmt.type) {
            case stmt_return_t:
                if (calling_function) { 
                    calling_function = false;
                    val.reset();

                    if (stmt.expr != nullptr) {
                        evaluate_expr(stmt.expr);
                        switch (evaluated_lit_type) {
                            case Lit_type_number:
                                val = static_cast<double>(evaluated_number);
                                break;

                            case Lit_type_string:
                                val = static_cast<std::string>(evaluated_string);
                                break;

                            case Lit_type_bool:
                                val = static_cast<bool>(evaluated_bool);
                                break;
                        }
                    }

                    throw ReturnException(val);
                }

                break;

            case stmt_function_t:
                func = ToyCallable(stmt);
                environment.define(stmt.name.lexeme, func);
                break;

            case stmt_break_t:
                if (while_loop_on) {
                    while_loop_on = false;
                }
                else { 
                    error("Cannot break out of something that is not a loop!");
                    had_error = true;
                }
                break;

            case stmt_while_t:
                evaluate_expr(stmt.expr);
                if (evaluated_lit_type != Lit_type_bool) {
                    error("Type does not evaluate to a boolean.");
                    return;
                }

                while_loop_on = true;

                if (evaluated_bool.has_value())
                    b_value = evaluated_bool.value();

                while (evaluated_bool.value() && while_loop_on) {
                    if (evaluated_lit_type != Lit_type_bool) {
                        error("Type does not evaluate to a boolean.");
                        return;
                    }

                    if (!evaluated_bool.value())
                        break;


                    //if (!while_loop_on)
                    //    break;

                    //std::cout << "Debug: " << std::boolalpha << evaluated_bool.value() << std::endl;

                    evaluate_stmt(stmt.branches[0]);
                    evaluate_expr(stmt.expr);
                }

                while_loop_on = false;

                break;

            case stmt_if_t:
                evaluate_expr(stmt.expr);
                if (evaluated_lit_type != Lit_type_bool) {
                    error("Type does not evaluate to a boolean.");
                    return;
                }

                if (evaluated_bool.value()) {
                    evaluate_stmt(stmt.branches[0]);
                }

                break;

            case stmt_if_else_t:
                evaluate_expr(stmt.expr);
                if (evaluated_lit_type != Lit_type_bool) {
                    error("Type does not evaluate to a boolean.");
                    return;
                }

                if (evaluated_bool.value()) {
                    evaluate_stmt(stmt.branches[0]);
                } else {
                    evaluate_stmt(stmt.branches[1]);
                }
                break;

            case stmt_block_t:
                execute_block(stmt.block_stmts, Environment(environment));
                break;

            case stmt_var_t:
                if (stmt.val == has_no_val_t) 
                    environment.define(stmt.name.lexeme, &empty_expression);
                environment.define(stmt.name.lexeme, stmt.expr);
                break;
            
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

                    case Lit_type_null:
                        std::cout << "nil" << std::endl;
                        break;

                    default:
                        had_error = true;
                        error("Cannot use evaluated type to print!");
                        break;
                }

                break;

            case stmt_println_t:
                if (stmt.val == has_val_t) {
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

                        //case Lit_type_var:
                        //    std::cout << "VAR" << std::endl;
                        //    break;

                        case Lit_type_null:
                            std::cout << "nil" << std::endl;
                            break;

                        default:
                            had_error = true;
                            error("Cannot use evaluated type to print!");
                            break;
                    }
                }
                else if (stmt.val == has_no_val_t) {
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

    void evaluate_expr(const Expr* expr) {
        expr_lit_type = Lit_type_none;
        evaluated_lit_type = Lit_type_none;
        evaluated_bool.reset();

        if (expr->type == logical_t) {
            evaluate_expr(expr->lhs);

            if (evaluated_lit_type != Lit_type_bool) {
                error("Type does not evaluate to a boolean.");
                return;
            }

            switch(expr->op.type) {
                case Or:
                    if (evaluated_bool.value())
                        return;
                    break;

                case And:
                    if (!evaluated_bool.value())
                        return;
                    break;
            }

            evaluate_expr(expr->rhs);

            if (evaluated_lit_type != Lit_type_bool) {
                error("Type does not evaluate to a boolean.");
                return;
            }

            return;
        }


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

    bool callable_active = false;
    void evaluate_expr_loop(const Expr* expr) {
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


        if (expr->lit_type == Lit_type_var) {
            evaluated_lit_type = Lit_type_var;
            Expr* ex = environment.get(expr->name);

            evaluate_expr(ex);

            return;
        }

        if (expr->type == call_t) {
            calling_function = true;

            ToyCallable callee = environment.get_callable(expr->lhs->name);

            if (expr->arguments.size() != callee.arity()) {
                oss <<  "Expected " << callee.arity() << " arguments but got " << expr->arguments.size() << ".";
                error(expr->name.line /* Paren  */, oss.str());
            }

            std::vector<std::any> args;

            for (auto expr : expr->arguments) {
                evaluate_expr(expr);

                switch (evaluated_lit_type) {
                    case Lit_type_number:
                        args.push_back(evaluated_number);
                        break;

                    case Lit_type_string:
                        args.push_back(evaluated_string);
                        break;

                    case Lit_type_bool:
                        args.push_back(evaluated_bool);
                        break;

                    case Lit_type_null:
                        args.push_back(0);
                        break;

                    default:
                        error("Type cannot be evaluated!");
                        break;
                }
            }

            auto val = callee.call(*this, args); // only used for native functions

            if (val.has_value() && val.type() == typeid(NO_VALUE)) {
                const auto& params = callee.declaration.params;

                Environment block_env(environment);

                std::vector<Expr> exprs_;


                for (unsigned int i = 0u; i < params.size(); i++) {
                    for (auto arg : args) {
                        if (arg.type() == typeid(std::string))
                            exprs_.push_back(Expr_new_literal(std::any_cast<std::string>(arg)));

                        if (arg.type() == typeid(double))
                            exprs_.push_back(Expr_new_literal(std::any_cast<double>(arg)));

                        if (arg.type() == typeid(bool))
                            exprs_.push_back(Expr_new_literal(std::any_cast<bool>(arg)));
                    }

                    block_env.define(params.at(i).lexeme, &exprs_[exprs_.size() - 1]);
                    //arguments.push_back(Stmt_new_Var(params.at(i), expr->arguments.at(i)));
                }

                try {
                    environment = block_env;

                    for (auto stmt : callee.declaration.block_stmts) {
                        evaluate_stmt(stmt);
                    }
 
                } catch (ReturnException& e) {
                    environment.values = environment.get_values();
                    environment.enclosing = nullptr;

                    if (e.val.has_value()) {
                        if (e.val.type() == typeid(std::string)) {
                            evaluated_lit_type = Lit_type_string;
                            evaluated_string = std::any_cast<std::string>(e.val);
                            return;
                        }

                        if (e.val.type() == typeid(bool)) {
                            evaluated_lit_type = Lit_type_bool;
                            evaluated_bool = std::any_cast<bool>(e.val);
                            return;
                        }

                        if (e.val.type() == typeid(double) || e.val.type() == typeid(int)) {
                            evaluated_lit_type = Lit_type_number;
                            evaluated_number = std::any_cast<double>(e.val);
                            return;
                        }

                        error("Returned value is not usable");
                        return;

                    } else if (!e.val.has_value()) {
                        evaluated_lit_type = Lit_type_null;
                        return;
                    }
                }

                environment.values = environment.get_values();
                environment.enclosing = nullptr;

                return;
            }

            if (val.has_value()) {
                if (val.type() == typeid(std::string)) {
                    evaluated_lit_type = Lit_type_string;
                    evaluated_string = std::any_cast<std::string>(val);
                    return;
                }

                if (val.type() == typeid(bool)) {
                    evaluated_lit_type = Lit_type_bool;
                    evaluated_bool = std::any_cast<bool>(val);
                    return;
                }

                if (val.type() == typeid(double) || val.type() == typeid(int)) {
                    evaluated_lit_type = Lit_type_number;
                    evaluated_number = std::any_cast<double>(val);
                    return;
                }

                error("Returned function type not usable");
                return;
            }

            evaluated_lit_type = Lit_type_null;
        }


        if (expr->type == assign_t) {
            evaluate_expr(expr->lhs);
            switch(evaluated_lit_type) {
                case Lit_type_number:
                    parser_exprs.push_back(Expr_new_literal(evaluated_number));
                    break;

                case Lit_type_string:
                    parser_exprs.push_back(Expr_new_literal(evaluated_string));
                    break;

                case Lit_type_bool:
                    parser_exprs.push_back(Expr_new_literal(evaluated_bool.value()));
                    break;

                case Lit_type_null:
                    parser_exprs.push_back(Expr_new_literal());
                    break;

                default:
                    had_error = true;
                    error("Evaluated type cannot be used.");
                    break;
            }

            environment.assign(expr->name, &CURRENT_EXPR);
            return;
        }




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
                evaluate_expr(expr->lhs);
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

    Lit_type loop_evaluate_type(const Expr* expr) {
        Expr* expr_;

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

                    case Lit_type_var:
                        expr_ = environment.get(expr->name);
                        if (expr_ == nullptr)
                            return Lit_type_null;
                        return loop_evaluate_type(expr_);
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

std::any clock(Interpreter& interpreter, const std::vector<std::any>& arguments) {
    auto v1 = std::any_cast<double>(arguments[0]);

    return static_cast<double>(v1 + 6);
}

} // end of toy namespace
