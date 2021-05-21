#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "token.hpp"
#include "utils.hpp"

struct Scanner {
    std::string source = "";
    std::vector<Token> tokens;

    int current = 0;
    int start = 0;
    int line = 0;
};

bool had_error = false;

void report(int line, std::string where, std::string message) {
    std::cout << " [line: " << line << "] Error" << where << ": " << message << std::endl;
    had_error = true;
}

void error(int line, std::string message) {
    report(line, "", message);
}

void report(std::string where, std::string message)
{
    std::cout << " Error" << where << ": " << message << std::endl;
    had_error = true;
}

void error(std::string message)
{
    report("", message);
}

Scanner Scanner_new(std::string input) { return Scanner { input }; }

bool Scanner_is_at_end(const Scanner& scanner) {
    return scanner.current >= scanner.source.length();
}

char Scanner_advance(Scanner& scanner) {
    return scanner.source.at(scanner.current++);
}

bool Scanner_match(Scanner& scanner, char expected) {
    if (Scanner_is_at_end(scanner))                     return false;
    if (scanner.source.at(scanner.current) != expected) return false;

    scanner.current++;
    return true;
}

char Scanner_peek(Scanner& scanner) {
    if (Scanner_is_at_end(scanner)) return '\0';
    return scanner.source.at(scanner.current);
}

char Scanner_peek_next(Scanner& scanner) {
    if (scanner.current + 1 >= scanner.source.length()) return '\0';
    return scanner.source.at(scanner.current + 1);
}

std::string Scanner_get_inbetween(Scanner& scanner, int p1, int p2) {
    return utils_get_inbetween(scanner.source, (scanner.start + p1), (scanner.current + p2));
}

void Scanner_add_token(Scanner& scanner, Token_type type, std::string literal) {
    std::string text = Scanner_get_inbetween(scanner, 0, 0);
    scanner.tokens.push_back(Token_new(type, text, literal, scanner.line));
}

void Scanner_add_token(Scanner& scanner, Token_type type) {
    Scanner_add_token(scanner, type, "");
}

void Scanner_number(Scanner& scanner) {
    while (utils_is_digit(Scanner_peek(scanner))) Scanner_advance(scanner);

    if (Scanner_peek(scanner) == '.' && utils_is_digit(Scanner_peek_next(scanner))) {
        Scanner_advance(scanner);

        while (utils_is_digit(Scanner_peek(scanner))) Scanner_advance(scanner);
    }

    Scanner_add_token(scanner, Number, Scanner_get_inbetween(scanner, 0, 0));
}

void Scanner_string(Scanner& scanner) {
    std::vector<char> str_buf;
    while (Scanner_peek(scanner) != '"' && !Scanner_is_at_end(scanner)) {

        if (Scanner_peek(scanner) == '\\') {
            switch(Scanner_peek_next(scanner)) {
                case '0':
                    str_buf.push_back('\0');
                    break;
                case 'n':
                    str_buf.push_back('\n');
                    break;
                case 't':
                    str_buf.push_back('\t');
                    break;
                case 'r':
                    str_buf.push_back('\r');
                    break;
                case '\\':
                    str_buf.push_back('\\');
                    break;
                case '"':
                    str_buf.push_back('\"');
                    break;
                case '\'':
                    str_buf.push_back('\'');
                    break;

                default:
                    error(scanner.line, "Invalid Char Token!");
            };
            Scanner_advance(scanner);
        } else {
            str_buf.push_back(Scanner_peek(scanner));
        }

        if (Scanner_peek(scanner) == '\n') scanner.line++;

        Scanner_advance(scanner);
    }

    if (Scanner_is_at_end(scanner)) {
        error(scanner.line, "unterminated string. ");
        return;
    }

    Scanner_advance(scanner);

    std::string final_value = "";

    for (auto c : str_buf) {
        final_value += c;
    };

    //final_value += Scanner_get_inbetween(scanner, 1, -1);

    Scanner_add_token(scanner, String, final_value);
}

void Scanner_identifer(Scanner& scanner) {
    while (utils_is_alphanumeric(Scanner_peek(scanner))) Scanner_advance(scanner);

    std::string text = Scanner_get_inbetween(scanner, 0, 0);
    Token_type type = Token_type_keywords[text];
    if (type == NONE) type = Identifer;

    Scanner_add_token(scanner, type);
}

void Scanner_scan_token(Scanner& scanner) {
    char c = Scanner_advance(scanner);

    switch(c) {
        case '(': Scanner_add_token(scanner, LeftParen); break;
        case ')': Scanner_add_token(scanner, RightParen); break;
        case '{': Scanner_add_token(scanner, LeftBrace); break;
        case '}': Scanner_add_token(scanner, RightBrace); break;
        case '.': Scanner_add_token(scanner, Dot); break;
        case ',': Scanner_add_token(scanner, Comma); break;

        case '-': Scanner_add_token(scanner, Minus); break;
        case '+': Scanner_add_token(scanner, Plus); break;
        case '*': Scanner_add_token(scanner, Star); break;

        case ';': Scanner_add_token(scanner, Semicolon); break;
        case '=': Scanner_add_token(scanner, Scanner_match(scanner, '=') ? EqualEqual : Equal); break;
        case '!': Scanner_add_token(scanner, Scanner_match(scanner, '=') ? NotEqual : Not); break;
        case '>': Scanner_add_token(scanner, Scanner_match(scanner, '=') ? GreaterEqual : Greater); break;
        case '<': Scanner_add_token(scanner, Scanner_match(scanner, '=') ? LessEqual : Less); break;
        case ':': Scanner_add_token(scanner, Scanner_match(scanner, '=') ? ColonEqual : Colon); break;

        case '/': 
            if (Scanner_match(scanner, '/')) {
                while (Scanner_peek(scanner) != '\n' && !Scanner_is_at_end(scanner)) Scanner_advance(scanner);
            } else {
                Scanner_add_token(scanner, Slash);
            }
            break;

        case '"': Scanner_string(scanner); break;

        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            scanner.line++;
            break;
        
        default:
            if (utils_is_digit(c)) {
                Scanner_number(scanner);
            } else if (utils_is_alpha(c)) {
                Scanner_identifer(scanner);
            } else {
                error(scanner.line, "Unexpected Character. ");
            }
    }
}

std::vector<Token> Scanner_scan_tokens(Scanner& scanner) {
    while (!Scanner_is_at_end(scanner)) {
        scanner.start = scanner.current;
        Scanner_scan_token(scanner);
    }

    scanner.tokens.push_back(Token_new(EOF, "", "", scanner.line));
    return scanner.tokens;
}