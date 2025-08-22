#include "avrInterpreter/Interpreter.h"
#include "avrInterpreter/Eval.h"  // internal engine
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

    bool Interpreter::run(const std::string& code) {
        try {
            p->eval.evaluate(code);
            return true;
        } catch (const std::exception& e) {
            p->lastError = e.what();
            return false;
        }
    }

    bool Interpreter::runFile(const std::string& filename) {
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
