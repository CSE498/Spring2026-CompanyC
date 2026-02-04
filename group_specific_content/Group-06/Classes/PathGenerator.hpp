#pragma once

#include <vector>

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



// Sample World View
class WorldView {
public:
    virtual bool IsWalkable(Position p) const = 0;
    virtual void GetNeighbors(
        Position p,
        std::vector<Position>& out_neighbors
    ) const = 0;

    virtual ~WorldView() = default;
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

private:
    std::vector<Position> points;
};

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
    const WorldView* world_view;

};
