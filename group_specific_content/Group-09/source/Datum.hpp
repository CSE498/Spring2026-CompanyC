#pragma once


#include <variant>
#include <string>
#include <cmath>
#include <limits>

class Datum
{
public:

    Datum();
    Datum(const std::string &);
    Datum(const char* );
    Datum(const double);
    Datum(const bool);

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
