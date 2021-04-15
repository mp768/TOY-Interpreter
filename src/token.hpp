#pragma once
#include <string>
#undef EOF

enum Token_type {
    // used to mark an incomplete token_type
    NONE,
    
    // Keywords
    Print, 

    // Literal
    String, Number, Identifer,

    // Marks the end of the file
    EOF,
};

std::string Token_type_to_string(Token_type type) {
    switch (type) {
        case NONE:       return "None";

        case Print:      return "Print";

        case String:     return "String";
        case Number:     return "Number";
        case Identifer:  return "Identifer";

        case EOF:        return "EOF";        

        default:         return "NO TOKEN";
    }
};

struct Token {
    Token_type type = NONE;
    std::string lexeme = "";
    std::string literal = "";
    int line = 0;
};

Token Token_new(Token_type type, std::string lexeme, std::string literal, int line) {
    return Token { type, lexeme, literal, line };
}

double Token_num_value(Token token) {
    if (token.type == Number) return std::stod(token.literal);
    else                      return -1;
}

std::string Token_string_value(Token token) {
    if (token.type == String) return token.literal;
    else                      return "NO VALUE";
}

std::string Token_to_string(Token token) {
    return "type: " + Token_type_to_string(token.type) + "    Lexeme: " + token.lexeme + "    Literal Value: " + token.literal + "    Line: " + std::to_string(token.line);
}

