#include "PathGenerator.hpp"

#include <algorithm>
#include <ranges>
#include <queue>
#include <unordered_map>
#include <optional>
#include <functional>
#include <limits>

namespace cse498 {

namespace {

std::vector<StateGridPosition> GetNeighbors(const StateGridPosition& p)
{
    std::vector<StateGridPosition> neighbors;

    neighbors.emplace_back(p.GetX() - 1, p.GetY());
    neighbors.emplace_back(p.GetX() + 1, p.GetY());
    neighbors.emplace_back(p.GetX(), p.GetY() - 1);
    neighbors.emplace_back(p.GetX(), p.GetY() + 1);

    neighbors.emplace_back(p.GetX() - 1, p.GetY() - 1);
    neighbors.emplace_back(p.GetX() + 1, p.GetY() - 1);
    neighbors.emplace_back(p.GetX() - 1, p.GetY() + 1);
    neighbors.emplace_back(p.GetX() + 1, p.GetY() + 1);

    return neighbors;
}

template <typename GoalFn, typename TraverseFn>
std::optional<StateGridPosition> RunBFS(
    StateGridPosition start,
    GoalFn is_goal,
    TraverseFn can_traverse,
    std::unordered_map<StateGridPosition, StateGridPosition, StateGridPositionHash>& parent
) {
    if (!can_traverse(start)) {
        return std::nullopt;
    }

    std::queue<StateGridPosition> q;
    q.push(start);
    parent[start] = start;

    while (!q.empty()) {
        StateGridPosition current = q.front();
        q.pop();

        if (is_goal(current)) {
            return current;
        }

        auto neighbors = GetNeighbors(current);
        auto filtered_neighbors = std::views::filter(neighbors, can_traverse);

        for (const auto& next : filtered_neighbors) {
            if (parent.find(next) != parent.end()) continue;

            parent[next] = current;
            q.push(next);
        }
    }

    return std::nullopt;
}

WorldPath ReconstructPath(
    StateGridPosition start,
    StateGridPosition goal,
    const std::unordered_map<StateGridPosition, StateGridPosition, StateGridPositionHash>& parent,
    std::optional<int> max_length,
    const std::function<Point(const StateGridPosition&)>& to_point
) {
    std::vector<Point> points;
    StateGridPosition p = goal;

    while (!(p.GetX() == parent.at(p).GetX() && p.GetY() == parent.at(p).GetY())) {
        points.push_back(to_point(p));
        p = parent.at(p);
    }

    points.push_back(to_point(start));
    std::reverse(points.begin(), points.end());

    size_t limit = max_length.has_value() ? static_cast<size_t>(*max_length) : 1000;

    if (points.size() > limit) {
        points.resize(limit);
    }

    return WorldPath(points);
}

} // namespace

void PathGenerator::SetWorldView(const WorldView& world) {
    world_view = std::cref(world);
}

Point PathGenerator::ToDoublePoint(const StateGridPosition& p)
{
    return Point{
        static_cast<double>(p.GetX()),
        static_cast<double>(p.GetY())
    };
}

WorldPath PathGenerator::GeneratePath(const PathRequest& req) const
{
    if (!world_view.has_value()) {
        return WorldPath{};
    }

    switch (req.type)
    {
        case PathType::Shortest:
            return GenerateShortestPath(req.start, req.goal);
        case PathType::Explore:
            return WorldPath{};
        default:
            return WorldPath{};
    }
}

WorldPath PathGenerator::GenerateShortestPath(
    StateGridPosition start,
    StateGridPosition goal
) const
{
    WorldPath path;

    if (!world_view.has_value()) {
        return path;
    }

    const WorldView& w = world_view->get();

    auto can_traverse = [&](const StateGridPosition& p) {
        return w.IsWalkable(p);
    };

    auto is_goal = [&](const StateGridPosition& p) {
        return p.GetX() == goal.GetX() && p.GetY() == goal.GetY();
    };

    std::unordered_map<
        StateGridPosition,
        StateGridPosition,
        StateGridPositionHash
    > parent;

    auto found = RunBFS(start, is_goal, can_traverse, parent);
    if (!found.has_value()) {
        return path;
    }

    return ReconstructPath(
        start,
        *found,
        parent,
        std::nullopt,
        [this](const StateGridPosition& p) { return ToDoublePoint(p); }
    );
}

// old signature kept for compatibility
WorldPath PathGenerator::GenerateExplorePath(
    StateGridPosition start,
    const SharedKnowledge& knowledge,
    std::optional<int> max_length
) const
{
    // Fallback if some older caller still uses this version.
    // No global edge knowledge here, so use a neutral size.
    return GenerateExplorePath(start, knowledge, 80, 80, max_length);
}

WorldPath PathGenerator::GenerateExplorePath(
    StateGridPosition start,
    const SharedKnowledge& knowledge,
    int world_width,
    int world_height,
    std::optional<int> max_length
) const
{
    auto ToWorldPos = [](const StateGridPosition& p) {
        return WorldPosition(p.GetX(), p.GetY());
    };

    auto InBounds = [&](const StateGridPosition& p) -> bool {
        return p.GetX() >= 0 &&
               p.GetY() >= 0 &&
               p.GetX() < world_width &&
               p.GetY() < world_height;
    };

    auto IsKnownWalkable = [&](const StateGridPosition& p) -> bool {
        if (!InBounds(p)) return false;

        auto it = knowledge.tiles.find(ToWorldPos(p));
        if (it == knowledge.tiles.end()) return false;

        const TileKnowledge& tile = it->second;
        return tile.walkable_known && tile.is_walkable;
    };

    auto IsDiscovered = [&](const StateGridPosition& p) -> bool {
        if (!InBounds(p)) return false;

        auto it = knowledge.tiles.find(ToWorldPos(p));
        return it != knowledge.tiles.end() && it->second.discovered;
    };

    auto CountUnknownNeighbors5x5 = [&](const StateGridPosition& p) -> int {
        int count = 0;
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                StateGridPosition n(p.GetX() + dx, p.GetY() + dy);
                if (!InBounds(n)) continue;
                if (!IsDiscovered(n)) {
                    ++count;
                }
            }
        }
        return count;
    };

    auto IsFrontier = [&](const StateGridPosition& p) -> bool {
        if (!IsKnownWalkable(p)) return false;
        return CountUnknownNeighbors5x5(p) > 0;
    };

    auto EdgePenalty = [&](const StateGridPosition& p) -> int {
        int left   = p.GetX();
        int right  = world_width - 1 - p.GetX();
        int top    = p.GetY();
        int bottom = world_height - 1 - p.GetY();

        int min_edge_dist = std::min({left, right, top, bottom});

        if (min_edge_dist <= 1) return 50;
        if (min_edge_dist == 2) return 25;
        if (min_edge_dist == 3) return 10;
        return 0;
    };

    WorldPath path;

    if (!IsKnownWalkable(start)) {
        return path;
    }

    std::queue<StateGridPosition> q;
    std::unordered_map<StateGridPosition, StateGridPosition, StateGridPositionHash> parent;
    std::unordered_map<StateGridPosition, int, StateGridPositionHash> dist;

    q.push(start);
    parent[start] = start;
    dist[start] = 0;

    std::vector<StateGridPosition> frontier_candidates;

    while (!q.empty()) {
        StateGridPosition current = q.front();
        q.pop();

        if (!(current.GetX() == start.GetX() && current.GetY() == start.GetY()) &&
            IsFrontier(current)) {
            frontier_candidates.push_back(current);
        }

        auto neighbors = GetNeighbors(current);
        auto bfs_filter = std::views::filter(neighbors, IsKnownWalkable);

        for (const auto& next : bfs_filter) {
            if (parent.find(next) != parent.end()) continue;

            parent[next] = current;
            dist[next] = dist[current] + 1;
            q.push(next);
        }
    }

    if (frontier_candidates.empty()) {
        return path;
    }

    StateGridPosition best = frontier_candidates.front();
    int best_score = std::numeric_limits<int>::min();

    for (const auto& candidate : frontier_candidates) {
        int unknown_gain = CountUnknownNeighbors5x5(candidate);
        int distance = dist[candidate];

        int score = unknown_gain * 10 - distance - EdgePenalty(candidate);

        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }

    std::vector<Point> points;
    StateGridPosition p = best;

    while (!(p.GetX() == parent[p].GetX() && p.GetY() == parent[p].GetY())) {
        points.push_back(ToDoublePoint(p));
        p = parent[p];
    }

    points.push_back(ToDoublePoint(start));
    std::reverse(points.begin(), points.end());

    size_t limit = max_length.has_value() ? static_cast<size_t>(*max_length) : 1000;
    if (points.size() > limit) {
        points.resize(limit);
    }

    return WorldPath(points);
}

