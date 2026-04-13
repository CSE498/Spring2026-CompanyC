#pragma once

#include <unordered_map>
#include "../core/WorldPosition.hpp"

namespace cse498 {

struct TileKnowledge {
    bool discovered = false;
    bool currently_visible = false;
    bool walkable_known = false;
    bool is_walkable = false;
    bool has_enemy = false;
    bool has_resource = false;
    int last_seen_turn = -1;
};

struct WorldPositionHash {
    std::size_t operator()(const WorldPosition& p) const {
        return std::hash<int>()(p.CellX()) ^
               (std::hash<int>()(p.CellY()) << 1);
    }
};

class SharedKnowledge {
public:
    std::unordered_map<WorldPosition, TileKnowledge, WorldPositionHash> tiles;

    TileKnowledge& GetTile(const WorldPosition& pos) {
        return tiles[pos]; // auto-creates if missing
    }

    bool IsDiscovered(const WorldPosition& pos) const {
        auto it = tiles.find(pos);
        return it != tiles.end() && it->second.discovered;
    }
};

} // namespace cse498