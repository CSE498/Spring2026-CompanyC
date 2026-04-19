#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace cse498 {

/*
 * FunctionSet
 *
 * A container for callable objects that all share the same signature
 * RETURN_T(Args...). Internally stores callables using std::function,
 * allowing support for:
 *   - Lambdas
 *   - Function pointers
 *   - Functors (objects with operator())
 *   - std::bind expressions
 *
 * Behavior:
 *   - CallAll(...) invokes each stored function in insertion order.
 *   - If RETURN_T is void:
 *       -> All functions are called; no value is returned.
 *   - If RETURN_T is non-void:
 *       -> Returns a std::vector<RETURN_T> containing results from each call.
 *
 * Constraints / Notes:
 *   - All stored callables must be constructible as std::function<RETURN_T(Args...)>.
 *   - Null (empty) std::function objects are NOT allowed; Add() will throw.
 *   - Arguments are passed identically to each stored function.
 *   - rvalue references should not be relied on, since arguments are reused
 *     across multiple function calls.
 */

template <typename Signature>
class FunctionSet;

template <typename RETURN_T, typename... Args>
class FunctionSet<RETURN_T(Args...)> {
public:
  using FunctionType = std::function<RETURN_T(Args...)>;

  FunctionSet() = default;

  /*
   * Add
   *
   * Adds a callable object to the FunctionSet.
   *
   * Requirements:
   *   - F must be constructible as FunctionType.
   *
   * Behavior:
   *   - Stores the callable internally.
   *   - Throws std::invalid_argument if the callable is empty/null.
   */
  template <typename F>
    requires std::constructible_from<FunctionType, F>
  void Add(F&& fn) {
    FunctionType wrapped = std::forward<F>(fn);
    if (!wrapped) {
      throw std::invalid_argument("Cannot add null function to FunctionSet");
    }
    functions_.emplace_back(std::move(wrapped));
  }

  /*
   * Clear
   *
   * Removes all stored functions.
   */
  void Clear() noexcept {
    functions_.clear();
  }

  /*
   * CallAll
   *
   * Calls every stored function with the provided arguments.
   *
   * Behavior:
   *   - Functions are invoked in insertion order.
   *
   * Return:
   *   - If RETURN_T is void:
   *       -> No return value.
   *   - If RETURN_T is non-void:
   *       -> Returns std::vector<RETURN_T> containing results from each call.
   */
  auto CallAll(Args... args) const {
    if constexpr (std::is_void_v<RETURN_T>) {
      for (const auto& fn : functions_) {
        fn(args...);
      }
    } else {
      std::vector<RETURN_T> results;
      results.reserve(functions_.size());
      for (const auto& fn : functions_) {
        results.push_back(fn(args...));
      }
      return results;
    }
  }

  /*
   * Empty
   *
   * Returns true if no functions are stored.
   */
  [[nodiscard]] bool Empty() const noexcept {
    return functions_.empty();
  }

  /*
   * Size
   *
   * Returns the number of stored functions.
   */
  [[nodiscard]] std::size_t Size() const noexcept {
    return functions_.size();
  }

private:
  /*
   * functions_
   *
   * Internal storage for all callable objects added to the FunctionSet.
   * Each element is a std::function<RETURN_T(Args...)>, allowing uniform
   * storage of heterogeneous callables (e.g., lambdas, function pointers,
   * functors, bind expressions).
   *
   * Order is preserved based on insertion; CallAll() invokes functions_
   * sequentially in this order.
   */
  std::vector<FunctionType> functions_;
};

}  // namespace cse498
