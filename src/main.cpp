#include <iostream>
#include <string>
#include <fstream>

void run(const std::string& file) {
    
}

void run_file(const std::string& path) {
    std::ifstream file (path);
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
    } 

    return 0;
}