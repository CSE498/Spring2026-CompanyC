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
//   long:   l:<value>;
//   vector: v:<size>:<elements>
//   map:           m:<size>:<kv pairs>
//   unordered_map: u:<size>:<kv pairs>   (added as requested by other teams)
//   custom:        custom:<type_id>:<len>:<content>;
//   variant:       t:<active_index>:<serialized_value>
// Claude assistance used for custom vector and map serialisation

#include <string>
#include <optional>
#include <vector>
#include <map>
#include <variant>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <any>
#include <cassert>
#include <charconv>
#include <limits>

namespace cse498 {

class Serializer {

    // helpers to figure out if a template param is a vector or map
    template <typename T> struct is_vector : std::false_type {};
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : std::true_type {};

    template <typename T> struct is_map : std::false_type {};
    template <typename K, typename V, typename C, typename A>
    struct is_map<std::map<K, V, C, A>> : std::true_type {};

    template <typename T> struct is_unordered_map : std::false_type {};
    template <typename K, typename V, typename H, typename E, typename A>
    struct is_unordered_map<std::unordered_map<K, V, H, E, A>> : std::true_type {};

    template <typename T> struct is_variant : std::false_type {};
    template <typename... Ts>
    struct is_variant<std::variant<Ts...>> : std::true_type {};

    // each registered custom type gets a pair of type-erased functions
    struct TypeEntry {
        std::function<std::string(const void*)> serialize_fn;
        std::function<std::any(const std::string&)> deserialize_fn;
    };

    std::unordered_map<std::string, TypeEntry> registry_;

    // named constants for fixed-size token lengths
    static constexpr size_t TAG_PREFIX_LEN = 2;  // "i:", "d:", "s:", "v:", "m:", "c:", "b:", "l:"
    static constexpr size_t BOOL_TOKEN_LEN = 4;  // "b:0;" or "b:1;"
    static constexpr size_t CHAR_TOKEN_LEN = 4;  // "c:X;"

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

    bool IsTypeRegistered(const std::string& type_id) const;

    // ------ serialize primitives + containers ------

    std::string Serialize(int value) const;
    std::string Serialize(double value) const;
    std::string Serialize(bool value) const;
    std::string Serialize(char value) const;
    std::string Serialize(const char* value) const;
    std::string Serialize(const std::string& value) const;
    std::string Serialize(long long value) const;
    std::string Serialize(unsigned long long value) const;
    std::string Serialize(float value) const;
    std::string Serialize(long value) const;
    std::string Serialize(unsigned int value) const;
    std::string Serialize(unsigned long value) const;

    template <typename T>
    std::string Serialize(const std::vector<T>& vec) const {
        std::string result = "v:" + std::to_string(vec.size()) + ":";
        for (const auto& elem : vec) {
            result += Serialize(elem);
        }
        return result;
    }

    template <typename K, typename V>
    std::string Serialize(const std::map<K, V>& m) const {
        std::string result = "m:" + std::to_string(m.size()) + ":";
        for (const auto& [key, val] : m) {
            result += Serialize(key);
            result += Serialize(val);
        }
        return result;
    }

    // added as requested by other teams
    template <typename K, typename V>
    std::string Serialize(const std::unordered_map<K, V>& m) const {
        std::string result = "u:" + std::to_string(m.size()) + ":";
        for (const auto& [key, val] : m) {
            result += Serialize(key);
            result += Serialize(val);
        }
        return result;
    }

    template <typename... Ts>
    std::string Serialize(const std::variant<Ts...>& v) const {
        std::string prefix = "t:" + std::to_string(v.index()) + ":";
        return std::visit([this, &prefix](const auto& val) -> std::string {
            return prefix + this->Serialize(val);
        }, v);
    }

    // ------ serialize custom types ------

    // wraps the inner content with: custom:<type_id>:<length>:<content>;
    template <typename T>
    std::string Serialize(const std::string& type_id, const T& value) const {
        auto it = registry_.find(type_id);
        assert(it != registry_.end());
        if (it == registry_.end()) return "";
        std::string inner = it->second.serialize_fn(static_cast<const void*>(&value));
        return "custom:" + type_id + ":" + std::to_string(inner.size()) + ":" + inner + ";";
    }

    // ------ deserialize wrappers ------
    // each one just calls the internal positional parser starting at 0

    std::optional<int> DeserializeInt(const std::string& data) const;
    std::optional<double> DeserializeDouble(const std::string& data) const;
    std::optional<bool> DeserializeBool(const std::string& data) const;
    std::optional<char> DeserializeChar(const std::string& data) const;
    std::optional<std::string> DeserializeString(const std::string& data) const;
    std::optional<long long> DeserializeLongLong(const std::string& data) const;
    std::optional<unsigned long long> DeserializeUnsignedLongLong(const std::string& data) const;
    std::optional<float> DeserializeFloat(const std::string& data) const;

