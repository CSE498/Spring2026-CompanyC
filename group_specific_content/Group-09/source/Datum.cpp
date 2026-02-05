#include "Datum.hpp"


//Helper function for error handling
//Return NaN for non numeric strings attempting to convert to string
//Also used for illegal math
double Datum::NaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

Datum::Datum() : mValue(NaN()) {}

Datum::Datum(double value) : mValue(value) {}

Datum::Datum(bool value) : mValue(value) {}

Datum::Datum(const std::string& value) : mValue(value) {}

Datum::Datum(const char* value) : mValue(std::string(value)) {}


//Type checking functions, returns original type

bool Datum::IsString() const {
    return std::holds_alternative<std::string>(mValue);
}

bool Datum::IsDouble() const {
    return std::holds_alternative<double>(mValue);
}

bool Datum::IsBool() const {
    return std::holds_alternative<bool>(mValue);
}
//Read as string, always works regardless of original data type
//If requesting on a bool type, uses all lowercase "true" and "false".
std::string Datum::AsString() const {
    if (IsString())
        return std::get<std::string>(mValue);

    if (IsDouble())
        return std::to_string(std::get<double>(mValue));

    if (IsBool())
        return std::get<bool>(mValue) ? "true" : "false";

    return "";
}
//Read datum as a double, for bool returns 1.0 for true and 0.0 for false.
double Datum::AsDouble() const {
    if (IsDouble()){
        return std::get<double>(mValue);
    }
    if (IsBool()){
        return std::get<bool>(mValue) ? 1.0 : 0.0;
    }
    if (IsString()){
        try 
        {
            return std::stod(std::get<std::string>(mValue));


        }catch (...){  //catch any exception that appears (Check if this is allowed)
            return NaN();
        }

    }
    return NaN();
}

//Behaves as expected, if the value is a double it checks that it is not NaN and not 0.0
//If its a string it checks if its empty, if so return false, if not return true
bool Datum::AsBool() const{
    if (IsBool()){
        return std::get<bool>(mValue);
    }
    if (IsDouble()){
        return std::get<double>(mValue) != 0.0 && !std::isnan(std::get<double>(mValue));

    }
    if (IsString()){

        return !std::get<std::string>(mValue).empty();
    }
    return false;

}

Datum Datum::operator+(const Datum& rhs) const {
    return Datum(AsDouble() + rhs.AsDouble()); //How will this handle NAN?


}
Datum Datum::operator-(const Datum& rhs)const{
    return Datum(AsDouble() - rhs.AsDouble()); 

}
Datum Datum::operator*(const Datum& rhs)const{
    return Datum(AsDouble() * rhs.AsDouble());

}
Datum Datum::operator/(const Datum& rhs)const{
    double divisor = rhs.AsDouble();
    
    if (divisor == 0.0 || std::isnan(divisor) || AsDouble() == NaN()){
        return Datum(NaN());

    }
 
    return Datum(AsDouble() / divisor);

}
//Comparison Operators
