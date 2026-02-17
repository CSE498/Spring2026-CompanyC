/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base path finding interface for all agent types.
 * @author Matthew Vazquez
 **/

#pragma once

#include <vector>
#include <cstddef>
#include <functional>
#include <ostream>


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
    // Constructor
    SampleWorldPath() = default;

    // Add positions to vector
    void Add(const Position& p) {
        points.push_back(p);
    }

    // Getter for vector
    const std::vector<Position>& Points() const {
        return points;
    }
    // Check to see if vector is empty
    bool Empty() const {
        return points.empty();
    }
    // Get length of vector
    std::size_t Length() const {
        return points.size();
    }
    // Reverse vector
    void Reverse() {
        std::reverse(points.begin(), points.end());
    }

private:
    // Vector for points
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

    // Init
    WorldView(int w, int h) : width(w), height(h) {}

    // Get if position is walkable
    bool IsWalkable(Position p) const{
        if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) return false;
        return true;
    }

    // Get 4 neighbors of each point
    void GetNeighbors(Position p, std::vector<Position>& out) const {
        out.clear();
        Position candidates[4] = {
            {p.x+1, p.y}, {p.x-1, p.y}, {p.x, p.y+1}, {p.x, p.y-1}
        };
        for (auto c : candidates) {
            if (IsWalkable(c)) out.push_back(c);
        }
    }
    // Get width
    int Width() const { return width; }
    // Get height
    int Height() const { return height; }

    
private:
    int width, height;
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
    // Set world view
    void SetWorldView(const WorldView& world);

    // Generate default path (shortest)
    SampleWorldPath GeneratePath(
        const PathRequest& req
    );

    // Generate shortest path
    SampleWorldPath GenerateShortestPath (
        Position start,
        Position goal
    );

    // Generate patrol path
    SampleWorldPath GeneratePatrolPath(
        Position start,
        int max_length
    );

    // Generator avoid path
    SampleWorldPath GenerateAvoidPath(
        Position start,
        Position goal,
        std::vector<Position> avoid
    );

    // Generator Explore path
    SampleWorldPath GenerateExplorePath(
        Position start, 
        int max_length
    );


private:
    const WorldView* world_view = nullptr;

};
