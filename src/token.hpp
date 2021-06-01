#pragma once
#include <string>
#include <sstream>
#include <map>
#undef EOF

enum Token_type {
    // used to mark an incomplete token_type
    NONE,
    
    // Keywords
    Print, PrintLn, Var, True, False, Nil, If, Else, And, Or, Switch,
    While, For, Break, Fun, Return,

    LeftParen, RightParen, LeftBrace, RightBrace,
    Comma, Dot, Minus, Plus, Star, Slash, Semicolon,

    Colon, ColonEqual,
    Equal, EqualEqual,
    Not, NotEqual,
    Greater, GreaterEqual,
    Less, LessEqual,


    // Literal
    String, Number, Identifer,

    // Marks the end of the file
    EOF,
};

std::string Token_type_to_string(Token_type type) {
    switch (type) {
        case NONE:         return "None";

        case Print:        return "Print";
        case PrintLn:      return "PrintLine";
        case Var:          return "Var";
        case If:           return "If";
        case Else:         return "Else";
        case Or:           return "Or";
        case And:          return "And";
        case Switch:       return "Switch";
        case For:          return "For";
        case While:        return "While";
        case Break:        return "Break";
        case Fun:          return "Fun";
        case Return:       return "Return";

        case LeftParen:    return "Left Paren";
        case RightParen:   return "Right Paren";
        case LeftBrace:    return "Left Brace";
        case RightBrace:   return "Right Brace";
        case Dot:          return "Dot";
        case Comma:        return "Comma";

        case True:         return "True";
        case False:        return "False";
        case Nil:          return "Nil";

        case Minus:        return "Minus";
        case Plus:         return "Plus";
        case Star:         return "Star";

        case Colon:        return "Colon"; // used to assign types. 
        case ColonEqual:   return "ColonEqual"; // used to infer types. 
        case Equal:        return "Equal";
        case EqualEqual:   return "EqualEqual";
        case Not:          return "Not";
        case NotEqual:     return "NotEqual";
        case Greater:      return "Greater";
        case GreaterEqual: return "GreaterEqual";
        case Less:         return "Less";
        case LessEqual:    return "LessEqual";

        case Slash:        return "Slash";

        case String:       return "String";
        case Number:       return "Number";
        case Identifer:    return "Identifer";
        
        case Semicolon:    return "Semicolon";

        case EOF:          return "EOF";        

        default:           return "NO TOKEN";
    }
};

std::map<std::string, Token_type> Token_type_keywords = {
    { "print", Print },
    { "println", PrintLn },
    { "var", Var },
    { "true", True },
    { "false", False },
    { "nil", Nil },
    { "if", If },
    { "else", Else },
    { "or", Or },
    { "and", And },
    { "switch", Switch },
    { "for", For },
    { "while", While },
    { "break", Break },
    { "func", Fun },
    { "return", Return },
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

struct String_unwrap {
    std::string value;
    bool success;
};

struct Number_unwrap {
    double value;
    bool success;
};

struct Bool_unwrap {
    bool value;
    bool success;
};

Number_unwrap Token_num_value(Token token) {
    if (token.type == Number) return { std::stod(token.literal), true };
    else                      return { -1, false };
}

String_unwrap Token_string_value(Token token) {
    if (token.type == String) return { token.literal, true };
    else                      return { "NO VALUE", false };
}

Bool_unwrap Token_bool_value(Token token) {
    if (token.type == True || token.type == False) {
        bool b;
        std::istringstream(token.literal) >> std::boolalpha >> b;
        return { b, true };
    }

    return { false, false };
}

std::string Token_to_string(Token token) {
    std::string text = "type: " + Token_type_to_string(token.type) + "    Lexeme: " + token.lexeme + "    Literal Value: ";
    if (token.literal == "") {
        text = text + "NO LITERAL VALUE";
    } else {
        text = text + token.literal;
    }

    text = text + "    Line: " + std::to_string(token.line);
    return text;
}

