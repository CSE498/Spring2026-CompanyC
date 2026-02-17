#include "PathGenerator.hpp"
#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>



void PathGenerator::SetWorldView(const WorldView& world)
{
    world_view = &world;
};

SampleWorldPath PathGenerator::GeneratePath(const PathRequest& req)
{
    if (!world_view) return SampleWorldPath{};

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
            return SampleWorldPath{};
    }
}

SampleWorldPath PathGenerator::GenerateShortestPath(
    Position start,
    Position goal
) {
    SampleWorldPath path;

    if (!world_view) return path;

    // Queue for BFS
    std::queue<Position> q;

    // parent map
    std::unordered_map<Position, Position, PositionHash> parent;

    q.push(start);
    parent[start] = start;  // mark visited

    bool found = false;

    // BFS search 
    while (!q.empty()) {
        Position current = q.front();
        q.pop();

        if (current == goal) {
            found = true;
            break;
        }

        std::vector<Position> neighbors;
        world_view->GetNeighbors(current, neighbors); // Get neighbors, updates neighbors if neighbors

        for (const Position& next : neighbors) {
            if (!world_view->IsWalkable(next)) continue;

            if (parent.find(next) == parent.end()) { // Make sure node isn't visited
                parent[next] = current;
                q.push(next);
            }
        }
    }

    if (!found) {
        return SampleWorldPath{}; // no path
    }

    // Reconstruct path: goal -> start
    Position p = goal;
    while (!(p == parent[p])) {
        path.Add(p);
        p = parent[p];
    }
    path.Add(start);

    // Reverse path: Now start -> goal
    path.Reverse();

    return path;
}

SampleWorldPath PathGenerator::GeneratePatrolPath(Position start, int max_length) {
    (void)start; (void)max_length;
    return SampleWorldPath{};
}

SampleWorldPath PathGenerator::GenerateAvoidPath(Position start, Position goal, std::vector<Position> avoid) {
    (void)start; (void)goal; (void)avoid;
    return SampleWorldPath{};
}

SampleWorldPath PathGenerator::GenerateExplorePath(Position start, int max_length) {
    (void)start; (void)max_length;
    return SampleWorldPath{};
}
