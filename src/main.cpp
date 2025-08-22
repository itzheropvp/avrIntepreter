#include <iostream>
#include "avrInterpreter/Interpreter.h"

int main(const int argc /* same shit here */, char* argv[]) {
    // omg another const that i need to add!
    const avr::Interpreter interp;

    if (argc > 1) {
        if (!interp.runFile(argv[1])) {
            std::cerr << "Error: " << interp.getLastError() << std::endl;
            return 1;
        }
    } else {
        std::cout << "avrInterpreter REPL (Ctrl+D to quit)\n";
        std::string line;
        while (std::getline(std::cin, line)) {
            if (!interp.run(line)) {
                std::cerr << "Error: " << interp.getLastError() << std::endl;
            }
        }
    }
    return 0;
}
