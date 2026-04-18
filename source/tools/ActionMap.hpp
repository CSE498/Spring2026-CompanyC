/*
 * ActionMap header file Arnav Deol
 */

#pragma once

// #include <__expected/expected.h>
#include <any>
#include <cassert>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @class ActionMap
 * @brief A string-to-function registry that enables dynamic function triggering
 * at runtime.
 *
 * ActionMap maps string identifiers to callable functions, allowing systems
 * to trigger actions by name rather than hardcoded function calls. This is
 * useful for building commands, event-driven actions, and dynamic user
 * interfaces.
 */
namespace cse498 {
class ActionMap {
private:
  struct FunctionEntry {

    std::any function; // The function itself (type-erased)
    std::type_index
        type_info; // Type information of the function for validation

    FunctionEntry(std::any func, std::type_index type)
        : function(std::move(func)), type_info(type) {}
  };

  // Map of function names to strings that will be accessed by the class
  std::unordered_map<std::string, FunctionEntry> function_map;

public:
  /**
   * Default constructor & destructor
   */
  ActionMap() = default;
  ~ActionMap() = default;

  /**
   * Delete copy constructor and assignment to prevent accidental copying
   */
  ActionMap(const ActionMap &) = delete;
  ActionMap &operator=(const ActionMap &) = delete;

  /**
   * @brief Registers a function that takes arguments
   * @tparam Args The argument types for the function
   * @param name The string identifier for the function
   * @param func The function to register
   * @return true if successfully added, false if name already exists
   */
  template <typename... Args>
  std::expected<void, std::string>
  AddFunction(const std::string &name, std::function<void(Args...)> func);

  /**
   * @brief Registers a function with a given name, replacing if it already
   * exists
   * @tparam Args The argument types for the function
   * @param name The string identifier for the function
   * @param func The function to register
   */
  template <typename... Args>
  std::expected<void, std::string>
  ReplaceFunction(const std::string &name, std::function<void(Args...)> func);

  /**
   * @brief Looks up and executes a function, forwarding the provided arguments
   * @tparam Args The argument types to forward
   * @param name The string identifier of the function to trigger
   * @param args The arguments to pass to the function
   * @return std::expected<void, std::string>
   *         - On success: contains no value (void)
   *         - On failure: contains an error message explaining the issue
   */
  template <typename... Args>
  std::expected<void, std::string> Trigger(const std::string &name,
                                           Args &&...args);

  /**
   * @brief Unregisters a function by name
   * @param name The string identifier of the function to remove
   * @return true if found and removed, false if not found
   */
  bool RemoveFunction(const std::string &name);

  /**
   * @brief Checks if a function is registered under the given name
   * @param name The string identifier to check
   * @return true if function exists, false otherwise
   */
  bool HasFunction(const std::string &name) const;

  /**
   * @brief Returns a vector of all registered function names
   * @return Vector of function name strings
   */
  std::vector<std::string> GetFunctionNames() const;

  /**
   * @brief Removes all registered functions
   */
  void Clear();

  /**
   * @brief Returns a size_t of the number of registered functions
   */
  size_t Count() const;
};

// Template Implementations
template <typename... Args>
std::expected<void, std::string>
ActionMap::AddFunction(const std::string &name,
                       std::function<void(Args...)> func) {
  if (name.empty()) {
    return std::unexpected("Function name cannot be empty");
  }

  if (HasFunction(name)) {
    return std::unexpected("Function '" + name + "' already exists");
  }

  function_map.emplace(
      name,
      FunctionEntry(std::any(std::move(func)),
                    std::type_index(typeid(std::function<void(Args...)>))));

  return {};
}

template <typename... Args>
std::expected<void, std::string>
ActionMap::ReplaceFunction(const std::string &name,
                           std::function<void(Args...)> func) {
  if (name.empty()) {
    return std::unexpected("Function name cannot be empty");
  }

  function_map.erase(name);
  function_map.emplace(
      name,
      FunctionEntry(std::any(std::move(func)),
                    std::type_index(typeid(std::function<void(Args...)>))));

  return {};
}
template <typename... Args>
std::expected<void, std::string> ActionMap::Trigger(const std::string &name,
                                                    Args &&...args) {
  auto it = function_map.find(name);
  if (it == function_map.end()) {
    return std::unexpected("Function '" + name + "' not found in ActionMap");
  }

  std::type_index expected_type(typeid(std::function<void(Args...)>));
  if (it->second.type_info != expected_type) {
    return std::unexpected("Type mismatch for function '" + name +
                           "': incorrect argument types");
  }

  auto &func =
      std::any_cast<std::function<void(Args...)> &>(it->second.function);

  try {
    func(std::forward<Args>(args)...);
  } catch (const std::exception &e) {
    return std::unexpected("Function '" + name +
                           "' threw exception: " + std::string(e.what()));
  } catch (...) {
    return std::unexpected("Function '" + name +
                           "' threw an unknown exception");
  }

  return {};
}

} // namespace cse498
