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
class StateGrid;

enum class Direction : std::uint8_t { North=0, East=1, South=2, West=3 };

/// Represents an agent position within a grid.
class StateGridPosition {
private:
int x = 0;   
int y = 0;   
Direction facing = Direction::North;

public:
StateGridPosition() = default;
StateGridPosition(int x, int y, Direction f = Direction::North)
    : x(x), y(y), facing(f)
{
    assert(x >= 0 && y >= 0 && "Negative cell coordinates are programmer error.");
}

StateGridPosition(const StateGridPosition&) = default;
StateGridPosition& operator=(const StateGridPosition&) = default;

// -- Accessors --
[[nodiscard]] int X() const { return x; }
[[nodiscard]] int Y() const { return y; }
[[nodiscard]] Direction Facing() const { return facing; }

void SetFacing(Direction d) { facing = d; }

// Enable all comparison operators (==, !=, <, <=, >, >=)
auto operator<=>(const StateGridPosition&) const = default;

// -- Movement helpers --
[[nodiscard]] StateGridPosition Moved(Direction d, int steps=1) const;
[[nodiscard]] StateGridPosition Forward(int steps=1) const { return Moved(facing, steps); }

[[nodiscard]] StateGridPosition RotatedLeft() const;
[[nodiscard]] StateGridPosition RotatedRight() const;

// -- Grid-aware checks --
[[nodiscard]] bool CanMove(Direction d, const StateGrid& grid) const;
bool MoveIfValid(Direction d, const StateGrid& grid);

[[nodiscard]] std::optional<StateGridPosition> Neighbor(Direction d, const StateGrid& grid) const;

// -- Utility --
[[nodiscard]] int ManhattanDistanceTo(const StateGridPosition& other) const;
[[nodiscard]] std::string ToString() const;

private:
static void Delta(Direction d, int& dx, int& dy);
static Direction TurnLeft(Direction d);
static Direction TurnRight(Direction d);
};


