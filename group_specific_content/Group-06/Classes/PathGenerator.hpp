
#pragma once

#include <vector>
#include <cstddef>
#include <functional>
#include <ostream>


// Take a start position, optional goal(s), and constraints
// Run a pathfinding or path-constructing strategy
// Output a WorldPath 

// Find shorest path
// Generate patrol path
// Create path that avoids _
// Generate explore path


// Position structure of world
struct Position {
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Optional: print Position nicely
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
class WorldPath {
public:
    WorldPath() = default;

    void Add(const Position& p) {
        points.push_back(p);
    }

    const std::vector<Position>& Points() const {
        return points;
    }

    bool Empty() const {
        return points.empty();
    }

    std::size_t Length() const {
        return points.size();
    }

    void Reverse() {
        std::reverse(points.begin(), points.end());
    }

private:
    std::vector<Position> points;
};

// Print WorldPath as: WorldPath(len=3): (0,0) -> (1,0) -> (1,1)
inline std::ostream& operator<<(std::ostream& os, const WorldPath& path) {
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



class WorldView {
public:

    WorldView(int w, int h) : width(w), height(h) {}

    bool IsWalkable(Position p) const{
        if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) return false;
        return true;
    }

    void GetNeighbors(Position p, std::vector<Position>& out) const {
        out.clear();
        Position candidates[4] = {
            {p.x+1, p.y}, {p.x-1, p.y}, {p.x, p.y+1}, {p.x, p.y-1}
        };
        for (auto c : candidates) {
            if (IsWalkable(c)) out.push_back(c);
        }
    }

    int Width() const { return width; }
    int Height() const { return height; }

    
private:
    int width, height;
};

inline std::ostream& operator<<(std::ostream& os, const WorldView& w) {
    os << "WorldView(" << w.Width() << "x" << w.Height() << ")";
    return os;
}

// Path Request API to generate path
struct PathRequest {
    PathType type;
    Position start;
    Position goal;        // unused for patrol / explore
    int max_length = 0;   // needed for patrol / explore
    std::vector<Position> avoid; // stuff to avoid
};




// Generate Path based on PathRequest, returns WorldPath
class PathGenerator {
public:
    void SetWorldView(const WorldView& world);

    WorldPath GeneratePath(
        const PathRequest& req
    );

    WorldPath GenerateShortestPath (
        Position start,
        Position goal
    );

    WorldPath GeneratePatrolPath(
        Position start,
        int max_length
    );

    WorldPath GenerateAvoidPath(
        Position start,
        Position goal,
        std::vector<Position> avoid
    );

    WorldPath GenerateExplorePath(
        Position start, 
        int max_length
    );


private:
    const WorldView* world_view = nullptr;

};
