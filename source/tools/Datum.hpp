/**
 * @file Datum.hpp
 * @brief Defines the Datum class, a type safe variant wrapper for string, double, and bool values.
 *
 * The Datum class provides a unified interface for storing and operating on heterogeneous
 * data types (std::string, double, bool) using std::variant. It supports arithmetic
 * operators (+, -, *, /), compound assignment (+=), comparison operators (==, !=, <, >, <=, >=),
 * assignment operators, stream output, and type-checking and value-access methods.
 *
 * Key behaviors:
 * - Arithmetic on doubles and bools operates numerically.
 *  - operator+ on two strings adds numerically if both are valid numbers, otherwise concatenates.
 *  - operator+= follows the same rule as operator+.
 *  - operator-, *, / on strings attempt numeric conversion; non-numeric strings yield NaN.
 *  - Division by zero returns NaN rather than throwing.
 *  - operator+ on two strings concatenates them instead of converting to double.
 *  - operator+= on strings appends; on numeric types adds in place.
 *  - Division by zero returns NaN.
 *  - Bool is stored as bool, not converted to double (integral template excludes bool).
 *
 * @author  Group 9
 * @course  CSE 498
 */

#pragma once

#include <concepts>
#include <variant>
#include <string>
#include <ostream>
#include <cmath>
#include <limits>

namespace cse498 {

    class Datum
    {
    public:

        Datum();
        Datum(const std::string&);
        Datum(const char*);
        Datum(double);
        Datum(bool);

        // Excludes bool so Datum(true) stores a bool, not 1.0
        template<std::integral T>
        requires (!std::same_as<T, bool>)
        Datum(T val) : Datum(static_cast<double>(val)) {}

        // --- Value access ---
        std::string AsString() const;
        double      AsDouble() const;
        bool        AsBool()   const;

        // --- Type checking ---
        bool IsString() const;
        bool IsDouble() const;
        bool IsBool()   const;

        // --- Assignment ---
        Datum& operator=(double val);
        Datum& operator=(bool val);
        Datum& operator=(const std::string& val);
        Datum& operator=(const char* val);

        // --- Arithmetic ---
        // Note: operator+ on two strings concatenates rather than converting to NaN
        Datum operator+(const Datum& rhs) const;
        Datum operator-(const Datum& rhs) const;
        Datum operator*(const Datum& rhs) const;
        Datum operator/(const Datum& rhs) const;

        // --- Compound assignment ---
        // Strings: appends rhs.AsString()
        // Numeric: adds rhs.AsDouble() in place
        Datum& operator+=(const Datum& rhs);

        // --- Comparison ---
        bool operator==(const Datum& rhs) const;
        bool operator!=(const Datum& rhs) const;
        bool operator<(const Datum& rhs)  const;
        bool operator>(const Datum& rhs)  const;
        bool operator>=(const Datum& rhs) const;
        bool operator<=(const Datum& rhs) const;

        // --- Stream output ---
        friend std::ostream& operator<<(std::ostream& os, const Datum& d);

    private:

        std::variant<std::string, double, bool> mValue;

        static double NaN();
    };

} // namespace cse498