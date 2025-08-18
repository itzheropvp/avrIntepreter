#include "Eval.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <thread>
#include <chrono>

AVRError::AVRError(const std::string& msg) : std::runtime_error(msg) {}

std::vector<std::string> Eval::splitTokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            token += c;
            if (in_quotes) {
                tokens.push_back(token);
                token.clear();
                in_quotes = false;
            } else {
                in_quotes = true;
            }
        } else if (isspace(c) && !in_quotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty())
        tokens.push_back(token);

    return tokens;
}

void Eval::waitFunction(const Value& v) {
    int seconds = 0;
    if (std::holds_alternative<int>(v)) seconds = std::get<int>(v);
    else if (std::holds_alternative<double>(v)) seconds = static_cast<int>(std::get<double>(v));
    else throw AVRError("Invalid wait argument type");
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}


// TYPE SUPPORT

Value Eval::parseValue(const std::string &token) {
    if (token == "true") return Value(true);
    if (token == "false") return Value(false);
    if (token == "null") return Value(nullptr);

    // string
    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        return Value(token.substr(1, token.size() - 2));
    }

    // Number (double or int)
    try {
        if (token.find('.') != std::string::npos) {
            return Value(std::stod(token));
        }
        return std::stoi(token);
    }
    catch (...) {
        throw AVRError("Invalid value '" + token + "'");
    }
}

std::string Eval::valueToString(const Value& v) {
    if (std::holds_alternative<int>(v)) return std::to_string(std::get<int>(v));
    if (std::holds_alternative<double>(v)) return std::to_string(std::get<double>(v));
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
    return std::get<std::string>(v);
}

void Eval::evaluate(const std::string& code) {
    std::istringstream stream(code);
    std::string line;
    int line_number = 0;

    while (std::getline(stream, line)) {
        ++line_number;
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        if (trimmed.empty() || trimmed.rfind("--", 0) == 0)
            continue; // comment or empty line

        auto tokens = splitTokens(trimmed);
        if (tokens.empty()) continue;

        if (tokens[0] == "local") {
            if (tokens.size() < 2)
                throw AVRError("Invalid local declaration at line " + std::to_string(line_number));

            const std::string& name = tokens[1];
            Value val = parseValue(tokens[2]);
            locals[name] = val;
        } else if (tokens[0] == "print") {
            std::string out;
            for (size_t i = 1; i < tokens.size(); ++i) {
                const std::string& part = tokens[i];
                if (part.size() >= 2 && part.front() == '"' && part.back() == '"') {
                    out += part.substr(1, part.size() - 2);
                }
                else {
                    if (locals.find(part) == locals.end())
                        throw AVRError("Variable '" + part + "' not defined at line " + std::to_string(line_number));
                    out += valueToString(locals[part]);
                }
            }
            std::cout << out << std::endl;
        } else if (tokens[0] == "wait") {
            if (tokens.size() != 2)
                throw AVRError("Invalid wait syntax at line " + std::to_string(line_number));

            std::string arg = tokens[1];
            Value v;
            if (locals.find(arg) != locals.end()) v = locals[arg];
            else v = parseValue(arg);

            waitFunction(v);
        } else {
            throw AVRError("Unknown command \"" + tokens[0] + "\" at line " + std::to_string(line_number));
        }
    }
}
