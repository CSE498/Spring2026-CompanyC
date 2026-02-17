#pragma once

#include <vector>
#include <functional>
#include <utility>

/// @brief A lightweight container that stores multiple functions
///        sharing the same signature and invokes them sequentially.
///
/// FunctionSet behaves like a simple event dispatcher:
/// - You can register multiple callbacks.
/// - All callbacks must have the same function signature.
/// - Calling CallAll(...) invokes each stored function in the
///   order they were added.
///
/// Example:
///     FunctionSet<void(int)> fs;
///     fs.Add([](int x) { std::cout << x + 1; });
///     fs.Add([](int x) { std::cout << x + 2; });
///     fs.CallAll(5);  // Calls both functions with argument 5
template <typename Signature>
class FunctionSet;

template <typename R, typename... Args>
class FunctionSet<R(Args...)> {
public:
    /// @brief Alias for the stored callable type.
    ///
    /// Internally uses std::function to allow storage of:
    /// - Lambdas
    /// - Function pointers
    /// - std::bind expressions
    /// - Functors
    using FunctionType = std::function<R(Args...)>;
    
    /// Default Constructor
    FunctionSet() = default;

    /// @brief Adds a new function to the set.
    ///
    /// The function will be stored and later invoked when CallAll(...)
    /// is called. Functions are executed in insertion order.
    void Add(FunctionType fn) {
        functions_.push_back(std::move(fn));
    }

    /// @brief Removes all stored functions.
    ///
    /// After calling Clear(), CallAll(...) will do nothing
    /// until new functions are added.
    void Clear() {
        functions_.clear();
    }

    /// @brief Calls all stored functions with the provided arguments.
    ///
    /// Each function is invoked sequentially using the same argument list.
    void CallAll(Args... args) const {
        for (const auto& fn : functions_) {
            fn(args...);
        }
    }

    /// @brief Checks whether the container has any registered functions.
    ///
    /// @return true if no functions are stored, false otherwise.
    [[nodiscard]] bool Empty() const {
        return functions_.empty();
    }

private:
    /// @brief Internal storage for registered functions.
    ///
    /// Maintains insertion order
    std::vector<FunctionType> functions_;
};
