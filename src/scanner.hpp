#pragma once
#include <string>
#include <vector>
#include "token.hpp"
#include "utils.hpp"

struct Scanner {
    std::string source = "";
    std::vector<Token> tokens;

    int current = 0;
};