    template <typename T>
    std::optional<std::vector<T>> DeserializeVector(const std::string& data) const {
        size_t pos = 0;
        return DeserializeVectorAt<T>(data, pos);
    }

    template <typename K, typename V>
    std::optional<std::map<K, V>> DeserializeMap(const std::string& data) const {
        size_t pos = 0;
        return DeserializeMapAt<K, V>(data, pos);
    }

    template <typename K, typename V>
    std::optional<std::unordered_map<K, V>> DeserializeUnorderedMap(const std::string& data) const {
        size_t pos = 0;
        return DeserializeUnorderedMapAt<K, V>(data, pos);
    }

    template <typename... Ts>
    std::optional<std::variant<Ts...>> DeserializeVariant(const std::string& data) const {
        size_t pos = 0;
        return DeserializeVariantAt<std::variant<Ts...>>(data, pos);
    }

    // ------ deserialize custom types ------

    // checks the prefix, reads the length, extracts inner content,
    // then hands it to the registered deserialize function
    template <typename T>
    std::optional<T> Deserialize(const std::string& type_id, const std::string& data) const {
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
        auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + colon, len);
        if (ec != std::errc{} || ptr != data.data() + colon) return std::nullopt;

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
    std::optional<T> DeserializeAt(const std::string& data, size_t& pos) const {
        if constexpr (std::is_same_v<T, int>)
            return DeserializeIntAt(data, pos);
        else if constexpr (std::is_same_v<T, double>)
            return DeserializeDoubleAt(data, pos);
        else if constexpr (std::is_same_v<T, float>)
            return DeserializeFloatAt(data, pos);
        else if constexpr (std::is_same_v<T, bool>)
            return DeserializeBoolAt(data, pos);
        else if constexpr (std::is_same_v<T, char>)
            return DeserializeCharAt(data, pos);
        else if constexpr (std::is_same_v<T, std::string>)
            return DeserializeStringAt(data, pos);
        else if constexpr (std::is_same_v<T, long long>)
            return DeserializeLongLongAt(data, pos);
        else if constexpr (std::is_same_v<T, unsigned long long>)
            return DeserializeUnsignedLongLongAt(data, pos);
        else if constexpr (std::is_same_v<T, long>) {
            auto r = DeserializeLongLongAt(data, pos);
            if (!r) return std::nullopt;
            if constexpr (sizeof(long) < sizeof(long long)) {
                if (*r < std::numeric_limits<long>::min() || *r > std::numeric_limits<long>::max())
                    return std::nullopt;
            }
            return static_cast<long>(*r);
        }
        else if constexpr (std::is_same_v<T, unsigned int>) {
            auto r = DeserializeUnsignedLongLongAt(data, pos);
            if (!r) return std::nullopt;
            if (*r > std::numeric_limits<unsigned int>::max())
                return std::nullopt;
            return static_cast<unsigned int>(*r);
        }
        else if constexpr (std::is_same_v<T, unsigned long>) {
            auto r = DeserializeUnsignedLongLongAt(data, pos);
            if (!r) return std::nullopt;
            if constexpr (sizeof(unsigned long) < sizeof(unsigned long long)) {
                if (*r > std::numeric_limits<unsigned long>::max())
                    return std::nullopt;
            }
            return static_cast<unsigned long>(*r);
        }
        else if constexpr (is_vector<T>::value)
            return DeserializeVectorAt<typename T::value_type>(data, pos);
        else if constexpr (is_map<T>::value)
            return DeserializeMapAt<typename T::key_type, typename T::mapped_type>(data, pos);
        else if constexpr (is_unordered_map<T>::value)
            return DeserializeUnorderedMapAt<typename T::key_type, typename T::mapped_type>(data, pos);
        else if constexpr (is_variant<T>::value)
            return DeserializeVariantAt<T>(data, pos);
        else
            static_assert(!std::is_same_v<T, T>, "Unsupported type for deserialization");
    }

private:

    // ------ internal parsers ------
    // these all take a position reference and advance it past whatever
    // they consume, which is how we parse multiple values out of one string

