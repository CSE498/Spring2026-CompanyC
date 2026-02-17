#pragma once
// Serializer.hpp — Group 9, Database Module
//
// Text-based serialization for C++ types.
// Each type gets a tag prefix so we know what we're reading back:
//   int:    i:<value>;
//   double: d:<value>;
//   bool:   b:<0|1>;
//   char:   c:<ch>;
//   string: s:<len>:<content>;
//   vector: v:<size>:<elements>
//   map:    m:<size>:<kv pairs>
//   custom: custom:<type_id>:<len>:<content>;
// Claude assistance used for custom vector and map serialisation 

#include <string>
#include <optional>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <functional>
#include <cstdlib>
#include <unordered_map>
#include <any>

class Serializer {

    // helpers to figure out if a template param is a vector or map
    template <typename T> struct is_vector : std::false_type {};
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : std::true_type {};

    template <typename T> struct is_map : std::false_type {};
    template <typename K, typename V, typename C, typename A>
    struct is_map<std::map<K, V, C, A>> : std::true_type {};

    // each registered custom type gets a pair of type-erased functions
    struct TypeEntry {
        std::function<std::string(const void*)> serialize_fn;
        std::function<std::any(const std::string&)> deserialize_fn;
    };

    std::unordered_map<std::string, TypeEntry> registry_;

public:

    // ------ custom type registration ------

    // register a type with a name and two lambdas (serialize + deserialize)
    template <typename T>
    void RegisterType(const std::string& type_id,
                      std::function<std::string(const T&)> serialize_fn,
                      std::function<std::optional<T>(const std::string&)> deserialize_fn) {
        TypeEntry entry;
        entry.serialize_fn = [serialize_fn](const void* ptr) -> std::string {
            return serialize_fn(*static_cast<const T*>(ptr));
        };
        entry.deserialize_fn = [deserialize_fn](const std::string& data) -> std::any {
            auto result = deserialize_fn(data);
            if (result.has_value()) return std::any(std::move(*result));
            return std::any();
        };
        registry_[type_id] = std::move(entry);
    }

    bool IsTypeRegistered(const std::string& type_id) const {
        return registry_.count(type_id) > 0;
    }

    // ------ serialize primitives + containers ------

    std::string Serialize(int value) {
        return "i:" + std::to_string(value) + ";";
    }

    // max_digits10 keeps full precision through a round-trip
    std::string Serialize(double value) {
        std::ostringstream oss;
        oss << std::setprecision(std::numeric_limits<double>::max_digits10)
            << value;
        return "d:" + oss.str() + ";";
    }

    std::string Serialize(bool value) {
        return std::string("b:") + (value ? "1" : "0") + ";";
    }

    std::string Serialize(char value) {
        return std::string("c:") + value + ";";
    }

    // without this, Serialize("hello") matches the bool overload
    // because pointer-to-bool is a standard conversion in C++
    std::string Serialize(const char* value) {
        return Serialize(std::string(value));
    }

    // length prefix means semicolons/colons inside the string are fine
    std::string Serialize(const std::string& value) {
        return "s:" + std::to_string(value.size()) + ":" + value + ";";
    }

    template <typename T>
    std::string Serialize(const std::vector<T>& vec) {
        std::string result = "v:" + std::to_string(vec.size()) + ":";
        for (const auto& elem : vec) {
            result += Serialize(elem);
        }
        return result;
    }

    template <typename K, typename V>
    std::string Serialize(const std::map<K, V>& m) {
        std::string result = "m:" + std::to_string(m.size()) + ":";
        for (const auto& [key, val] : m) {
            result += Serialize(key);
            result += Serialize(val);
        }
        return result;
    }

    // ------ serialize custom types ------

    // wraps the inner content with: custom:<type_id>:<length>:<content>;
    template <typename T>
    std::string Serialize(const std::string& type_id, const T& value) {
        auto it = registry_.find(type_id);
        if (it == registry_.end()) return "";
        std::string inner = it->second.serialize_fn(static_cast<const void*>(&value));
        return "custom:" + type_id + ":" + std::to_string(inner.size()) + ":" + inner + ";";
    }

    // ------ deserialize wrappers------
    // each one just calls the internal positional parser starting at 0

    std::optional<int> DeserializeInt(const std::string& data) {
        size_t pos = 0;
        return DeserializeIntAt(data, pos);
    }

    std::optional<double> DeserializeDouble(const std::string& data) {
        size_t pos = 0;
        return DeserializeDoubleAt(data, pos);
    }

    std::optional<bool> DeserializeBool(const std::string& data) {
        size_t pos = 0;
        return DeserializeBoolAt(data, pos);
    }

    std::optional<char> DeserializeChar(const std::string& data) {
        size_t pos = 0;
        return DeserializeCharAt(data, pos);
    }

    std::optional<std::string> DeserializeString(const std::string& data) {
        size_t pos = 0;
        return DeserializeStringAt(data, pos);
    }

    template <typename T>
    std::optional<std::vector<T>> DeserializeVector(const std::string& data) {
        size_t pos = 0;
        return DeserializeVectorAt<T>(data, pos);
    }

    template <typename K, typename V>
    std::optional<std::map<K, V>> DeserializeMap(const std::string& data) {
        size_t pos = 0;
        return DeserializeMapAt<K, V>(data, pos);
    }

    // ------ deserialize custom types ------

