#pragma once
#include <vector>
#include "expr.hpp"
#include "scanner.hpp"
#include "token.hpp"
#include "ast_printer.hpp"

Expr global_true_expr_literal = Expr_new_literal(true);
Expr global_false_expr_literal = Expr_new_literal(false);

struct Parser {
    std::vector<Token> tokens;
    int current;
};

Parser Parser_new(std::vector<Token> tokens) {
    return Parser {
        tokens, 
        0 // current
    };
}

Token Parser_peek(Parser& parser) {
    return parser.tokens.at(parser.current);
}

Token Parser_previous(Parser& parser) {
    return parser.tokens.at(parser.current - 1);
}

bool Parser_is_at_end(Parser& parser) {
    return Parser_peek(parser).type == EOF;
}

Token Parser_advance(Parser& parser) {
    if (!Parser_is_at_end(parser)) parser.current++;
    return Parser_previous(parser);
}

bool Parser_check(Parser& parser, Token_type type) {
    if (Parser_is_at_end(parser)) return false;
    return Parser_peek(parser).type == type;
}

bool Parser_match(Parser& parser, std::vector<Token_type> types) {
    for (auto type : types) {
        if (Parser_check(parser, type)) {
            Parser_advance(parser);
            return true;
        }
    }

    return false;
}

struct ParseError : public std::exception {};

void error(Token token, std::string message) {
    if (token.type == EOF) {
        report(token.line, " at end", message);
    } else {
        report(token.line, " at '" + token.lexeme + "'", message);
    }
}

ParseError parse_error(Token token, std::string message) {
    error(token, message);
    return ParseError();
}

Token Parser_consume(Parser& parser, Token_type type, std::string message) {
    if (Parser_check(parser, type)) return Parser_advance(parser);

    throw parse_error(Parser_peek(parser), message);
}

void Parser_synchronize(Parser& parser) {
    Parser_advance(parser);

    while (!Parser_is_at_end(parser)) {
        if (Parser_previous(parser).type == Semicolon) return;

        switch (Parser_peek(parser).type) {
            case Print:
            case PrintLn:
                return;
        }
        
        Parser_advance(parser);
    }
}

std::vector<Stmt> parser_stmts(500);
#define PARSER_GET_STMT(x) parser_stmts[parser_stmts.size() + x]
#define CURRENT_STMT PARSER_GET_STMT(-1);
#define PREVIOUS_STMT PARSER_GET_STMT(-2);

std::vector<Expr> parser_exprs;
#define PARSER_GET_FMAX(x) parser_exprs[parser_exprs.size() + x]
#define CURRENT_EXPR PARSER_GET_FMAX(-1)
#define PREVIOUS_EXPR PARSER_GET_FMAX(-2)

Expr Parser_primary(Parser& parser);
Expr Parser_expression(Parser& parser);

Expr Parser_finish_call(Parser& parser, Expr* callee) {
    std::vector<Expr*> arguments;

    if (!Parser_check(parser, RightParen)) {
        do {
            if (arguments.size() >= 255)
                error(Parser_peek(parser).line, "Can't have more than 255 arguments in a call.");

            Parser_expression(parser);
            arguments.push_back(&CURRENT_EXPR);
        } while (Parser_match(parser, { Comma }));
    }

    Token paren = Parser_consume(parser, RightParen, "Expect ')' after arguments.");

    return Expr_new_Call(callee, paren, arguments);
}

Expr Parser_call(Parser& parser) {
    Expr expr = Parser_primary(parser);

    while (true) {
        if (Parser_match(parser, { LeftParen })) {
            parser_exprs.push_back(expr);
            expr = Parser_finish_call(parser, &CURRENT_EXPR);
            parser_exprs.push_back(expr);
        }
        else    
            break;
    }

    return expr;
}

Expr Parser_unary(Parser& parser) {
    if (Parser_match(parser, { Not, Minus })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_unary(parser);

        parser_exprs.push_back(right);

        return Expr_new_unary(op, &PARSER_GET_FMAX(-1));
    }

    return Parser_call(parser);
}

