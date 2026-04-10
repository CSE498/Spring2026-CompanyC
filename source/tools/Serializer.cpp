// Serializer.cpp — Group 9, Database Module
#include "Serializer.hpp"

#include <cassert>
#include <sstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <cstdlib>
#include <xlocale.h>

namespace cse498 {

bool Serializer::IsTypeRegistered(const std::string& type_id) const {
    return registry_.count(type_id) > 0;
}

// ------ serialize primitives ------

std::string Serializer::Serialize(int value) const {
    return "i:" + std::to_string(value) + ";";
}

// max_digits10 keeps full precision through a round-trip;
// classic locale ensures '.' decimal separator regardless of user locale
std::string Serializer::Serialize(double value) const {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::setprecision(std::numeric_limits<double>::max_digits10)
        << value;
    return "d:" + oss.str() + ";";
}

std::string Serializer::Serialize(bool value) const {
    return std::string("b:") + (value ? "1" : "0") + ";";
}

std::string Serializer::Serialize(char value) const {
    return std::string("c:") + value + ";";
}

// without this, Serialize("hello") matches the bool overload
// because pointer-to-bool is a standard conversion in C++
std::string Serializer::Serialize(const char* value) const {
    assert(value != nullptr);
    return Serialize(std::string(value));
}

// length prefix means semicolons/colons inside the string are fine
std::string Serializer::Serialize(const std::string& value) const {
    return "s:" + std::to_string(value.size()) + ":" + value + ";";
}

std::string Serializer::Serialize(long long value) const {
    return "l:" + std::to_string(value) + ";";
}

std::string Serializer::Serialize(unsigned long long value) const {
    return "l:" + std::to_string(value) + ";";
}

std::string Serializer::Serialize(float value) const {
    return Serialize(static_cast<double>(value));
}

std::string Serializer::Serialize(long value) const {
    return Serialize(static_cast<long long>(value));
}

std::string Serializer::Serialize(unsigned int value) const {
    return Serialize(static_cast<long long>(value));
}

std::string Serializer::Serialize(unsigned long value) const {
    return Serialize(static_cast<unsigned long long>(value));
}

// ------ deserialize wrappers ------

std::optional<int> Serializer::DeserializeInt(const std::string& data) const {
    size_t pos = 0;
    return DeserializeIntAt(data, pos);
}

std::optional<double> Serializer::DeserializeDouble(const std::string& data) const {
    size_t pos = 0;
    return DeserializeDoubleAt(data, pos);
}

std::optional<bool> Serializer::DeserializeBool(const std::string& data) const {
    size_t pos = 0;
    return DeserializeBoolAt(data, pos);
}

std::optional<char> Serializer::DeserializeChar(const std::string& data) const {
    size_t pos = 0;
    return DeserializeCharAt(data, pos);
}

std::optional<std::string> Serializer::DeserializeString(const std::string& data) const {
    size_t pos = 0;
    return DeserializeStringAt(data, pos);
}

std::optional<long long> Serializer::DeserializeLongLong(const std::string& data) const {
    size_t pos = 0;
    return DeserializeLongLongAt(data, pos);
}

std::optional<unsigned long long> Serializer::DeserializeUnsignedLongLong(const std::string& data) const {
    size_t pos = 0;
    return DeserializeUnsignedLongLongAt(data, pos);
}

std::optional<float> Serializer::DeserializeFloat(const std::string& data) const {
    size_t pos = 0;
    return DeserializeFloatAt(data, pos);
}

// ------ internal parsers ------

std::optional<int> Serializer::DeserializeIntAt(const std::string& data,
                                                size_t& pos) const {
    if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
    if (data[pos] != 'i' || data[pos + 1] != ':') return std::nullopt;
    pos += TAG_PREFIX_LEN;

    size_t semi = data.find(';', pos);
    if (semi == std::string::npos) return std::nullopt;

    int val = 0;
    auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + semi, val);
    if (ec != std::errc{} || ptr != data.data() + semi) return std::nullopt;
    pos = semi + 1;

