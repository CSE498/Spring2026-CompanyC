/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base path finding interface for all agent types.
 * @author Matthew Vazquez
 * AI was used in assistance of developing this class
 **/

#pragma once

#include <vector>
#include <cstddef>
#include <functional>
#include <ostream>
#include <algorithm>   // std::reverse
#include <optional>    // std::optional
#include <utility>     // std::reference_wrapper
#include <functional>  // std::reference_wrapper, std::cref

namespace cse498 {

// Take a start position, optional goal(s), and constraints
// Run a pathfinding or path-constructing strategy
// Output a WorldPath

// Position structure of world
struct Position {
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Print Position nicely
inline std::ostream& operator<<(std::ostream& os, const Position& p) {
    return os << "(" << p.x << "," << p.y << ")";
}

// Multiple Path Types
enum class PathType {
    Shortest,
    Patrol,
    Avoid,
    Explore
};

// Position Hash for visited look up
struct PositionHash {
    std::size_t operator()(const Position& p) const {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }
};

// Sample World Path
class SampleWorldPath {
public:
    SampleWorldPath() = default;

    void Add(const Position& p) { points.push_back(p); }

    const std::vector<Position>& Points() const { return points; }

    bool Empty() const { return points.empty(); }

    std::size_t Length() const { return points.size(); }

    void Reverse() { std::reverse(points.begin(), points.end()); }

private:
    std::vector<Position> points;
};

// Print WorldPath
inline std::ostream& operator<<(std::ostream& os, const SampleWorldPath& path) {
    os << "WorldPath(len=" << path.Length() << "): ";
    if (path.Empty()) {
        os << "<empty>";
        return os;
    }

    const auto& pts = path.Points();
    for (std::size_t i = 0; i < pts.size(); ++i) {
        os << pts[i];
        if (i + 1 < pts.size()) os << " -> ";
    }
    return os;
}

// Sample World View
class WorldView {
public:
    WorldView(int w, int h) : width(w), height(h) {}

    bool IsWalkable(Position p) const {
        return !(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height);
    }

    void GetNeighbors(Position p, std::vector<Position>& out) const {
        out.clear();
        Position candidates[4] = {
            {p.x + 1, p.y}, {p.x - 1, p.y}, {p.x, p.y + 1}, {p.x, p.y - 1}
        };
        for (auto c : candidates) {
            if (IsWalkable(c)) out.push_back(c);
        }
    }

    int Width() const { return width; }
    int Height() const { return height; }

private:
    int width = 0;
    int height = 0;
};

// Print world
inline std::ostream& operator<<(std::ostream& os, const WorldView& w) {
    os << "WorldView(" << w.Width() << "x" << w.Height() << ")";
    return os;
}

// Path Request API to generate path
struct PathRequest {
    PathType type;
    Position start;
    Position goal;
    int max_length = 0;
    std::vector<Position> avoid;
};

// Generate Path based on PathRequest, returns WorldPath
class PathGenerator {
public:
    // Store a safe non-owning reference (no raw pointer)
    void SetWorldView(const WorldView& world);

    // Const because they should not mutate PathGenerator state
    SampleWorldPath GeneratePath(const PathRequest& req) const;

    SampleWorldPath GenerateShortestPath(Position start, Position goal) const;

    SampleWorldPath GeneratePatrolPath(Position start, int max_length) const;

    SampleWorldPath GenerateAvoidPath(
        Position start,
        Position goal,
        const std::vector<Position>& avoid
    ) const;

    SampleWorldPath GenerateExplorePath(Position start, int max_length) const;

private:
    std::optional<std::reference_wrapper<const WorldView>> world_view;
};

} // namespace cse498