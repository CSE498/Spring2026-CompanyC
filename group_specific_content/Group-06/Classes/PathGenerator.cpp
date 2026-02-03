#include "PathGenerator.hpp"
#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>


void PathGenerator::SetWorldView(const WorldView& world)
{
    world_view = &world;
};

WorldPath PathGenerator::GeneratePath(const PathRequest& req)
{
    if (!world_view) return WorldPath{};

    switch (req.type) {
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
    Position start,
    Position goal
) {
    WorldPath path;

    if (!world_view) return path;

    // BFS frontier
    std::queue<Position> q;

    // parent map: presence == visited
    std::unordered_map<Position, Position, PositionHash> parent;

    q.push(start);
    parent[start] = start;  // mark visited, root points to itself

    bool found = false;

    while (!q.empty()) {
        Position current = q.front();
        q.pop();

        if (current == goal) {
            found = true;
            break;
        }

        std::vector<Position> neighbors;
        world_view->GetNeighbors(current, neighbors);

        for (const Position& next : neighbors) {
            if (!world_view->IsWalkable(next)) continue;

            if (parent.find(next) == parent.end()) {
                parent[next] = current;
                q.push(next);
            }
        }
    }

    if (!found) {
        return WorldPath{}; // no path
    }

    // Reconstruct path: goal → start
    Position p = goal;
    while (!(p == parent[p])) {
        path.Add(p);
        p = parent[p];
    }
    path.Add(start);

    // Reverse to start → goal
    std::reverse(
        path.Points().begin(),
        path.Points().end()
    );

    return path;
}