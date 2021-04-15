#pragma once
#include <string>
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

Scanner Scanner_new(std::string input) {
    return Scanner { input };
}