Expr Parser_factor(Parser& parser) {
    Expr expr = Parser_unary(parser);

    while (Parser_match(parser, { Star, Slash })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_unary(parser);

        parser_exprs.push_back(right);
        parser_exprs.push_back(expr);

        expr = Expr_new_binary(&PARSER_GET_FMAX(-1), op, &PARSER_GET_FMAX(-2));
        parser_exprs.push_back(expr);
        #ifdef DEBUG_PRINT
            std::cout << "Binary found at factor()" << std::endl;
        #endif
    }

    return expr;
}

Expr Parser_term(Parser& parser) {
    Expr expr = Parser_factor(parser);

    while (Parser_match(parser, { Minus, Plus })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_factor(parser);

        parser_exprs.push_back(right);
        parser_exprs.push_back(expr);

        expr = Expr_new_binary(&PARSER_GET_FMAX(-1), op, &PARSER_GET_FMAX(-2));
        parser_exprs.push_back(expr);
        #ifdef DEBUG_PRINT
            std::cout << "Binary found at term()" << std::endl;
        #endif
    }

    return expr;
}

Expr Parser_comparison(Parser& parser) {
    Expr expr = Parser_term(parser);

    while (Parser_match(parser, { Greater, GreaterEqual, Less, LessEqual })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_term(parser);

        parser_exprs.push_back(right);
        parser_exprs.push_back(expr);

        expr = Expr_new_binary(&PARSER_GET_FMAX(-1), op, &PARSER_GET_FMAX(-2));
        parser_exprs.push_back(expr);
        #ifdef DEBUG_PRINT
            std::cout << "Binary found at comparsion()" << std::endl;
        #endif
    }

    return expr;
}

Expr Parser_equality(Parser& parser) {
    Expr expr = Parser_comparison(parser);

    while (Parser_match(parser, { NotEqual, EqualEqual })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_comparison(parser);

        parser_exprs.push_back(right);
        parser_exprs.push_back(expr);

        expr = Expr_new_binary(&PARSER_GET_FMAX(-1), op, &PARSER_GET_FMAX(-2));
        parser_exprs.push_back(expr);
        #ifdef DEBUG_PRINT
            std::cout << "Binary found at equality()" << std::endl;
        #endif
    }

    return expr;
}

Expr Parser_and(Parser& parser) {
    Expr expr = Parser_equality(parser);

    while (Parser_match(parser, { And })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_equality(parser);

        parser_exprs.push_back(right);
        parser_exprs.push_back(expr);

        expr = Expr_new_logical(&CURRENT_EXPR, op, &PREVIOUS_EXPR);
        parser_exprs.push_back(expr);
    }

    return expr;
}

Expr Parser_or(Parser& parser) {
    Expr expr = Parser_and(parser);

    while (Parser_match(parser, { Or })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_and(parser);

        parser_exprs.push_back(right);
        parser_exprs.push_back(expr);

        expr = Expr_new_logical(&CURRENT_EXPR, op, &PREVIOUS_EXPR);
        parser_exprs.push_back(expr);
    }

    return expr;
}

Expr Parser_assignment(Parser& parser) {
    Expr expr = Parser_or(parser);

    parser_exprs.push_back(expr);

    if (Parser_match(parser, { Equal })) {
        Token equals = Parser_previous(parser);
        Expr value = Parser_assignment(parser);

        parser_exprs.push_back(value);

        if (expr.lit_type = Lit_type_var) {
            Token name = expr.name;
            parser_exprs.push_back(Expr_new_assign(name, &CURRENT_EXPR));
            return CURRENT_EXPR;
        }

        report(parser.current, "=", "Invalid assignment target.");
    }

    return CURRENT_EXPR;
}

Expr Parser_expression(Parser& parser) {
    parser_exprs.push_back(Parser_assignment(parser));
    return PARSER_GET_FMAX(-1);
}