    std::optional<int> DeserializeIntAt(const std::string& data, size_t& pos) const;
    std::optional<double> DeserializeDoubleAt(const std::string& data, size_t& pos) const;
    std::optional<bool> DeserializeBoolAt(const std::string& data, size_t& pos) const;
    std::optional<char> DeserializeCharAt(const std::string& data, size_t& pos) const;
    std::optional<std::string> DeserializeStringAt(const std::string& data, size_t& pos) const;
    std::optional<long long> DeserializeLongLongAt(const std::string& data, size_t& pos) const;
    std::optional<unsigned long long> DeserializeUnsignedLongLongAt(const std::string& data, size_t& pos) const;
    std::optional<float> DeserializeFloatAt(const std::string& data, size_t& pos) const;

    template <typename T>
    std::optional<std::vector<T>> DeserializeVectorAt(
            const std::string& data, size_t& pos) const {
        if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
        if (data[pos] != 'v' || data[pos + 1] != ':') return std::nullopt;
        pos += TAG_PREFIX_LEN;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return std::nullopt;

        size_t count = 0;
        auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + colon, count);
        if (ec != std::errc{} || ptr != data.data() + colon) return std::nullopt;

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
            const std::string& data, size_t& pos) const {
        if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
        if (data[pos] != 'm' || data[pos + 1] != ':') return std::nullopt;
        pos += TAG_PREFIX_LEN;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return std::nullopt;

        size_t count = 0;
        auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + colon, count);
        if (ec != std::errc{} || ptr != data.data() + colon) return std::nullopt;

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

    template <typename K, typename V>
    std::optional<std::unordered_map<K, V>> DeserializeUnorderedMapAt(
            const std::string& data, size_t& pos) const {
        if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
        if (data[pos] != 'u' || data[pos + 1] != ':') return std::nullopt;
        pos += TAG_PREFIX_LEN;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return std::nullopt;

        size_t count = 0;
        auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + colon, count);
        if (ec != std::errc{} || ptr != data.data() + colon) return std::nullopt;

        pos = colon + 1;

        // same sanity check as vectors/maps
        size_t remaining = data.size() - pos;
        if (count > remaining / 3 + 1) return std::nullopt;

        std::unordered_map<K, V> result;
        result.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            auto key = DeserializeAt<K>(data, pos);
            if (!key.has_value()) return std::nullopt;
            auto val = DeserializeAt<V>(data, pos);
            if (!val.has_value()) return std::nullopt;
            result[std::move(*key)] = std::move(*val);
        }

        return result;
    }

    // --- variant deserialization helpers ---

    // Recursively try each alternative at compile time until I == sizeof...(Ts)
    template <size_t I, typename Variant, typename T0, typename... Rest>
    std::optional<Variant> DeserializeVariantAlternative(
            size_t target, const std::string& data, size_t& pos) const {
        if (I == target) {
            auto val = DeserializeAt<T0>(data, pos);
            if (!val) return std::nullopt;
            return Variant(std::in_place_index<I>, std::move(*val));
        }
        if constexpr (sizeof...(Rest) > 0) {
            return DeserializeVariantAlternative<I + 1, Variant, Rest...>(target, data, pos);
        }
        return std::nullopt;
    }

    // Parse "t:<index>:" header then dispatch to the right alternative
    template <typename Variant, typename... Ts>
    std::optional<Variant> DeserializeVariantAtImpl(
            const std::string& data, size_t& pos) const {
        if (pos + TAG_PREFIX_LEN >= data.size()) return std::nullopt;
        if (data[pos] != 't' || data[pos + 1] != ':') return std::nullopt;
        size_t saved = pos;
        pos += TAG_PREFIX_LEN;

        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) { pos = saved; return std::nullopt; }

        size_t idx = 0;
        auto [ptr, ec] = std::from_chars(data.data() + pos, data.data() + colon, idx);
        if (ec != std::errc{} || ptr != data.data() + colon) { pos = saved; return std::nullopt; }

        if (idx >= sizeof...(Ts)) { pos = saved; return std::nullopt; }

        pos = colon + 1;

        auto result = DeserializeVariantAlternative<0, Variant, Ts...>(idx, data, pos);
        if (!result) { pos = saved; return std::nullopt; }
        return result;
    }

    // Unpack variant template args and call Impl
    template <typename Variant>
    struct VariantUnpacker;

    template <typename... Ts>
    struct VariantUnpacker<std::variant<Ts...>> {
        static std::optional<std::variant<Ts...>> unpack(
                const Serializer* self, const std::string& data, size_t& pos) {
            return self->template DeserializeVariantAtImpl<std::variant<Ts...>, Ts...>(data, pos);
        }
    };

    template <typename Variant>
    std::optional<Variant> DeserializeVariantAt(
            const std::string& data, size_t& pos) const {
        return VariantUnpacker<Variant>::unpack(this, data, pos);
    }
};

} // namespace cse498
