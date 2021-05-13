#pragma once
#include <vector>
#include "expr.hpp"
#include "scanner.hpp"
#include "token.hpp"

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
                return;
        }
    }
}

std::vector<Expr> parser_exprs;
#define PARSER_GET_FMAX(x) parser_exprs[parser_exprs.size() + x]
#define CURRENT_EXPR PARSER_GET_FMAX(-1)
#define PREVIOUS_EXPR PARSER_GET_FMAX(-2)

Expr Parser_primary(Parser& parser);

Expr Parser_unary(Parser& parser) {
    if (Parser_match(parser, { Not, Minus })) {
        Token op = Parser_previous(parser);
        Expr right = Parser_unary(parser);

        parser_exprs.push_back(right);

        return Expr_new_unary(op, &PARSER_GET_FMAX(-1));
    }

    return Parser_primary(parser);
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

Expr Parser_expression(Parser& parser) {
    parser_exprs.push_back(Parser_equality(parser));
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

    if (Parser_match(parser, { LeftParen })) {
        Expr expr = Parser_expression(parser);

        Parser_consume(parser, RightParen, "Expected ')' after expression.");
        parser_exprs.push_back(expr);

        return Expr_new_group(&PARSER_GET_FMAX(-1));
    }

    throw parse_error(Parser_peek(parser), " Expected an expression.");
} 

void Parser_parse(Parser& parser) {
    parser_exprs.resize(UINT16_MAX);
    try {
        Parser_expression(parser);
    } catch (ParseError p) {
        return;
    }
}