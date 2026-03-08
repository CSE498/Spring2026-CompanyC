#include "PathGenerator.hpp"

#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace cse498 {

void PathGenerator::SetWorldView(const WorldView& world) {
    world_view = std::cref(world);
}

SampleWorldPath PathGenerator::GeneratePath(const PathRequest& req) const {
    if (!world_view.has_value()) {
        return SampleWorldPath{};
    }

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

SampleWorldPath PathGenerator::GenerateShortestPath(Position start, Position goal) const {
    SampleWorldPath path;

    // Defensive in case someone calls GenerateShortestPath directly without SetWorldView.
    if (!world_view.has_value()) {
        return path;
    }

    const WorldView& w = world_view->get();

    // Validate start/goal are in-bounds/walkable (review comment)
    if (!w.IsWalkable(start) || !w.IsWalkable(goal)) {
        return SampleWorldPath{};
    }

    std::queue<Position> q;
    std::unordered_map<Position, Position, PositionHash> parent;

    q.push(start);
    parent[start] = start;  // mark visited

    bool found = false;

    while (!q.empty()) {
        Position current = q.front();
        q.pop();

        if (current == goal) {
            found = true;
            break;
        }

        std::vector<Position> neighbors;
        w.GetNeighbors(current, neighbors);

        for (const Position& next : neighbors) {
            // GetNeighbors already filters by IsWalkable(), but this is harmless.
            if (parent.find(next) == parent.end()) {
                parent[next] = current;
                q.push(next);
            }
        }
    }

    if (!found) {
        return SampleWorldPath{};
    }

    // Reconstruct path: goal -> start
    Position p = goal;
    while (!(p == parent[p])) {
        path.Add(p);
        p = parent[p];
    }
    path.Add(start);

    // Reverse path: start -> goal
    path.Reverse();
    return path;
}

SampleWorldPath PathGenerator::GeneratePatrolPath(Position start, int max_length) const {
    (void)start;
    (void)max_length;
    return SampleWorldPath{};
}

SampleWorldPath PathGenerator::GenerateAvoidPath(
    Position start,
    Position goal,
    const std::vector<Position>& avoid
) const {
    (void)start;
    (void)goal;
    (void)avoid;
    return SampleWorldPath{};
}

SampleWorldPath PathGenerator::GenerateExplorePath(Position start, int max_length) const {
    (void)start;
    (void)max_length;
    return SampleWorldPath{};
}

} // namespace cse498