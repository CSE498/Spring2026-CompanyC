#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace cse498 {

    class ExpressionParser {
    public:
        using VarMap = std::unordered_map<std::string, double>;

        static std::function<double(const VarMap&)> Parse(const std::string& expr);

    private:
        static std::vector<std::string> ToRPN(const std::string& expr);
        static std::vector<std::string> Tokenize(const std::string& expr);

        static bool IsNumber(const std::string& token);
        static bool IsIdentifier(const std::string& token);
        static bool IsOperator(const std::string& token);

        static int Precedence(const std::string& op);
    };

} // namespace cse498
