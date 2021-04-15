#include <iostream>
#include <string>
#include <fstream>
#undef EOF

#include "token.hpp"
#include "utils.hpp"
#include "scanner.hpp"

void run(const std::string& file) {
    
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
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("toy interpreter usage: toy {file}");
        return 0;
    } else if (argc == 2) {
        run_file(argv[1]);
    }

    return 0;
}