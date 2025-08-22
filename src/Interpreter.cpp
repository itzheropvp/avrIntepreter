// custom engines
#include "avrInterpreter/Interpreter.h"
#include "avrInterpreter/Eval.h"

// c++ engines!
#include <fstream>
#include <sstream>

namespace avr {

    class Interpreter::Impl {
    public:
        Eval eval;
        std::string lastError;
    };

    Interpreter::Interpreter() : p(std::make_unique<Impl>()) {}
    Interpreter::~Interpreter() = default;

    // added const bcs fking linter give random ahh warning
    bool Interpreter::run(const std::string& code) const {
        try {
            p->eval.evaluate(code);
            return true;
        } catch (const std::exception& e) {
            p->lastError = e.what();
            return false;
        }
    }

    // honestly same shit here.. and idk how many times i need to add a const
    bool Interpreter::runFile(const std::string& filename) const {
        std::ifstream file(filename);
        if (!file.is_open()) {
            p->lastError = "Could not open file: " + filename;
            return false;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return run(buffer.str());
    }

    std::string Interpreter::getLastError() const {
        return p->lastError;
    }

} // namespace avr
