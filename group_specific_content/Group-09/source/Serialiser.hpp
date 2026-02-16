#pragma once

#include <string>
#include <optional>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cstdlib>

class SerializerBase {

public:

    // --- Serialize overloads ---

    std::string Serialize(int value) const {
        return "i:" + std::to_string(value) + ";";
    }

    std::string Serialize(double value) const {
        std::ostringstream oss;
        oss << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
        return "d:" + oss.str() + ";";
    }

    std::string Serialize(bool value) const {
        return std::string("b:") + (value ? "1" : "0") + ";";
    }

    std::string Serialize(char value) const {
        return std::string("c:") + value + ";";
    }

    // prevents "hello" from matching the bool overload
    std::string Serialize(const char* value) const {
        return Serialize(std::string(value));
    }

    std::string Serialize(const std::string& value) const {
        return "s:" + std::to_string(value.size()) + ":" + value + ";";
    }

    // --- Deserialize wrappers ---

    std::optional<int> DeserializeInt(const std::string& data) const {
        size_t pos = 0;
        return DeserializeIntAt(data, pos);
    }

    std::optional<double> DeserializeDouble(const std::string& data) const {
        size_t pos = 0;
        return DeserializeDoubleAt(data, pos);
    }

    std::optional<bool> DeserializeBool(const std::string& data) const {
        size_t pos = 0;
        return DeserializeBoolAt(data, pos);
    }

    std::optional<char> DeserializeChar(const std::string& data) const {
        size_t pos = 0;
        return DeserializeCharAt(data, pos);
    }

    std::optional<std::string> DeserializeString(const std::string& data) const {
        size_t pos = 0;
        return DeserializeStringAt(data, pos);
    }

    // generic positional deserialize — advances pos past the consumed element
    template <typename T>
    std::optional<T> DeserializeAt(const std::string& data, size_t& pos) const {
        if constexpr (std::is_same_v<T, int>)
            return DeserializeIntAt(data, pos);
        else if constexpr (std::is_same_v<T, double>)
            return DeserializeDoubleAt(data, pos);
        else if constexpr (std::is_same_v<T, bool>)
            return DeserializeBoolAt(data, pos);
        else if constexpr (std::is_same_v<T, char>)
            return DeserializeCharAt(data, pos);
        else if constexpr (std::is_same_v<T, std::string>)
            return DeserializeStringAt(data, pos);
        else
            static_assert(!std::is_same_v<T, T>, "Unsupported type");
    }

private:

    std::optional<int> DeserializeIntAt(const std::string& data, size_t& pos) const {
        if (pos + 2 >= data.size()) return std::nullopt;
        if (data[pos] != 'i' || data[pos + 1] != ':') return std::nullopt;
        pos += 2;

        size_t semi = data.find(';', pos);
        if (semi == std::string::npos) return std::nullopt;

        std::string numStr = data.substr(pos, semi - pos);
        pos = semi + 1;

        try {
            size_t idx = 0;
            int val = std::stoi(numStr, &idx);
            if (idx != numStr.size()) return std::nullopt;
            return val;
        } catch (...) {
            return std::nullopt;
        }
    }

    std::optional<double> DeserializeDoubleAt(const std::string& data, size_t& pos) const {
        if (pos + 2 >= data.size()) return std::nullopt;
        if (data[pos] != 'd' || data[pos + 1] != ':') return std::nullopt;
        pos += 2;

        size_t semi = data.find(';', pos);
        if (semi == std::string::npos) return std::nullopt;

        std::string numStr = data.substr(pos, semi - pos);
        pos = semi + 1;

        const char* start = numStr.c_str();
        char* end = nullptr;
        double val = std::strtod(start, &end);
        if (end == start || static_cast<size_t>(end - start) != numStr.size())
            return std::nullopt;
        return val;
    }

    std::optional<bool> DeserializeBoolAt(const std::string& data, size_t& pos) const {
        if (pos + 4 > data.size()) return std::nullopt;
        if (data[pos] != 'b' || data[pos + 1] != ':') return std::nullopt;

        char val = data[pos + 2];
        if (val != '0' && val != '1') return std::nullopt;
        if (data[pos + 3] != ';') return std::nullopt;

        pos += 4;
        return (val == '1');
    }

    std::optional<char> DeserializeCharAt(const std::string& data, size_t& pos) const {
        if (pos + 4 > data.size()) return std::nullopt;
        if (data[pos] != 'c' || data[pos + 1] != ':') return std::nullopt;
        if (data[pos + 3] != ';') return std::nullopt;

        char val = data[pos + 2];
        pos += 4;
        return val;
    }

    std::optional<std::string> DeserializeStringAt(const std::string& data, size_t& pos) const {
        if (pos + 2 >= data.size()) return std::nullopt;
        if (data[pos] != 's' || data[pos + 1] != ':') return std::nullopt;
        pos += 2;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return std::nullopt;

        size_t len = 0;
        try {
            size_t idx = 0;
            len = std::stoul(data.substr(pos, colon - pos), &idx);
            if (idx != colon - pos) return std::nullopt;
        } catch (...) {
            return std::nullopt;
        }

        pos = colon + 1;

        if (pos + len + 1 > data.size()) return std::nullopt;

        std::string val = data.substr(pos, len);
        pos += len;

        if (data[pos] != ';') return std::nullopt;
        pos += 1;

        return val;
    }
};