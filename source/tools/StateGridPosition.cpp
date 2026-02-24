#include "StateGridPosition.hpp"
#include <cstdint>
#include <sstream>

namespace cse498 {

static constexpr size_t numNeighbors = 4;

int StateGridPosition::DistanceTo(const StateGridPosition& other) const {
    std::int64_t dx = static_cast<std::int64_t>(mX) - other.mX;
    std::int64_t dy = static_cast<std::int64_t>(mY) - other.mY;

    dx = dx < 0 ? -dx : dx;
    dy = dy < 0 ? -dy : dy;

    return static_cast<int>(dx + dy);
}

std::string StateGridPosition::ToString() const {
    std::ostringstream oss;
    oss << "(x=" << mX << ",y=" << mY << ")";
    return oss.str();
}

StateGridPosition StateGridPosition::MoveUp() const {
    return StateGridPosition(mX, mY - 1);
}
StateGridPosition StateGridPosition::MoveDown() const {
    return StateGridPosition(mX, mY + 1);
}
StateGridPosition StateGridPosition::MoveLeft() const {
    return StateGridPosition(mX - 1, mY);
}
StateGridPosition StateGridPosition::MoveRight() const {
    return StateGridPosition(mX + 1, mY);
}

std::array<StateGridPosition, numNeighbors> StateGridPosition::Neighbors() const {
    return {
        MoveUp(),
        MoveDown(),
        MoveLeft(),
        MoveRight()
    };
}

}
