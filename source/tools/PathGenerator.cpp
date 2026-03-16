#include "PathGenerator.hpp"

#include <algorithm>

namespace cse498 {

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
    assert(!world_view.has_value());

    switch (req.type)
    {
        case PathType::Shortest:
            return GenerateShortestPath(req.start, req.goal);

        case PathType::Patrol:
            return GeneratePatrolPath(req.start, req.max_length);

        case PathType::Avoid:
            return GenerateAvoidPath(req.start, req.goal, req.avoid);

        case PathType::Explore:
            return GenerateExplorePath(req.start, req.max_length);

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

        for (const auto& next : current.Neighbors())
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
    std::optional<int> max_length
) const
{
    (void)start;
    (void)max_length;
    return WorldPath{};
}

}