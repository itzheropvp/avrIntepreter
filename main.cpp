#include <iostream>
#include <fstream>
#include <string>
#include "Eval.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.avr>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        return 1;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Eval evaluator;
    evaluator.evaluate(content);

    return 0;
}