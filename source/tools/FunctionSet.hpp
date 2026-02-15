#pragma once

#include <vector>
#include <functional>
#include <utility>

/// A container that stores multiple functions
/// with identical signature and calls them sequentially.
template <typename Signature>
class FunctionSet;

template <typename R, typename... Args>
class FunctionSet<R(Args...)> {
public:
    using FunctionType = std::function<R(Args...)>;

    FunctionSet() = default;

    /// Add a function to the set.
    void Add(FunctionType fn) {
        functions_.push_back(std::move(fn));
    }

    /// Remove all functions.
    void Clear() {
        functions_.clear();
    }

    /// Call all stored functions with the same arguments.
    void CallAll(Args... args) const {
        for (const auto& fn : functions_) {
            fn(args...);
        }
    }

    /// Check if empty.
    [[nodiscard]] bool Empty() const {
        return functions_.empty();
    }

private:
    std::vector<FunctionType> functions_;
};
