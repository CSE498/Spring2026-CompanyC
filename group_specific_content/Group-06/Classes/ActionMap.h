/*
* ActionMap header file Arnav Deol
*/

#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <optional>
#include <any>
#include <typeindex>


class ActionMap {
private:
    
    struct FunctionEntry {
        std::any function;
        std::type_index type_info;
        
        FunctionEntry(std::any func, std::type_index type)
            : function(std::move(func)), type_info(type) {}
    };
    
    std::unordered_map<std::string, FunctionEntry> function_map;
    
public:

    ActionMap() = default;
    ~ActionMap() = default;
    
    ActionMap(const ActionMap&) = delete;
    ActionMap& operator=(const ActionMap&) = delete;
    

    bool AddFunction(const std::string& name, std::function<void()> func);

    template<typename... Args>
    bool AddFunction(const std::string& name, std::function<void(Args...)> func);
    

    void ReplaceFunction(const std::string& name, std::function<void()> func);

    template<typename... Args>
    void ReplaceFunction(const std::string& name, std::function<void(Args...)> func);
    

    std::optional<std::string> Trigger(const std::string& name);
    
    template<typename... Args>
    std::optional<std::string> Trigger(const std::string& name, Args&&... args);
    
    bool RemoveFunction(const std::string& name);
    bool HasFunction(const std::string& name) const;
    std::vector<std::string> GetFunctionNames() const;
    void Clear();
    size_t Count() const;
};


// Template Implementations
template<typename... Args>
bool ActionMap::AddFunction(const std::string& name, std::function<void(Args...)> func) {
    
    assert(!name.empty());
    
    if (HasFunction(name)) {
        return false;
    }
    
    function_map.emplace(
        name,
        FunctionEntry(
            std::any(std::move(func)),
            std::type_index(typeid(std::function<void(Args...)>))
        )
    );
    
    return true;
}

template<typename... Args>
void ActionMap::ReplaceFunction(const std::string& name, std::function<void(Args...)> func) {
    
    assert(!name.empty());
    function_map.erase(name);
    function_map.emplace(
        name,
        FunctionEntry(
            std::any(std::move(func)),
            std::type_index(typeid(std::function<void(Args...)>))
        )
    );
}

// Ngl I chatted this one, will need to rewrite it to better understand passing the return of the function to where it needs to go.
template<typename... Args>
std::optional<std::string> ActionMap::Trigger(const std::string& name, Args&&... args) {
    // Find the function
    auto it = function_map.find(name);
    if (it == function_map.end()) {
        return "Function '" + name + "' not found in ActionMap";
    }
    
    // Verify type matches
    std::type_index expected_type = std::type_index(typeid(std::function<void(Args...)>));
    if (it->second.type_info != expected_type) {
        return "Type mismatch for function '" + name + "': incorrect argument types";
    }
    
    // Extract and call the function
    try {
        auto& func = std::any_cast<std::function<void(Args...)>&>(it->second.function);
        func(std::forward<Args>(args)...);
        return std::nullopt; // Success
    } catch (const std::bad_any_cast&) {
        return "Internal error: failed to cast function '" + name + "'";
    } catch (const std::exception& e) {
        return "Exception thrown by function '" + name + "': " + e.what();
    }
}