    // checks the prefix, reads the length, extracts inner content,
    // then hands it to the registered deserialize function
    template <typename T>
    std::optional<T> Deserialize(const std::string& type_id, const std::string& data) {
        auto it = registry_.find(type_id);
        if (it == registry_.end()) return std::nullopt;

        std::string prefix = "custom:" + type_id + ":";
        if (data.size() < prefix.size() || data.compare(0, prefix.size(), prefix) != 0)
            return std::nullopt;

        size_t pos = prefix.size();

        // grab the content length between the colons
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

        if (pos + len > data.size()) return std::nullopt;
        std::string inner = data.substr(pos, len);
        pos += len;

        if (pos >= data.size() || data[pos] != ';') return std::nullopt;

        std::any result = it->second.deserialize_fn(inner);
        if (!result.has_value()) return std::nullopt;

        try {
            return std::any_cast<T>(result);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    // ------ positional deserialize ------
    // used inside custom type lambdas to read fields one after another.
    // pos gets advanced past each element so the next call picks up
    // right where the last one left off.
    template <typename T>
    std::optional<T> DeserializeAt(const std::string& data, size_t& pos) {
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
        else if constexpr (is_vector<T>::value)
            return DeserializeVectorAt<typename T::value_type>(data, pos);
        else if constexpr (is_map<T>::value)
            return DeserializeMapAt<typename T::key_type, typename T::mapped_type>(data, pos);
        else
            static_assert(!std::is_same_v<T, T>, "Unsupported type for deserialization");
    }

private:

    // ------ internal parsers ------
    // these all take a position reference and advance it past whatever
    // they consume, which is how we parse multiple values out of one string

    std::optional<int> DeserializeIntAt(const std::string& data,
                                               size_t& pos) {
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

    std::optional<double> DeserializeDoubleAt(const std::string& data,
                                                     size_t& pos) {
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

    std::optional<bool> DeserializeBoolAt(const std::string& data,
                                                 size_t& pos) {
        // always exactly 4 chars: b:0; or b:1;
        if (pos + 4 > data.size()) return std::nullopt;
        if (data[pos] != 'b' || data[pos + 1] != ':') return std::nullopt;

        char val = data[pos + 2];
        if (val != '0' && val != '1') return std::nullopt;
        if (data[pos + 3] != ';') return std::nullopt;

        pos += 4;
        return (val == '1');
    }

    std::optional<char> DeserializeCharAt(const std::string& data,
                                                 size_t& pos) {
        // always exactly 4 chars: c:X;
        if (pos + 4 > data.size()) return std::nullopt;
        if (data[pos] != 'c' || data[pos + 1] != ':') return std::nullopt;
        if (data[pos + 3] != ';') return std::nullopt;

        char val = data[pos + 2];
        pos += 4;
        return val;
    }

    std::optional<std::string> DeserializeStringAt(const std::string& data,
                                                          size_t& pos) {
        if (pos + 2 >= data.size()) return std::nullopt;
        if (data[pos] != 's' || data[pos + 1] != ':') return std::nullopt;
        pos += 2;

        // find the length value before the next colon
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

        // check we have enough data left (subtraction avoids overflow)
        if (len >= data.size() - pos) return std::nullopt;

        std::string val = data.substr(pos, len);
        pos += len;

        if (data[pos] != ';') return std::nullopt;
        pos += 1;

        return val;
    }

    template <typename T>
    std::optional<std::vector<T>> DeserializeVectorAt(
            const std::string& data, size_t& pos) {
        if (pos + 2 >= data.size()) return std::nullopt;
        if (data[pos] != 'v' || data[pos + 1] != ':') return std::nullopt;
        pos += 2;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return std::nullopt;

        size_t count = 0;
        try {
            size_t idx = 0;
            count = std::stoul(data.substr(pos, colon - pos), &idx);
            if (idx != colon - pos) return std::nullopt;
        } catch (...) {
            return std::nullopt;
        }

        pos = colon + 1;

        // reject obviously bogus counts before we try to allocate
        size_t remaining = data.size() - pos;
        if (count > remaining / 3 + 1) return std::nullopt;

        std::vector<T> result;
        result.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            auto elem = DeserializeAt<T>(data, pos);
            if (!elem.has_value()) return std::nullopt;
            result.push_back(std::move(*elem));
        }

        return result;
    }

    template <typename K, typename V>
    std::optional<std::map<K, V>> DeserializeMapAt(
            const std::string& data, size_t& pos) {
        if (pos + 2 >= data.size()) return std::nullopt;
        if (data[pos] != 'm' || data[pos + 1] != ':') return std::nullopt;
        pos += 2;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return std::nullopt;

        size_t count = 0;
        try {
            size_t idx = 0;
            count = std::stoul(data.substr(pos, colon - pos), &idx);
            if (idx != colon - pos) return std::nullopt;
        } catch (...) {
            return std::nullopt;
        }

        pos = colon + 1;

        // same sanity check as vectors
        size_t remaining = data.size() - pos;
        if (count > remaining / 3 + 1) return std::nullopt;

        std::map<K, V> result;
        for (size_t i = 0; i < count; ++i) {
            auto key = DeserializeAt<K>(data, pos);
            if (!key.has_value()) return std::nullopt;
            auto val = DeserializeAt<V>(data, pos);
            if (!val.has_value()) return std::nullopt;
            result[std::move(*key)] = std::move(*val);
        }

        return result;
    }
};
