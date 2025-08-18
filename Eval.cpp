#include "Eval.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <thread>
#include <chrono>
#include <stack>
#include <functional>

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

    if (!token.empty()) tokens.push_back(token);
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

    // string
    if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
        return Value(token.substr(1, token.size() - 2));
    }

    // Number (double or int)
    try {
        if (token.find('.') != std::string::npos) return Value(std::stod(token));
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

double Eval::toNumber(const std::string& token) {
    if (locals.find(token) != locals.end()) {
        const Value& v = locals[token];
        if (std::holds_alternative<int>(v)) return std::get<int>(v);
        if (std::holds_alternative<double>(v)) return std::get<double>(v);

        std::string s = std::get<std::string>(v);
        try { return std::stod(s); }
        catch (...) { throw AVRError("Cannot convert '" + s + "' to number"); }
    }

    try {
        if (token.front() == '"' && token.back() == '"')
            return std::stod(token.substr(1, token.size() - 2));
        if (token.find('.') != std::string::npos) return std::stod(token);
        return std::stoi(token);
    }
    catch (...) { throw AVRError("Invalid number: " + token); }
}

double Eval::evalExpression(const std::string& expr) {
    std::istringstream ss(expr);
    std::string t;
    std::vector<std::string> outputQueue;
    std::stack<std::string> ops;

    auto prec = [](const std::string& o) {
        if (o == "+" || o == "-") return 1;
        if (o == "*" || o == "/") return 2;
        return 0;
    };

    while (ss >> t) {
        if (t == "(") ops.push(t);
        else if (t == ")") {
            while (!ops.empty() && ops.top() != "(") {
                outputQueue.push_back(ops.top());
                ops.pop();
            }
            if (!ops.empty()) ops.pop();
        }
        else if (t == "+" || t == "-" || t == "*" || t == "/") {
            while (!ops.empty() && prec(ops.top()) >= prec(t)) {
                outputQueue.push_back(ops.top());
                ops.pop();
            }
            ops.push(t);
        } else {
            outputQueue.push_back(t);
        }
    }

    while (!ops.empty()) {
        outputQueue.push_back(ops.top());
        ops.pop();
    }

    std::stack<double> st;
    for (auto &tk : outputQueue) {
        if (tk == "+" || tk == "-" || tk == "*" || tk == "/") {
            double b = st.top(); st.pop();
            double a = st.top(); st.pop();

            if (tk == "+") st.push(a + b);
            if (tk == "-") st.push(a - b);
            if (tk == "*") st.push(a * b);
            if (tk == "/") st.push(a / b);
        } else {
            st.push(toNumber(tk));
        }
    }

    return st.top();
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
            if (tokens.size() < 4 || tokens[2] != "=")
                throw AVRError("Invalid local declaration at line " + std::to_string(line_number));

            const std::string& name = tokens[1];

            if (trimmed.find_first_of("+-*/") != std::string::npos) {
                std::string expr = trimmed.substr(trimmed.find("=") + 1);
                double res = evalExpression(expr);
                locals[name] = Value(res);
            } else {
                locals[name] = parseValue(tokens[3]);
            }

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
