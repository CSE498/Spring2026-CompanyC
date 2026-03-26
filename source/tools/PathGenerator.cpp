#include "PathGenerator.hpp"

#include <algorithm>

namespace cse498 {

namespace {
std::vector<StateGridPosition> GetNeighbors(const StateGridPosition& p)
{
    std::vector<StateGridPosition> neighbors;

    if (p.GetX() > 0) {
        neighbors.emplace_back(p.GetX() - 1, p.GetY());
    }

    neighbors.emplace_back(p.GetX() + 1, p.GetY());

    if (p.GetY() > 0) {
        neighbors.emplace_back(p.GetX(), p.GetY() - 1);
    }

    neighbors.emplace_back(p.GetX(), p.GetY() + 1);

    return neighbors;
}
}

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
        case PathType::Patrol:
            return GeneratePatrolPath(req.start, req.max_length);
        case PathType::Avoid:
            return GenerateAvoidPath(req.start, req.goal, req.avoid);
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

    const WorldView& w = world_view->get();

    if (!w.IsWalkable(start) || !w.IsWalkable(goal))
        return path;

    std::queue<StateGridPosition> q;

    std::unordered_map<
        StateGridPosition,
        StateGridPosition,
        StateGridPositionHash
    > parent;

    q.push(start);
    parent[start] = start;

    bool found = false;

    while (!q.empty())
    {
        StateGridPosition current = q.front();
        q.pop();

        if (current == goal)
        {
            found = true;
            break;
        }

        for (const auto& next : GetNeighbors(current))
        {
            if (!w.IsWalkable(next))
                continue;

            if (parent.find(next) == parent.end())
            {
                parent[next] = current;
                q.push(next);
            }
        }
    }

    if (!found)
        return path;

    std::vector<Point> points;

    StateGridPosition p = goal;

    while (!(p == parent[p]))
    {
        points.push_back(ToDoublePoint(p));
        p = parent[p];
    }

    points.push_back(ToDoublePoint(start));

    std::reverse(points.begin(), points.end());

    return WorldPath(points);
}

WorldPath PathGenerator::GeneratePatrolPath(
    StateGridPosition start,
    std::optional<int> max_length
) const
{
    (void)start;
    (void)max_length;
    return WorldPath{};
}

WorldPath PathGenerator::GenerateAvoidPath(
    StateGridPosition start,
    StateGridPosition goal,
    const std::vector<StateGridPosition>& avoid
) const
{
    (void)start;
    (void)goal;
    (void)avoid;
    return WorldPath{};
}

WorldPath PathGenerator::GenerateExplorePath(
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

    auto IsDiscovered = [&](const StateGridPosition& p) -> bool {
        auto it = knowledge.tiles.find(ToWorldPos(p));
        return it != knowledge.tiles.end() && it->second.discovered;
    };

    auto IsFrontier = [&](const StateGridPosition& p) -> bool {
        if (!IsKnownWalkable(p)) return false;

        for (const auto& neighbor : GetNeighbors(p)) {
            if (!IsDiscovered(neighbor)) {
                return true;
            }
        }

        return false;
    };

    WorldPath path;

    if (!IsKnownWalkable(start)) {
        return path;
    }

    std::queue<StateGridPosition> q;
    std::unordered_map<
        StateGridPosition,
        StateGridPosition,
        StateGridPositionHash
    > parent;

    q.push(start);
    parent[start] = start;

    std::optional<StateGridPosition> target;

    while (!q.empty()) {
        StateGridPosition current = q.front();
        q.pop();

        if (IsFrontier(current)) {
            target = current;
            break;
        }

        for (const auto& next : GetNeighbors(current)) {
            if (!IsKnownWalkable(next)) continue;
            if (parent.find(next) != parent.end()) continue;

            parent[next] = current;
            q.push(next);
        }
    }

    if (!target.has_value()) {
        return path;
    }

    std::vector<Point> points;
    StateGridPosition p = *target;

    while (!(p == parent[p])) {
        points.push_back(ToDoublePoint(p));
        p = parent[p];
    }

    points.push_back(ToDoublePoint(start));
    std::reverse(points.begin(), points.end());

    if (max_length.has_value() &&
        points.size() > static_cast<size_t>(*max_length)) {
        points.resize(*max_length);
    }

    return WorldPath(points);
}

}