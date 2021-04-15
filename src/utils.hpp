#pragma once

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}
        
bool is_alpha(char c) {
    return  (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '_';
}

bool is_alphanumeric(char c) {
    return is_digit(c) || is_alpha(c); 
}