    return val;
}

std::optional<double> Serializer::DeserializeDoubleAt(const std::string& data,
                                                      size_t& pos) const {
    if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
    if (data[pos] != 'd' || data[pos + 1] != ':') return std::nullopt;
    pos += TAG_PREFIX_LEN;

    size_t semi = data.find(';', pos);
    if (semi == std::string::npos) return std::nullopt;

    // strtod_l with C locale is locale-independent (no comma decimal separator)
    const char* start = data.data() + pos;
    char* end = nullptr;
    double val = strtod_l(start, &end, LC_C_LOCALE);
    if (end == start || end != data.data() + semi)
        return std::nullopt;
    pos = semi + 1;

    return val;
}

std::optional<bool> Serializer::DeserializeBoolAt(const std::string& data,
                                                  size_t& pos) const {
    if (pos + BOOL_TOKEN_LEN > data.size()) return std::nullopt;
    if (data[pos] != 'b' || data[pos + 1] != ':') return std::nullopt;

    char val = data[pos + 2];
    if (val != '0' && val != '1') return std::nullopt;
    if (data[pos + 3] != ';') return std::nullopt;

    pos += BOOL_TOKEN_LEN;
    return (val == '1');
}

std::optional<char> Serializer::DeserializeCharAt(const std::string& data,
                                                  size_t& pos) const {
    if (pos + CHAR_TOKEN_LEN > data.size()) return std::nullopt;
    if (data[pos] != 'c' || data[pos + 1] != ':') return std::nullopt;
    if (data[pos + 3] != ';') return std::nullopt;

    char val = data[pos + 2];
    pos += CHAR_TOKEN_LEN;
    return val;
}

std::optional<std::string> Serializer::DeserializeStringAt(const std::string& data,
                                                           size_t& pos) const {
    if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
    if (data[pos] != 's' || data[pos + 1] != ':') return std::nullopt;
    pos += TAG_PREFIX_LEN;

    // find the length value before the next colon
    size_t colon = data.find(':', pos);
    if (colon == std::string::npos) return std::nullopt;

    size_t len = 0;
    auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + colon, len);
    if (ec != std::errc{} || ptr != data.data() + colon) return std::nullopt;

    pos = colon + 1;

    // check we have enough data left (subtraction avoids overflow)
    if (len >= data.size() - pos) return std::nullopt;

    std::string val = data.substr(pos, len);
    pos += len;

    if (data[pos] != ';') return std::nullopt;
    pos += 1;

    return val;
}

std::optional<long long> Serializer::DeserializeLongLongAt(const std::string& data,
                                                          size_t& pos) const {
    if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
    if (data[pos] != 'l' || data[pos + 1] != ':') return std::nullopt;
    pos += TAG_PREFIX_LEN;

    size_t semi = data.find(';', pos);
    if (semi == std::string::npos) return std::nullopt;

    long long val = 0;
    auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + semi, val);
    if (ec != std::errc{} || ptr != data.data() + semi) return std::nullopt;
    pos = semi + 1;

    return val;
}

std::optional<unsigned long long> Serializer::DeserializeUnsignedLongLongAt(const std::string& data,
                                                                            size_t& pos) const {
    if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
    if (data[pos] != 'l' || data[pos + 1] != ':') return std::nullopt;
    pos += TAG_PREFIX_LEN;

    size_t semi = data.find(';', pos);
    if (semi == std::string::npos) return std::nullopt;

    unsigned long long val = 0;
    auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + semi, val);
    if (ec != std::errc{} || ptr != data.data() + semi) return std::nullopt;
    pos = semi + 1;

    return val;
}

std::optional<float> Serializer::DeserializeFloatAt(const std::string& data,
                                                    size_t& pos) const {
    auto r = DeserializeDoubleAt(data, pos);
    if (!r) return std::nullopt;
    return static_cast<float>(*r);
}

} // namespace cse498
