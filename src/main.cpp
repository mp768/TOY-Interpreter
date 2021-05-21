#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#undef EOF

/*
    Use to activate debug printing for all interpreter operations such as the parser, 
    abstract syntax tree printer, and tokenizer.

    NOTE: MIGHT BE SEPERATED INTO SEPERATE DEFINITIONS FOR THE DIFFERENT PARTS OF THE INTERPRETER.
*/
#define DEBUG_PRINTO
#define DEBUG_PRINT_INTERPRETERO

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

    Interpreter i;
    i.evaluate_stmts(parser_stmts);

    //std::cout << "1" << std::endl;
    //Parser_parse(parser);
    //i.evaluate_expr(&CURRENT_EXPR);
    ////Parser_synchronize(parser);
//
//
    //for (int o = 1; o < 16; o++) {
    //    #ifdef DEBUG_PRINT_INTERPRETER
    //        std::cout << o + 1 << std::endl;
    //    #endif
    //    Parser_parse(parser);
    //    i.evaluate_expr(&CURRENT_EXPR);
    //    Parser_synchronize(parser);
    //}

    if (had_error) return;
}

bool is_file_empty(std::ifstream& file) {
    return file.peek() == std::ifstream::traits_type::eof();
}

void run_file(const std::string& path) {
    std::ifstream file(path);
    if (is_file_empty(file)) {
        std::cout << " File is empty or doesn't exist!" << std::endl;
        return;
    }
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