Expr Parser_primary(Parser& parser) {
    if (Parser_match(parser, { True })) return Expr_new_literal(true);
    if (Parser_match(parser, { False })) return Expr_new_literal(false);
    if (Parser_match(parser, { Nil })) return Expr_new_literal();

    if (Parser_match(parser, { Number, String })) {
        if (Parser_previous(parser).type == Number) {
            parser_exprs.push_back(Expr_new_literal((double)std::stod(Parser_previous(parser).literal)));
            return PARSER_GET_FMAX(-1);
        } 

        if(Parser_previous(parser).type == String) {
            parser_exprs.push_back(Expr_new_literal(Parser_previous(parser).literal));
            return PARSER_GET_FMAX(-1);
        }
    }

    if (Parser_match(parser, { Identifer })) 
        return Expr_new_variable(Parser_previous(parser));

    if (Parser_match(parser, { LeftParen })) {
        Expr expr = Parser_expression(parser);

        Parser_consume(parser, RightParen, "Expected ')' after expression.");
        parser_exprs.push_back(expr);

        return Expr_new_group(&PARSER_GET_FMAX(-1));
    }

    throw parse_error(Parser_peek(parser), " Expected an expression.");
} 




Stmt Parser_declaration(Parser& parser);
Stmt Parser_statement(Parser& parser);
Stmt Parser_var_declaration(Parser& parser);
std::vector<Stmt> Parser_block(Parser &parser);

    Stmt Parser_print_stmt(Parser &parser)
{
    Parser_expression(parser);
    Parser_consume(parser, Semicolon, "Expect ';' after value!");
    return Stmt_new_print(&CURRENT_EXPR);
}

Stmt Parser_printline_stmt(Parser& parser) {
    if (Parser_check(parser, Semicolon)) {
        Parser_advance(parser);
        return Stmt_new_printline();
    }

    Parser_expression(parser);
    Parser_consume(parser, Semicolon, "Expect ';' after value!");
    return Stmt_new_printline(&CURRENT_EXPR);
}

Stmt Parser_expr_stmt(Parser& parser) {
    Parser_expression(parser);
    Parser_consume(parser, Semicolon, "Expect ';' after value!");
    return Stmt_new_expression(&CURRENT_EXPR);
}

Stmt Parser_if_stmt(Parser& parser);

Stmt Parser_while_stmt(Parser& parser) {
    Parser_consume(parser, LeftParen, "Expect '(' after while.");

    Parser_expression(parser);
    Expr* condition = &CURRENT_EXPR;

    Parser_consume(parser, RightParen, "Expect ')' after condition.");

    Stmt body = Parser_declaration(parser);

    return Stmt_new_While(condition, body);
}

Stmt Parser_for_stmt(Parser& parser) {
    Parser_consume(parser, LeftParen, "Expecte '(' after 'for'.");

    Stmt init;
    if (Parser_match(parser, { Semicolon })) {
        // skip and do not assign init.
    } else if (Parser_match(parser, { Var })) {
        init = Parser_var_declaration(parser);
    } else {
        init = Parser_expr_stmt(parser);
    }

    Expr* condition = nullptr;
    if (!Parser_check(parser, Semicolon)) {
        Parser_expression(parser);
        condition = &CURRENT_EXPR;
    }

    Parser_consume(parser, Semicolon, "Expect ';' after for loop condition.");

    Expr* increment = nullptr;
    if (!Parser_check(parser, RightParen)) {
        Parser_expression(parser);
        increment = &CURRENT_EXPR;
    }

    Parser_consume(parser, RightParen, "Expect ')' after for clauses.");

    Stmt body = Parser_declaration(parser);

    if (increment != nullptr)
        body = Stmt_new_Block({ body, Stmt_new_expression(increment) });

    if (condition == nullptr) condition = &global_true_expr_literal;
    body = Stmt_new_While(condition, body);

    if (init.type != stmt_none_t)
        body = Stmt_new_Block({init, body});

    
    return body;
}

Stmt Parser_break(Parser& parser) {
    Parser_consume(parser, Semicolon, "Expect ';' after 'break'.");
    return Stmt_new_Break();
}

