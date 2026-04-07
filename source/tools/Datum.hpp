/**
 * @file Datum.hpp
 * @brief Defines the Datum class, a type safe variant wrapper for string, double, and bool values.
 *
 * The Datum class provides a unified interface for storing and operating on heterogeneous
 * data types (std::string, double, bool) using std::variant. It supports arithmetic
 * operators (+, -, *, /) and comparison operators (==, !=, <, >, <=, >=), as well as
 * type-checking and value-access methods.
 *
 * Arithmetic on non-numeric types or type mismatches will result in NaN.
 *
 * @author  Ian Wettlaufer
 * @date    2/21/2026
 * @course  CSE 498
 */

#pragma once


#include <concepts>
#include <variant>
#include <string>
#include <cmath>
#include <limits>

namespace cse498 {

    class Datum
    {
    public:

        Datum();
        Datum(const std::string &);
        Datum(const char* );
        Datum(const double);
        Datum(const bool);

        template<std::integral T>
        Datum(T val) : Datum(static_cast<double>(val)) {}

        std::string AsString() const;
        double AsDouble() const;
        bool AsBool() const;

        bool IsString() const;
        bool IsDouble() const;
        bool IsBool() const;

        Datum operator+(const Datum& rhs) const;
        Datum operator-(const Datum& rhs) const;
        Datum operator*(const Datum& rhs) const;
        Datum operator/(const Datum& rhs) const;

        bool operator==(const Datum& rhs) const;
        bool operator!=(const Datum& rhs) const;
        bool operator<(const Datum& rhs) const;
        bool operator>(const Datum& rhs) const;
        bool operator>=(const Datum& rhs) const;
        bool operator<=(const Datum& rhs) const;

    private:

        std::variant<std::string, double, bool> mValue;
            
        static double NaN();
    };
}