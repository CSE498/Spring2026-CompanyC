#pragma once
/**
 * @file ExpressionParser.hpp
 *
 * @brief Provides functionality to parse and evaluate mathematical expressions.
 *
 * Supported features:
 *  - Binary operators: +, -, *, /
 *  - Unary minus (e.g. -x, -3, a*-b, -(x+1))
 *  - Parentheses for grouping
 *  - Floating point numbers (e.g. 3, 3.14, .5, 5.)
 *  - Variables consisting of letters, digits, and underscores
 *    (must begin with a letter or underscore)
 *  - Arbitrary whitespace (ignored)
 */
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace cse498 {

    class ExpressionParser {
    public:
        // Map of variable names to numeric values
        using VarMap = std::unordered_map<std::string, double>;

        /**
        * @brief Parses an expression string and returns an evaluation function
        *
        * @param expr The input expression string
        * @return A function that takes a variable map and returns the computed result
        */
        static std::function<double(const VarMap&)> Parse(const std::string& expr);

    private:
        // Converts infix expression to Reverse Polish Notation
        static std::vector<std::string> ToRPN(const std::string& expr);

        // Splits raw input into tokens
        static std::vector<std::string> Tokenize(const std::string& expr);

        // Token classification helpers
        static bool IsNumber(const std::string& token);
        static bool IsIdentifier(const std::string& token);
        static bool IsOperator(const std::string& token);

        // Operator precedence helper
        static int Precedence(const std::string& op);
    };

} // namespace cse498
