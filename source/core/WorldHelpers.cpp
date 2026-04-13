#include "WorldHelpers.hpp"
#include "../tools/Serializer.hpp"

namespace cse498 {

namespace {


std::string EncodeLocation(const Location& loc) {
    Serializer s;
    std::string result;

    if (loc.IsPosition()) {
        result += s.Serialize(std::string("P"));
        result += s.Serialize(loc.AsWorldPosition().X());
        result += s.Serialize(loc.AsWorldPosition().Y());

    } else if (loc.IsItemID()) {
        result += s.Serialize(std::string("I"));
        result += s.Serialize(static_cast<unsigned long>(loc.AsItemID()));

    } else if (loc.IsAgentID()) {
        result += s.Serialize(std::string("A"));
        result += s.Serialize(static_cast<unsigned long>(loc.AsAgentID()));
    }

    return result;
}


std::optional<Location> DecodeLocationAt(const std::string& data, size_t& pos) {
    Serializer s;
    auto tag = s.DeserializeAt<std::string>(data, pos);
    if (!tag) return std::nullopt;

    if (*tag == "P") {
        auto x = s.DeserializeAt<double>(data, pos);
        auto y = s.DeserializeAt<double>(data, pos);
        if (!x || !y) return std::nullopt;
        return Location(WorldPosition(*x, *y));

    } else if (*tag == "I") {
        auto id = s.DeserializeAt<unsigned long>(data, pos);
        if (!id) return std::nullopt;
        return Location(ItemID{static_cast<size_t>(*id)});

    } else if (*tag == "A") {
        auto id = s.DeserializeAt<unsigned long>(data, pos);
        if (!id) return std::nullopt;
        return Location(AgentID{static_cast<size_t>(*id)});
    }

    return std::nullopt;
}

std::string BuildKey(const std::string& world_name, const std::string& suffix) {
    return "world:" + world_name + ":" + suffix;
}

} // namespace

std::expected<void, DatabaseError> SaveWorld(Database& db, const std::string& world_name, const WorldBase& world) {

    Serializer s;

    // the data we want to save
    //completely up for change later just an idea of what to save
    {
        std::string data;
        data += s.Serialize(static_cast<unsigned long>(world.GetNumAgents()));
        data += s.Serialize(static_cast<unsigned long>(world.GetNumItems()));
        data += s.Serialize(world.IsRunOver());
        auto result = db.Store(BuildKey(world_name, "meta"), data);
        if (!result) return std::unexpected(result.error());
    }

    // what we want to save for the grid. once again up for debate
    {
        const auto& grid = world.GetGrid();
        std::string data;
        data += s.Serialize(static_cast<unsigned long>(grid.GetWidth()));
        data += s.Serialize(static_cast<unsigned long>(grid.GetHeight()));
        
        for (size_t y = 0; y < grid.GetHeight(); ++y) {
            for (size_t x = 0; x < grid.GetWidth(); ++x) {
                data += s.Serialize(static_cast<unsigned long>(grid[x, y]));
            }
        }
        auto result = db.Store(BuildKey(world_name, "grid"), data);
        if (!result) return std::unexpected(result.error());
    }

    // saving agent data
    for (size_t i = 0; i < world.GetNumAgents(); ++i) {
        const auto& agent = world.GetAgent(i);

        std::string data;
        data += s.Serialize(agent.GetName());
        data += EncodeLocation(agent.GetLocation());
        data += s.Serialize(agent.GetSymbol());

        auto result = db.Store(BuildKey(world_name, "agent:" + std::to_string(i)), data);
        if (!result) return std::unexpected(result.error());
    }

    // item info
    for (size_t i = 0; i < world.GetNumItems(); ++i) {
        const auto& item = world.GetItem(i);

        std::string data;
        data += s.Serialize(item.GetName());
        data += EncodeLocation(item.GetLocation());

        auto result = db.Store(BuildKey(world_name, "item:" + std::to_string(i)), data);
        if (!result) return std::unexpected(result.error());
    }

    return {};
}

std::expected<void, DatabaseError> LoadWorld(Database& db, const std::string& world_name, WorldBase& world) {
    Serializer s;

    // validate metadata count 
    {
        auto data = db.Load<std::string>(BuildKey(world_name, "meta"));
        if (!data) return std::unexpected(data.error());

        size_t pos = 0;
        auto agent_count = s.DeserializeAt<unsigned long>(*data, pos);
        auto item_count = s.DeserializeAt<unsigned long>(*data, pos);
        auto run_over = s.DeserializeAt<bool>(*data, pos);

        if (!agent_count || !item_count || !run_over) {
            return std::unexpected(DatabaseError::DeserializationFailed);
        }
            

        if (static_cast<size_t>(*agent_count) != world.GetNumAgents() || static_cast<size_t>(*item_count) != world.GetNumItems()) {
            return std::unexpected(DatabaseError::InvalidData);
        }
            

    }

    // grid loading
    {
        auto data = db.Load<std::string>(BuildKey(world_name, "grid"));
        if (!data) return std::unexpected(data.error());

        size_t pos = 0;
        auto width = s.DeserializeAt<unsigned long>(*data, pos);
        auto height = s.DeserializeAt<unsigned long>(*data, pos);

        if (!width || !height) {
            return std::unexpected(DatabaseError::DeserializationFailed);
        }
            

        auto& grid = world.GetGrid();
        grid.Resize(static_cast<size_t>(*width), static_cast<size_t>(*height));

        for (size_t y = 0; y < *height; ++y) {
            for (size_t x = 0; x < *width; ++x) {
                auto cell = s.DeserializeAt<unsigned long>(*data, pos);
                if (!cell) return std::unexpected(DatabaseError::DeserializationFailed);
                grid[x, y] = static_cast<size_t>(*cell);
            }
        }
    }

    // agents
    for (size_t i = 0; i < world.GetNumAgents(); ++i) {
        auto data = db.Load<std::string>(BuildKey(world_name, "agent:" + std::to_string(i)));
        if (!data) return std::unexpected(data.error());

        size_t pos = 0;
        auto name = s.DeserializeAt<std::string>(*data, pos);
        if (!name) return std::unexpected(DatabaseError::DeserializationFailed);

        auto loc = DecodeLocationAt(*data, pos);
        if (!loc) return std::unexpected(DatabaseError::DeserializationFailed);

        auto symbol = s.DeserializeAt<char>(*data, pos);
        if (!symbol) return std::unexpected(DatabaseError::DeserializationFailed);

        auto& agent = world.GetAgent(i);
        agent.SetName(*name);
        agent.SetLocation(*loc);
        agent.SetSymbol(*symbol);
    }

    //items
    for (size_t i = 0; i < world.GetNumItems(); ++i) {
        auto data = db.Load<std::string>(BuildKey(world_name, "item:" + std::to_string(i)));
        if (!data) return std::unexpected(data.error());

        size_t pos = 0;
        auto name = s.DeserializeAt<std::string>(*data, pos);
        if (!name) return std::unexpected(DatabaseError::DeserializationFailed);

        auto loc = DecodeLocationAt(*data, pos);
        if (!loc) return std::unexpected(DatabaseError::DeserializationFailed);

        auto& item = world.GetItem(i);
        item.SetName(*name);
        item.SetLocation(*loc);
    }

    return {};
}

} // namespace cse498
