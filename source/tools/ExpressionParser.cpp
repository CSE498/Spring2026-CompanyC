#include "ExpressionParser.hpp"

#include <cctype>
#include <cmath>
#include <functional>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498 {

namespace {


static const char* kUnaryMinusToken = "NEG";

struct OpInfo {
    int precedence;
    int arity; // 1 for unary 2 for binary
};


const std::unordered_map<std::string, OpInfo>& OpTable() {
    static const std::unordered_map<std::string, OpInfo> ops = {
        {"+",   {1, 2}},
        {"-",   {1, 2}},
        {"*",   {2, 2}},
        {"/",   {2, 2}},
        {kUnaryMinusToken, {3, 1}},
    };
    return ops;
}

bool IsOperatorToken(const std::string& t) {
    return OpTable().find(t) != OpTable().end();
}

int PrecedenceOf(const std::string& op) {
    auto it = OpTable().find(op);
    return (it == OpTable().end()) ? 0 : it->second.precedence;
}

int ArityOf(const std::string& op) {
    auto it = OpTable().find(op);
    return (it == OpTable().end()) ? 0 : it->second.arity;
}

// Strict number validation
bool IsNumberTokenStrict(const std::string& token) {
    if (token.empty()) return false;

    bool seen_digit = false;
    bool seen_dot = false;

    for (size_t i = 0; i < token.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(token[i]);
        if (std::isdigit(c)) {
            seen_digit = true;
            continue;
        }
        if (token[i] == '.') {
            if (seen_dot) return false;
            seen_dot = true;
            continue;
        }
        return false;
    }

    // Must contain at least one digit
    return seen_digit;
}

double ParseDoubleStrict(const std::string& token) {
    try {
        size_t pos = 0;
        double val = std::stod(token, &pos);
        if (pos != token.size()) {
            throw std::invalid_argument("Bad number token: " + token);
        }
        if (!std::isfinite(val)) {
            throw std::invalid_argument("Non-finite number token: " + token);
        }
        return val;
    } catch (const std::exception&) {

        throw std::invalid_argument("Bad number token: " + token);
    }
}

bool IsIdentifierToken(const std::string& token) {
    if (token.empty()) return false;
    const unsigned char c0 = static_cast<unsigned char>(token[0]);
    if (!(std::isalpha(c0) || token[0] == '_')) return false;

    for (size_t i = 1; i < token.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(token[i]);
        if (!(std::isalnum(c) || token[i] == '_')) return false;
    }
    return true;
}

} // namespace

std::function<double(const ExpressionParser::VarMap&)>
ExpressionParser::Parse(const std::string& expr)
{
    auto rpn = ToRPN(expr);

    return [rpn](const VarMap& vars) -> double {
        std::stack<double> st;

        for (const auto& token : rpn) {
            if (IsNumberTokenStrict(token)) {
                st.push(ParseDoubleStrict(token));
                continue;
            }

            if (IsOperatorToken(token)) {
                const int arity = ArityOf(token);
                if (static_cast<int>(st.size()) < arity) {
                    throw std::runtime_error("Not enough operands for operator: " + token);
                }

                if (arity == 1) {
                    double a = st.top(); st.pop();
                    if (token == kUnaryMinusToken) {
                        st.push(-a);
                    } else {
                        throw std::runtime_error("Unhandled unary operator: " + token);
                    }
                } else { // arity == 2
                    double b = st.top(); st.pop();
                    double a = st.top(); st.pop();

                    if (token == "+") st.push(a + b);
                    else if (token == "-") st.push(a - b);
                    else if (token == "*") st.push(a * b);
                    else if (token == "/") {
                        if (b == 0.0) throw std::runtime_error("Division by zero");
                        st.push(a / b);
                    } else {
                        throw std::runtime_error("Unhandled binary operator: " + token);
                    }
                }
                continue;
            }

            // variable
            if (!IsIdentifierToken(token)) {
                throw std::invalid_argument("Unknown token in RPN: " + token);
            }

            auto it = vars.find(token);
            if (it == vars.end())
                throw std::runtime_error("Unknown variable: " + token);

            st.push(it->second);
        }

        if (st.size() != 1)
            throw std::runtime_error("Invalid expression evaluation (stack size = " + std::to_string(st.size()) + ")");

        return st.top();
    };
}

std::vector<std::string> ExpressionParser::ToRPN(const std::string& expr)
{
    std::vector<std::string> output;
    std::stack<std::string> ops;

    auto tokens = Tokenize(expr);

    // Track what kind of token we expect to decide unary vs binary '-'

    bool expect_operand = true;

    for (const auto& raw : tokens) {
        std::string token = raw;

        if (token == "-") {

            if (expect_operand) token = kUnaryMinusToken;
        }

        if (IsNumberTokenStrict(token) || IsIdentifierToken(token)) {
            output.push_back(token);
            expect_operand = false;
        }
        else if (IsOperatorToken(token)) {
            while (!ops.empty() && IsOperatorToken(ops.top()) &&
                   PrecedenceOf(ops.top()) >= PrecedenceOf(token)) {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.push(token);
            expect_operand = true;
        }
        else if (token == "(") {
            ops.push(token);
            expect_operand = true;
        }
        else if (token == ")") {
            while (!ops.empty() && ops.top() != "(") {
                output.push_back(ops.top());
                ops.pop();
            }
            if (ops.empty()) throw std::invalid_argument("Mismatched parentheses");
            ops.pop();
            expect_operand = false;
        }
        else {

            throw std::invalid_argument("Unknown token: " + token);
        }
    }

    while (!ops.empty()) {
        if (ops.top() == "(") throw std::invalid_argument("Mismatched parentheses");
        output.push_back(ops.top());
        ops.pop();
    }

    return output;
}

std::vector<std::string> ExpressionParser::Tokenize(const std::string& expr)
{
    std::vector<std::string> tokens;

    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (std::isspace(static_cast<unsigned char>(c)))
            continue;

        // number
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            std::string current;
            bool seen_dot = (c == '.');
            bool seen_digit = std::isdigit(static_cast<unsigned char>(c)) != 0;

            current += c;

            while (i + 1 < expr.size()) {
                char ch = expr[i + 1];
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    seen_digit = true;
                    current += ch;
                    ++i;
                    continue;
                }
                if (ch == '.') {
                    if (seen_dot) {
                        throw std::invalid_argument("Invalid number token (multiple '.'): " + current + ".");
                    }
                    seen_dot = true;
                    current += ch;
                    ++i;
                    continue;
                }
                break;
            }

            if (!seen_digit) {
                // token was just like "." or etc
                throw std::invalid_argument("Invalid number token: " + current);
            }

            tokens.push_back(current);
            continue;
        }

        // identifier
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string current;
            current += c;

            while (i + 1 < expr.size()) {
                char ch = expr[i + 1];
                if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_')
                    break;
                current += ch;
                ++i;
            }
            tokens.push_back(current);
            continue;
        }

        // operators and parentheses
        const std::string one(1, c);
        if (one == "(" || one == ")" || IsOperatorToken(one)) {
            tokens.push_back(one);
            continue;
        }

        throw std::invalid_argument(std::string("Invalid character in expression: '") + c + "'");
    }

    return tokens;
}


bool ExpressionParser::IsNumber(const std::string& token) { return IsNumberTokenStrict(token); }
bool ExpressionParser::IsIdentifier(const std::string& token) { return IsIdentifierToken(token); }
bool ExpressionParser::IsOperator(const std::string& token) { return IsOperatorToken(token); }
int ExpressionParser::Precedence(const std::string& op) { return PrecedenceOf(op); }

} // namespace cse498
