#pragma once

#include <cassert>
#include <string>

/**
 * Direction an agent is facing in the grid.
 */
enum class Direction {
    North,
    East,
    South,
    West
};

class StateGrid;

/**
 * Represents a single position inside a StateGrid.
 * Stores (x, y) coordinates and an orientation.
 */
class StateGridPosition {
public:
    // Constructors
    StateGridPosition();
    StateGridPosition(int x, int y, Direction facing = Direction::North);

    // Getters
    int X() const;
    int Y() const;
    Direction Facing() const;

    void SetFacing(Direction d);

    // Movement
    StateGridPosition Moved(Direction d) const;
    // Add move if valid func when stategrid done
    // Add can move func when stategrid done

    // Comparisons
    bool operator==(const StateGridPosition& other) const;
    bool operator!=(const StateGridPosition& other) const;

    // String Representation
    std::string ToString() const;

private:
    int mX;
    int mY;
    Direction mFacing;

    static void Delta(Direction d, int& dx, int& dy);
};
