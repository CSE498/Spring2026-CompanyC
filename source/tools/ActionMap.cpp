#include "ActionMap.hpp"
#include <algorithm>

bool cse498::ActionMap::RemoveFunction(const std::string &name) {
  auto it = function_map.find(name);
  if (it == function_map.end()) {
    return false;
  }

  function_map.erase(it);
  return true;
}

bool cse498::ActionMap::HasFunction(const std::string &name) const {
  return function_map.find(name) != function_map.end();
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
