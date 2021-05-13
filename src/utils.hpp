#pragma once
#include <string>


bool utils_is_digit(char c) {
    return c >= '0' && c <= '9';
}
        
bool utils_is_alpha(char c) {
    return  (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
}

bool utils_is_alphanumeric(char c) {
    return utils_is_digit(c) || utils_is_alpha(c); 
}

std::string utils_get_inbetween(std::string source, int p1, int p2) {
    std::string value;
    
    for (int i = p1; i < p2; i++) {
        value.push_back(source.at(i));
    }

    return value;
}