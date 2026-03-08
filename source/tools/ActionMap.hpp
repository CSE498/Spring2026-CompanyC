/*
 * ActionMap header file Arnav Deol
 */

#pragma once

#include <any>
#include <assert.h>
#include <functional>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
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
   * @brief Registers a zero-argument function with a given name
   * @param name The string identifier for the function
   * @param func The function to register
   * @return true if successfully added, false if name already exists
   */
  bool AddFunction(const std::string &name, std::function<void()> func);

  /**
   * @brief Registers a function that takes arguments
   * @tparam Args The argument types for the function
   * @param name The string identifier for the function
   * @param func The function to register
   * @return true if successfully added, false if name already exists
   */
  template <typename... Args>
  bool AddFunction(const std::string &name, std::function<void(Args...)> func);

  /**
   * @brief Registers a function with a given name, replacing if it already
   * exists
   * @param name The string identifier for the function
   * @param func The function to register
   */
  void ReplaceFunction(const std::string &name, std::function<void()> func);

  /**
   * @brief Registers a function with a given name, replacing if it already
   * exists
   * @tparam Args The argument types for the function
   * @param name The string identifier for the function
   * @param func The function to register
   */
  template <typename... Args>
  void ReplaceFunction(const std::string &name,
                       std::function<void(Args...)> func);

  /**
   * @brief Looks up and executes the zero-argument function with the given name
   * @param name The string identifier of the function to trigger
   * @return Optional error message if function not found or type mismatch
   */
  std::optional<std::string> Trigger(const std::string &name);

  /**
   * @brief Looks up and executes a function, forwarding the provided arguments
   * @tparam Args The argument types to forward
   * @param name The string identifier of the function to trigger
   * @param args The arguments to pass to the function
   * @return Optional error message if function not found or type mismatch
   */
  template <typename... Args>
  std::optional<std::string> Trigger(const std::string &name, Args &&...args);

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
bool ActionMap::AddFunction(const std::string &name,
                            std::function<void(Args...)> func) {

  assert(!name.empty());

  if (HasFunction(name)) {
    return false;
  }

  function_map.emplace(
      name,
      FunctionEntry(std::any(std::move(func)),
                    std::type_index(typeid(std::function<void(Args...)>))));

  return true;
}

template <typename... Args>
void ActionMap::ReplaceFunction(const std::string &name,
                                std::function<void(Args...)> func) {

  assert(!name.empty());
  function_map.erase(name);
  function_map.emplace(
      name,
      FunctionEntry(std::any(std::move(func)),
                    std::type_index(typeid(std::function<void(Args...)>))));
}

// Ngl I chatted this one, will need to rewrite it to better understand passing
// the return of the function to where it needs to go.
template <typename... Args>
std::optional<std::string> ActionMap::Trigger(const std::string &name,
                                              Args &&...args) {
  // Find the function
  auto it = function_map.find(name);
  if (it == function_map.end()) {
    return "Function '" + name + "' not found in ActionMap";
  }

  // Verify type matches
  std::type_index expected_type =
      std::type_index(typeid(std::function<void(Args...)>));
  if (it->second.type_info != expected_type) {
    return "Type mismatch for function '" + name +
           "': incorrect argument types";
  }

  // Extract and call the function
  try {
    auto &func =
        std::any_cast<std::function<void(Args...)> &>(it->second.function);
    func(std::forward<Args>(args)...);
    return std::nullopt; // Success
  } catch (const std::bad_any_cast &) {
    return "Internal error: failed to cast function '" + name + "'";
  } catch (const std::exception &e) {
    return "Exception thrown by function '" + name + "': " + e.what();
  }
}
} // namespace cse498
