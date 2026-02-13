/**
 * A coordinate position in a StateGrid (grid cell coordinates + facing).
 * 
 */

#pragma once

#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <array>
class StateGrid;

/// Represents an agent position within a grid.
class StateGridPosition {
private:
int mX = 0;   
int mY = 0;   

public:
StateGridPosition() = default;
StateGridPosition(int x, int y)
    : mX(x), mY(y)
{
    assert(x >= 0 && y >= 0 && "Negative cell coordinates are programmer error.");
}

StateGridPosition(const StateGridPosition&) = default;
StateGridPosition& operator=(const StateGridPosition&) = default;

// -- Accessors --
int GetX() const { return mX; }
int GetY() const { return mY; }

// Enable all comparison operators 
auto operator<=>(const StateGridPosition&) const = default;

// -- Movement helpers --
StateGridPosition MoveUp() const;
StateGridPosition MoveDown() const;
StateGridPosition MoveLeft() const;
StateGridPosition MoveRight() const;
std::array<StateGridPosition, 4> Neighbors() const;

// -- Utility --
int DistanceTo(const StateGridPosition& other) const;
std::string ToString() const;

};


