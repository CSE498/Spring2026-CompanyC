#include "ActionMap.h"
#include <algorithm>
#include <cassert>

bool cse498::ActionMap::AddFunction(const std::string &name,
                                    std::function<void()> func) {

  if (name.empty())
    return false;
  if (HasFunction(name)) {
    return false;
  }
  function_map.emplace(
      name, FunctionEntry(std::any(std::move(func)),
                          std::type_index(typeid(std::function<void()>))));
  return true;
}

void cse498::ActionMap::ReplaceFunction(const std::string &name,
                                        std::function<void()> func) {

  assert(!name.empty());
  function_map.erase(name);
  function_map.emplace(
      name, FunctionEntry(std::any(std::move(func)),
                          std::type_index(typeid(std::function<void()>))));
}

// Ngl I chatted this one, will need to rewrite it to better understand passing
// the return of the function to where it needs to go.
std::optional<std::string> cse498::ActionMap::Trigger(const std::string &name) {
  // Find the function
  auto it = function_map.find(name);
  if (it == function_map.end()) {
    return "Function '" + name + "' not found in ActionMap";
  }

  // Verify type matches (zero-argument function)
  std::type_index expected_type =
      std::type_index(typeid(std::function<void()>));
  if (it->second.type_info != expected_type) {
    return "Type mismatch for function '" + name + "': expected zero arguments";
  }

  // Extract and call the function
  try {
    auto &func = std::any_cast<std::function<void()> &>(it->second.function);
    func();
    return std::nullopt; // Success
  } catch (const std::bad_any_cast &) {
    return "Internal error: failed to cast function '" + name + "'";
  } catch (const std::exception &e) {
    return "Exception thrown by function '" + name + "': " + e.what();
  }

  return std::optional<std::string>();
}

bool cse498::ActionMap::RemoveFunction(const std::string &name) {
  auto it = function_map.find(name);
  if (it == function_map.end()) {
    return false;
  }
  function_map.erase(it);
  return true;
}

bool cse498::ActionMap::HasFunction(const std::string &name) const {
  return (function_map.find(name) != function_map.end());
}

std::vector<std::string> cse498::ActionMap::GetFunctionNames() const {
  std::vector<std::string> names;
  names.reserve(function_map.size());

  for (const auto &pair : function_map) {
    names.push_back(pair.first);
  }
  std::sort(names.begin(), names.end());

  return names;
}

void cse498::ActionMap::Clear() { function_map.clear(); }

size_t cse498::ActionMap::Count() const { return function_map.size(); }
