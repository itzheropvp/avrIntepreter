#pragma once
#include <string>
#include <memory>

namespace avr {

    class Interpreter {
    public:
        Interpreter();
        ~Interpreter();

        // Run code from a string
        bool run(const std::string& code);

        // Run code from a file
        bool runFile(const std::string& filename);

        // had to add [[nodiscard]] cuz compiler bothers AAAAIOJAIOJIGOJASIOG
        [[nodiscard]] std::string getLastError() const;

    private:
        class Impl;              // forward-declared PIMPL
        std::unique_ptr<Impl> p; // hides Eval from public API
    };

}
