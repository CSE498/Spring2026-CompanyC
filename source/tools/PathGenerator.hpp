/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project. *
 * @brief A base path finding interface for all agent types. * @author Matthew Vazquez
 * AI was used in assistance of developing this class
**/

// Compile using at least -std=c++20

// Take a start position, optional goal(s), and constraints
// Run a pathfinding or path-constructing strategy
// Output a WorldPath

#pragma once

#include "WorldPath.hpp"
#include "StateGridPosition.hpp"
#include "SharedKnowledge.hpp"


#include <functional>
#include <optional>
#include <unordered_set>
#include <vector>

namespace cse498 {

// Multiple Path Types
enum class PathType {
    Shortest,
    Patrol,
    Avoid,
    Explore
};

// Hash for unordered containers
struct StateGridPositionHash {
    std::size_t operator()(const StateGridPosition& p) const {
        return std::hash<int>()(p.GetX()) ^
              (std::hash<int>()(p.GetY()) << 1);
    }
};

// Sample World View
class WorldView {
public:
    WorldView(int w, int h) : width(w), height(h) {}


    bool IsWalkable(const StateGridPosition& p) const {
        if (p.GetX() < 0 || p.GetX() >= width ||
            p.GetY() < 0 || p.GetY() >= height)
            return false;

        return blocked_positions.find(p) == blocked_positions.end();
    }

        // Mark a cell as blocked
    void SetBlocked(const StateGridPosition& p) {
        blocked_positions.insert(p);
    }

    // Mark a cell as walkable again
    void SetUnBlocked(const StateGridPosition& p) {
        blocked_positions.erase(p);
    }
private:
    int width = 0;
    int height = 0;
    std::unordered_set<StateGridPosition, StateGridPositionHash> blocked_positions;
};

// Path Request API
struct PathRequest {
    PathType type;
    StateGridPosition start;
    StateGridPosition goal;
    std::optional<int> max_length;
    std::vector<StateGridPosition> avoid;
};

class PathGenerator {
public:
    void SetWorldView(const WorldView& world);

    WorldPath GeneratePath(const PathRequest& req) const;

    WorldPath GenerateShortestPath(
        StateGridPosition start,
        StateGridPosition goal
    ) const;

    WorldPath GenerateExplorePath(
        StateGridPosition start,
        const SharedKnowledge& knowledge,
        std::optional<int> max_length
    ) const;

    WorldPath GenerateResourcePath(
    StateGridPosition start,
    const SharedKnowledge& knowledge,
    std::optional<int> max_length = std::nullopt
    ) const;

    WorldPath GeneratePathToKnownTile(
    StateGridPosition start,
    StateGridPosition goal,
    const SharedKnowledge& knowledge,
    std::optional<int> max_length = std::nullopt
    ) const;

    WorldPath GenerateExplorePath(
    StateGridPosition start,
    const SharedKnowledge& knowledge,
    int world_width,
    int world_height,
    std::optional<int> max_length
    ) const;

private:
    std::optional<std::reference_wrapper<const WorldView>> world_view;

    static Point ToDoublePoint(const StateGridPosition& p);
};

}
