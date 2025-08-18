#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class AVRError : public std::runtime_error {
public:
    explicit AVRError(const std::string& msg);
};

using Value = std::variant<int, double, std::string, bool>;

class Eval {
private:
    std::unordered_map<std::string, Value> locals;


    static std::vector<std::string> splitTokens(const std::string& line);
    static void waitFunction(const Value& v);

    static Value parseValue(const std::string& token);
    static std::string valueToString(const Value& v);
    std::vector<std::string> tokenize(const std::string& line, char delimiter = ' ');

    void cmdPrint(const std::vector<std::string>& args);
    void cmdWait(const std::vector<std::string>& args);
    void cmdLocal(const std::vector<std::string>& args);

public:
    Eval() = default;
    void evaluate(const std::string& code);
};
