#include "Worlds/StateGridPosition.h"


#include <sstream>

namespace companyc {

void StateGridPosition::Delta(Direction d, int& dx, int& dy) {
  dx = 0; dy = 0;
  switch (d) {
    case Direction::North: dy = -1; break;
    case Direction::East:  dx =  1; break;
    case Direction::South: dy =  1; break;
    case Direction::West:  dx = -1; break;
    default: assert(false && "Invalid Direction");
  }
}

Direction StateGridPosition::TurnLeft(Direction d) {
  return static_cast<Direction>((static_cast<std::uint8_t>(d) + 3) % 4);
}

Direction StateGridPosition::TurnRight(Direction d) {
  return static_cast<Direction>((static_cast<std::uint8_t>(d) + 1) % 4);
}

StateGridPosition StateGridPosition::Moved(Direction d, int steps) const {
  assert(steps >= 0 && "Negative steps are programmer error.");
  int dx, dy;
  Delta(d, dx, dy);
  return StateGridPosition{x + dx*steps, y + dy*steps, facing};
}

StateGridPosition StateGridPosition::RotatedLeft() const {
  StateGridPosition out = *this;
  out.facing = TurnLeft(out.facing);
  return out;
}

StateGridPosition StateGridPosition::RotatedRight() const {
  StateGridPosition out = *this;
  out.facing = TurnRight(out.facing);
  return out;
}

bool StateGridPosition::CanMove(Direction d, const StateGrid& grid) const {
  StateGridPosition nxt = Moved(d, 1);
  return grid.InBounds(nxt.X(), nxt.Y());
}

bool StateGridPosition::MoveIfValid(Direction d, const StateGrid& grid) {
  StateGridPosition nxt = Moved(d, 1);
  if (!grid.InBounds(nxt.X(), nxt.Y())) return false;

  x = nxt.x;
  y = nxt.y;
  return true;
}

std::optional<StateGridPosition> StateGridPosition::Neighbor(Direction d, const StateGrid& grid) const {
  StateGridPosition nxt = Moved(d, 1);
  if (!grid.InBounds(nxt.X(), nxt.Y())) return std::nullopt;
  return nxt;
}

int StateGridPosition::ManhattanDistanceTo(const StateGridPosition& other) const {
  auto abs = [](int v){ return v < 0 ? -v : v; };
  return abs(x - other.x) + abs(y - other.y);
}

std::string StateGridPosition::ToString() const {
  std::ostringstream oss;
  oss << "(x=" << x << ",y=" << y << ")";
  return oss.str();
}

} 