WorldPath PathGenerator::GenerateResourcePath(
    StateGridPosition start,
    const SharedKnowledge& knowledge,
    std::optional<int> max_length
) const
{
    auto ToWorldPos = [](const StateGridPosition& p) {
        return WorldPosition(p.GetX(), p.GetY());
    };

    auto IsKnownWalkable = [&](const StateGridPosition& p) -> bool {
        auto it = knowledge.tiles.find(ToWorldPos(p));
        if (it == knowledge.tiles.end()) return false;

        const TileKnowledge& tile = it->second;
        return tile.walkable_known && tile.is_walkable;
    };

    auto HasResource = [&](const StateGridPosition& p) -> bool {
        auto it = knowledge.tiles.find(ToWorldPos(p));
        if (it == knowledge.tiles.end()) return false;

        return it->second.has_resource;
    };

    auto IsResourceGoal = [&](const StateGridPosition& p) -> bool {
        return HasResource(p) &&
               !(p.GetX() == start.GetX() && p.GetY() == start.GetY());
    };

    std::unordered_map<
        StateGridPosition,
        StateGridPosition,
        StateGridPositionHash
    > parent;

    auto target = RunBFS(start, IsResourceGoal, IsKnownWalkable, parent);
    if (!target.has_value()) {
        return WorldPath{};
    }

    return ReconstructPath(
        start,
        *target,
        parent,
        max_length,
        [this](const StateGridPosition& p) { return ToDoublePoint(p); }
    );
}

WorldPath PathGenerator::GeneratePathToKnownTile(
    StateGridPosition start,
    StateGridPosition goal,
    const SharedKnowledge& knowledge,
    std::optional<int> max_length
) const
{
    auto ToWorldPos = [](const StateGridPosition& p) {
        return WorldPosition(p.GetX(), p.GetY());
    };

    auto IsKnownWalkable = [&](const StateGridPosition& p) -> bool {
        auto it = knowledge.tiles.find(ToWorldPos(p));
        if (it == knowledge.tiles.end()) return false;

        const TileKnowledge& tile = it->second;
        return tile.walkable_known && tile.is_walkable;
    };

    auto IsGoal = [&](const StateGridPosition& p) -> bool {
        return p.GetX() == goal.GetX() && p.GetY() == goal.GetY();
    };

    std::unordered_map<
        StateGridPosition,
        StateGridPosition,
        StateGridPositionHash
    > parent;

    auto target = RunBFS(start, IsGoal, IsKnownWalkable, parent);
    if (!target.has_value()) {
        return WorldPath{};
    }

    return ReconstructPath(
        start,
        *target,
        parent,
        max_length,
        [this](const StateGridPosition& p) { return ToDoublePoint(p); }
    );
}

} // namespace cse498
