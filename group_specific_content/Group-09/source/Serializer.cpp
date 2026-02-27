// main.cpp — CSE 498 Group 9, Serializer usage demo

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Serializer.hpp"

using cse498::Serializer;

struct Agent {
    std::string name;
    double health;
    int level;
};

int main() {
    Serializer s;

    // Serialize some built-in values
    std::string data;

    data = s.Serialize(42);
    std::cout << "int 42       -> " << data << "\n";

    data = s.Serialize(3.14);
    std::cout << "double 3.14  -> " << data << "\n";

    data = s.Serialize(true);
    std::cout << "bool true    -> " << data << "\n";

    data = s.Serialize('Z');
    std::cout << "char 'Z'     -> " << data << "\n";

    data = s.Serialize(std::string("hello world"));
    std::cout << "string       -> " << data << "\n";

    // Claude AI assistance used for container and custom type serialisation

    // Round-trip a vector
    std::vector<int> nums = {10, 20, 30};
    data = s.Serialize(nums);
    std::cout << "vector<int>  -> " << data << "\n";

    auto restored = s.DeserializeVector<int>(data);
    if (restored) {
        std::cout << "  restored:  ";
        for (int n : *restored) std::cout << n << " ";
        std::cout << "\n";
    }

    // Round-trip a map
    std::map<std::string, int> scores = {{"alice", 95}, {"bob", 87}};
    data = s.Serialize(scores);
    std::cout << "map          -> " << data << "\n";

    auto restored_map = s.DeserializeMap<std::string, int>(data);
    if (restored_map) {
        std::cout << "  restored:  ";
        for (const auto& [k, v] : *restored_map)
            std::cout << k << "=" << v << " ";
        std::cout << "\n";
    }

    // ---- Custom type: Agent ----
    std::cout << "\n--- Custom type demo ---\n";

    s.RegisterType<Agent>("Agent",
        [&s](const Agent& a) -> std::string {
            return s.Serialize(a.name) + s.Serialize(a.health) + s.Serialize(a.level);
        },
        [&s](const std::string& data) -> std::optional<Agent> {
            Agent a;
            size_t pos = 0;
            auto name = s.DeserializeAt<std::string>(data, pos);
            if (!name) return std::nullopt;
            auto health = s.DeserializeAt<double>(data, pos);
            if (!health) return std::nullopt;
            auto level = s.DeserializeAt<int>(data, pos);
            if (!level) return std::nullopt;
            a.name = *name;
            a.health = *health;
            a.level = *level;
            return a;
        }
    );

    Agent agent{"Steve", 100.0, 7};
    data = s.Serialize<Agent>("Agent", agent);
    std::cout << "Agent        -> " << data << "\n";

    auto restored_agent = s.Deserialize<Agent>("Agent", data);
    if (restored_agent) {
        std::cout << "  restored:  name=" << restored_agent->name
                  << " health=" << restored_agent->health
                  << " level=" << restored_agent->level << "\n";
    }

    return 0;
}
