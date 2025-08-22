#include "../include/avrInterpreter/Eval.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <thread>
#include <chrono>
#include <stack>
#include <functional>

std::vector<std::string> Eval::splitTokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;

    for (const char c : line) {
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
    else throw InterpError("Invalid wait argument type");
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
        throw InterpError("Invalid value '" + token + "'");
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
        catch (...) { throw InterpError("Cannot convert '" + s + "' to number"); }
    }

    try {
        if (token.front() == '"' && token.back() == '"')
            return std::stod(token.substr(1, token.size() - 2));
        if (token.find('.') != std::string::npos) return std::stod(token);
        return std::stoi(token);
    }
    catch (...) { throw InterpError("Invalid number: " + token); }
}

double Eval::evalExpression(const std::string& expr) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : expr) {
        if (std::isspace(c)) continue;
        if (std::isdigit(c) || c == '.' || std::isalpha(c)) {
            current += c;
        } else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            std::string op(1, c);
            tokens.push_back(op);
        }
    }
    if (!current.empty()) tokens.push_back(current);

    std::vector<std::string> output;
    std::stack<std::string> ops;
    auto prec = [](const std::string& o) {
        if (o == "+" || o == "-") return 1;
        if (o == "*" || o == "/") return 2;
        return 0;
    };

    for (auto &t : tokens) {
        if (t == "(") {
            ops.push(t);
        } else if (t == ")") {
            while (!ops.empty() && ops.top() != "(") {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.pop();
        } else if (t == "+" || t == "-" || t == "*" || t == "/") {
            while (!ops.empty() && prec(ops.top()) >= prec(t)) {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.push(t);
        } else {
            output.push_back(t);
        }
    }
    while (!ops.empty()) {
        output.push_back(ops.top());
        ops.pop();
    }

    std::stack<double> st;
    for (auto &tk : output) {
        if (tk == "+" || tk == "-" || tk == "*" || tk == "/") {
            auto b = st.top(); st.pop();
            auto a = st.top(); st.pop();
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
                throw InterpError("Invalid local declaration", line_number);

            const std::string& name = tokens[1];

            try {
                if (trimmed.find_first_of("+-*/") != std::string::npos) {
                    std::string expr = trimmed.substr(trimmed.find('=') + 1);
                    double res = evalExpression(expr);
                    locals[name] = Value(res);
                } else {
                    locals[name] = parseValue(tokens[3]);
                }
            } catch (const std::exception& e) {
                throw InterpError(std::string("Error evaluating local '") + name + "': " + e.what(), line_number);
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
                        throw InterpError("Variable '" + part + "' not defined", line_number);
                    out += valueToString(locals[part]);
                }
            }
            std::cout << out << std::endl;
        } else if (tokens[0] == "wait") {
            if (tokens.size() != 2)
                throw InterpError("Invalid wait syntax", line_number);

            const std::string& arg = tokens[1];
            Value v;
            if (locals.find(arg) != locals.end()) v = locals[arg];
            else v = parseValue(arg);

            try {
                waitFunction(v);
            } catch (const std::exception& e) {
                throw InterpError(std::string("Error in wait: ") + e.what(), line_number);
            }
        } else if (tokens[0] == "if") {
            const std::string& ifLine = trimmed;

            std::string wholeBlock;
            int nested = 0;
            while (std::getline(stream, line)) {
                ++line_number;
                std::string t = line;
                t.erase(0, t.find_first_not_of(" \t"));
                if (t.rfind("if",0)==0) nested++;
                if (t == "end") {
                    if(nested == 0) break;
                    nested--;
                }
                wholeBlock += line + "\n";
            }

            struct Part {
                std::string type; // "if", "elseif", "else"
                std::string cond;
                std::string code;
            };

            std::vector<Part> parts;
            // string comparation bla bla bla bullshit + is ok do not care to do string::append. IT WORKS SO I LEAVE LIKE THIS
            std::istringstream bl( ifLine + "\n" + wholeBlock );
            std::string ln;
            Part current;
            bool first = true;
            while(std::getline(bl, ln)) {
                std::string t = ln;
                t.erase(0, t.find_first_not_of(" \t"));

                if( (t.rfind("if",0)==0 && first) || t.rfind("elseif",0)==0 || t=="else" ) {
                    if(!first) parts.push_back(current);
                    current = Part();
                    current.code.clear();
                    first = false;

                    if(t.rfind("if",0)==0) { current.type="if"; current.cond = t.substr(2); }
                    if(t.rfind("elseif",0)==0){ current.type="elseif"; current.cond = t.substr(6);}
                    if(t=="else"){ current.type="else"; }

                    auto posThen = current.cond.find("then");
                    if(posThen!=std::string::npos) current.cond = current.cond.substr(0,posThen);

                    current.cond.erase(0,current.cond.find_first_not_of(" \t"));
                    current.cond.erase(current.cond.find_last_not_of(" \t")+1);
                }
                else if(t!="end"){
                    current.code += ln + "\n";
                }
            }
            parts.push_back(current);

            auto evalCond = [&](const std::string &c)->bool{
                std::string ops[] = {">=", "<=", "==", "!=", ">", "<"};
                for (auto &op: ops) {
                    auto pos = c.find(op);
                    if(pos!=std::string::npos){
                        std::string left=c.substr(0,pos);
                        std::string right=c.substr(pos+op.size());
                        left.erase(0,left.find_first_not_of(" \t"));
                        left.erase(left.find_last_not_of(" \t")+1);
                        right.erase(0,right.find_first_not_of(" \t"));
                        right.erase(right.find_last_not_of(" \t")+1);
                        const double a = evalExpression(left);
                        const double b = evalExpression(right);
                        if(op==">") return a>b;
                        if(op=="<") return a<b;
                        if(op==">=") return a>=b;
                        if(op=="<=") return a<=b;
                        if(op=="==") return a==b;
                        if(op=="!=") return a!=b;
                    }
                }
                throw InterpError("Invalid condition '" + c + "'", line_number);
            };

            bool executed = false;
            for(auto &p: parts){
                if(p.type=="else"){
                    if(!executed){
                        Eval inner;
                        inner.locals = this->locals;
                        inner.evaluate(p.code);
                        this->locals = inner.locals;
                        executed = true;
                    }
                }
                else{
                    bool res = evalCond(p.cond);
                    if(res && !executed){
                        Eval inner;
                        inner.locals = this->locals;
                        inner.evaluate(p.code);
                        this->locals = inner.locals;
                        executed = true;
                    }
                }
            }
        } else if (tokens[0] == "while") {
            // condLine fucking means condition like this --> while <CONDITION> do
            std::string condline = trimmed.substr(5);
            auto posDo = condline.find("do");
            if (posDo == std::string::npos)
                throw InterpError("Missing 'do' in while", line_number);

            std::string condition = condline.substr(0, posDo);
            // trim
            condition.erase(0, condition.find_first_not_of(" \t"));
            condition.erase(condition.find_last_not_of(" \t") + 1);

            // Prendiamo il blocco interno al while (fino a "end")
            std::string block;
            int nested = 0;
            while (std::getline(stream, line)) {
                ++line_number;
                std::string t = line;
                t.erase(0, t.find_first_not_of(" \t"));
                if (t.rfind("while", 0) == 0) nested++;
                if (t == "end") {
                    if (nested == 0) break;
                    nested--;
                }
                block += line + "\n";
            }

            // Funzione per valutare la condizione (ricicliamo comparatori logici)
            auto evalCond = [&](const std::string& c)->bool {
                std::string ops[] = { ">=", "<=", "==", "!=", ">", "<" };
                for (auto &op : ops) {
                    auto p = c.find(op);
                    if (p != std::string::npos) {
                        std::string left = c.substr(0, p);
                        std::string right = c.substr(p + op.size());
                        left.erase(0, left.find_first_not_of(" \t"));
                        left.erase(left.find_last_not_of(" \t") + 1);
                        right.erase(0, right.find_first_not_of(" \t"));
                        right.erase(right.find_last_not_of(" \t") + 1);
                        const double a = evalExpression(left);
                        const double b = evalExpression(right);
                        if (op == ">") return a > b;
                        if (op == "<") return a < b;
                        if (op == ">=") return a >= b;
                        if (op == "<=") return a <= b;
                        if (op == "==") return a == b;
                        if (op == "!=") return a != b;
                    }
                }
                throw InterpError("Invalid condition '" + c + "'", line_number);
            };

            while (evalCond(condition)) {
                Eval inner;
                inner.locals = this->locals;
                inner.evaluate(block);
                this->locals = inner.locals;
            }
        } else {
            throw InterpError("Unknown command '" + tokens[0] + "'", line_number);
        }
    }
}
