#include "StateGridPosition.hpp"



#include <sstream>

int StateGridPosition::DistanceTo(const StateGridPosition& other) const {
  auto abs = [](int v){ return v < 0 ? -v : v; };
  return abs(mX - other.mX) + abs(mY - other.mY);
}

std::string StateGridPosition::ToString() const {
  std::ostringstream oss;
  oss << "(x=" << mX << ",y=" << mY << ")";
  return oss.str();
}



StateGridPosition StateGridPosition::MoveUp() const {
    return StateGridPosition(mX, mY + 1);
}
StateGridPosition StateGridPosition::MoveDown() const {
    return StateGridPosition(mX, mY - 1);
}
StateGridPosition StateGridPosition::MoveLeft() const {
    return StateGridPosition(mX - 1, mY);
}
StateGridPosition StateGridPosition::MoveRight() const {
    return StateGridPosition(mX + 1, mY);
}


std::array<StateGridPosition, 4> StateGridPosition::Neighbors() const {
    return {
        MoveUp(),
        MoveDown(),
        MoveLeft(),
        MoveRight()
    };
}