Stmt Parser_fun_stmt(Parser& parser, std::string kind) {
    Token name = Parser_consume(parser, Identifer, "Expect " + kind + " name.");

    Parser_consume(parser, LeftParen, "Expect '(' after " + kind + " name.");

    std::vector<Token> params;
    if (!Parser_check(parser, RightParen)) {
        do {
            if (params.size() >= 255) {
                error(Parser_peek(parser).line, "Can't have more than 255 parameters.");
                return Stmt();
            }

            params.push_back(Parser_consume(parser, Identifer, "Expect parameter name."));
        } while (Parser_match(parser, { Comma }));
    }

    Parser_consume(parser, RightParen, "Expect ')' after parameters.");

    Parser_consume(parser, LeftBrace, "Expect '{' before " + kind + " body.");
    std::vector<Stmt> body = Parser_block(parser);

    return Stmt_new_Function(name, params, body);
}

Stmt Parser_return_stmt(Parser& parser) {
    Token keyword = Parser_previous(parser);
    Expr* value = nullptr;
    if (!Parser_check(parser, Semicolon)) {
        Parser_expression(parser);
        value = &CURRENT_EXPR;
    }

    Parser_consume(parser, Semicolon, "Expect ';' after return value.");
    return Stmt_new_Return(keyword, value);
}

Stmt Parser_statement(Parser& parser) {
    if (Parser_match(parser, { Fun }))      return Parser_fun_stmt(parser, "function");
    if (Parser_match(parser, { For }))      return Parser_for_stmt(parser);
    if (Parser_match(parser, { If }))       return Parser_if_stmt(parser);
    if (Parser_match(parser, { Return }))   return Parser_return_stmt(parser);
    if (Parser_match(parser, { Print }))    return Parser_print_stmt(parser);
    if (Parser_match(parser, { While }))    return Parser_while_stmt(parser);
    if (Parser_match(parser, { PrintLn }))  return Parser_printline_stmt(parser);
    if (Parser_match(parser, { Break }))    return Parser_break(parser);

    return Parser_expr_stmt(parser);
}

Stmt Parser_if_stmt(Parser& parser) {
    Parser_consume(parser, LeftParen, "Expect '(' after if.");

    Parser_expression(parser);
    Expr* condition = &CURRENT_EXPR;

    Parser_consume(parser, RightParen, "Expect ')' after if condition.");

    Stmt then_branch = Parser_declaration(parser);

    if (Parser_match(parser, { Else }))
        return Stmt_new_If_Else(condition, then_branch, Parser_declaration(parser));

    return Stmt_new_If(condition, then_branch);
}

Stmt Parser_var_declaration(Parser& parser) {
    Token name = Parser_consume(parser, Identifer, "Expected variable name.");

    Expr initializer;
    if (Parser_match(parser, { Equal }))
        initializer = Parser_expression(parser);

    Parser_consume(parser, Semicolon, "Expected ';' after variable declaration");

    if (initializer.type == none_t) 
        return Stmt_new_Var(name);

    return Stmt_new_Var(name, &CURRENT_EXPR);
} 


std::vector<Stmt> Parser_block(Parser& parser) {
    std::vector<Stmt> statements;
    
    while (!Parser_check(parser, RightBrace) && !Parser_is_at_end(parser)) {
        statements.push_back(Parser_declaration(parser));
    }

    Parser_consume(parser, RightBrace, "Expect '}' after the block!");
    return statements;
}

Stmt Parser_declaration(Parser& parser) {
    try {
        if (Parser_match(parser, { Var })) return Parser_var_declaration(parser);
        if (Parser_match(parser, { LeftBrace })) return Stmt_new_Block(Parser_block(parser));

        return Parser_statement(parser);
    } catch (ParseError e) {
        Parser_synchronize(parser);
        return Stmt();
    }
}

void Parser_parse(Parser& parser) {
    try {
        while (!Parser_is_at_end(parser)) 
            parser_stmts.push_back(Parser_declaration(parser));
    } catch (ParseError p) {
        return;
    }
}