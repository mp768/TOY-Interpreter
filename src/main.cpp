#include <iostream>
#include <string>
#include <fstream>
#undef EOF

/*
    Use to activate debug printing for all interpreter operations such as the parser, 
    abstract syntax tree printer, and tokenizer.

    NOTE: MIGHT BE SEPERATED INTO SEPERATE DEFINITIONS FOR THE DIFFERENT PARTS OF THE INTERPRETER.
*/
#define DEBUG_PRINT

#include "token.hpp"
#include "scanner.hpp"
#include "parser.hpp"
#include "ast_printer.hpp"
#include "interpreter.hpp"

void run(const std::string& file) {
    Scanner scanner = Scanner_new(file);
    std::vector<Token> tokens = Scanner_scan_tokens(scanner);
    Parser parser = Parser_new(tokens);
    Parser_parse(parser);

    if (had_error) return;

    //ast_print_expr(&CURRENT_EXPR);

    Interpreter i(&CURRENT_EXPR);
    i.evaluate(&CURRENT_EXPR);

    if (had_error) return;
}

void run_file(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    std::string whole_file = "";

    while(std::getline(file, line)) {
        whole_file.append(line);
        whole_file.append("\n");
    }

    run(whole_file);
    if (had_error) return;
}

int main(int argc, char* argv[]) {
    if (argc == 1 || argc > 2) {
        printf("toy interpreter usage: toy {file}");
        return 0;
    } else if (argc == 2) {
        run_file(argv[1]);
    }

    return 0;
}