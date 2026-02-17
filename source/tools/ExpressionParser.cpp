#include "ExpressionParser.hpp"

#include <stdexcept>
#include <cctype>
#include <stack>

namespace cse498 {

std::function<double(const ExpressionParser::VarMap&)>
ExpressionParser::Parse(const std::string& expr)
{
    auto rpn = ToRPN(expr);

    return [rpn](const VarMap& vars) {
        std::stack<double> stack;

        for (const auto& token : rpn) {
            if (IsNumber(token)) {
                stack.push(std::stod(token));
            }
            else if (IsOperator(token)) {
                if (stack.size() < 2)
                    throw std::runtime_error("Invalid expression");

                double b = stack.top(); stack.pop();
                double a = stack.top(); stack.pop();

                if (token == "+") stack.push(a + b);
                else if (token == "-") stack.push(a - b);
                else if (token == "*") stack.push(a * b);
                else if (token == "/") {
                    if (b == 0.0)
                        throw std::runtime_error("Division by zero");
                    stack.push(a / b);
                }
            }
            else { // variable
                auto it = vars.find(token);
                if (it == vars.end())
                    throw std::runtime_error("Unknown variable: " + token);
                stack.push(it->second);
            }
        }

        if (stack.size() != 1)
            throw std::runtime_error("Invalid expression evaluation");

        return stack.top();
    };
}

std::vector<std::string> ExpressionParser::ToRPN(const std::string& expr)
{
    std::vector<std::string> output;
    std::stack<std::string> operators;

    auto tokens = Tokenize(expr);

    for (const auto& token : tokens) {
        if (IsNumber(token) || IsIdentifier(token)) {
            output.push_back(token);
        }
        else if (IsOperator(token)) {
            while (!operators.empty() &&
                   IsOperator(operators.top()) &&
                   Precedence(operators.top()) >= Precedence(token)) {
                output.push_back(operators.top());
                operators.pop();
            }
            operators.push(token);
        }
        else if (token == "(") {
            operators.push(token);
        }
        else if (token == ")") {
            while (!operators.empty() && operators.top() != "(") {
                output.push_back(operators.top());
                operators.pop();
            }
            if (operators.empty())
                throw std::invalid_argument("Mismatched parentheses");
            operators.pop();
        }
    }

    while (!operators.empty()) {
        if (operators.top() == "(")
            throw std::invalid_argument("Mismatched parentheses");
        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}

std::vector<std::string> ExpressionParser::Tokenize(const std::string& expr)
{
    std::vector<std::string> tokens;
    std::string current;

    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (std::isspace(static_cast<unsigned char>(c)))
            continue;

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            current.clear();
            while (i < expr.size()) {
                char ch = expr[i];
                if (!std::isdigit(static_cast<unsigned char>(ch)) && ch != '.')
                    break;
                current += ch;
                ++i;
            }
            --i;
            tokens.push_back(current);
        }
        else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            current.clear();
            while (i < expr.size()) {
                char ch = expr[i];
                if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_')
                    break;
                current += ch;
                ++i;
            }
            --i;
            tokens.push_back(current);
        }
        else if (IsOperator(std::string(1, c)) || c == '(' || c == ')') {
            tokens.emplace_back(1, c);
        }
        else {
            throw std::invalid_argument("Invalid character in expression");
        }
    }

    return tokens;
}

bool ExpressionParser::IsNumber(const std::string& token)
{
    return !token.empty() &&
           (std::isdigit(static_cast<unsigned char>(token[0])) || token[0] == '.');
}

bool ExpressionParser::IsIdentifier(const std::string& token)
{
    return !token.empty() &&
           (std::isalpha(static_cast<unsigned char>(token[0])) || token[0] == '_');
}

bool ExpressionParser::IsOperator(const std::string& token)
{
    return token == "+" || token == "-" ||
           token == "*" || token == "/";
}

int ExpressionParser::Precedence(const std::string& op)
{
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/") return 2;
    return 0;
}

} // namespace cse498
