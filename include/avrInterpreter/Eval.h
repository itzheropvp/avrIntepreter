#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class AVRError : public std::runtime_error {
public:
    explicit AVRError(const std::string& msg, int line = -1, int col = -1)
        : std::runtime_error(formatMessage(msg, line, col)) {}
private:
    static std::string formatMessage(const std::string& msg, int line, int col) {
        std::string full = msg;
        if (line != -1) full += " (line " + std::to_string(line);
        if (col != -1) full += ", col " + std::to_string(col);
        if (line != -1) full += ")";
        return full;
    }
};

using Value = std::variant<int, double, std::string, bool>;

class Eval {
private:
    std::unordered_map<std::string, Value> locals;


    static std::vector<std::string> splitTokens(const std::string& line);
    static Value parseValue(const std::string& token);
    static std::string valueToString(const Value& v);

    double toNumber(const std::string& token);
    double evalExpression(const std::string& expr);

    static void waitFunction(const Value& v);



public:
    Eval() = default;
    void evaluate(const std::string& code);
};
