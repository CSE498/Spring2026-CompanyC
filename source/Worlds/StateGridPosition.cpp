#include "StateGridPosition.h"

#include <sstream>

StateGridPosition::StateGridPosition()
    : mX(0), mY(0), mFacing(Direction::North)
{
}

StateGridPosition::StateGridPosition(int x, int y, Direction facing)
    : mX(x), mY(y), mFacing(facing)
{
    // Negative coordinates are programmer error
    assert(mX >= 0 && mY >= 0);
}

int StateGridPosition::X() const {
    return mX;
}

int StateGridPosition::Y() const {
    return mY;
}

Direction StateGridPosition::Facing() const {
    return mFacing;
}

void StateGridPosition::SetFacing(Direction d) {
    mFacing = d;
}

StateGridPosition StateGridPosition::Moved(Direction d) const {
    int dx, dy;
    Delta(d, dx, dy);
    return StateGridPosition(mX + dx, mY + dy, mFacing);
}

/* Add a can move function tied to state grid class */

/* Add a moveifvalid function tied to state grid class */

bool StateGridPosition::operator==(const StateGridPosition& other) const {
    return mX == other.mX &&
           mY == other.mY &&
           mFacing == other.mFacing;
}

bool StateGridPosition::operator!=(const StateGridPosition& other) const {
    return !(*this == other);
}

std::string StateGridPosition::ToString() const {
    std::ostringstream oss;
    oss << "(x=" << mX << ", y=" << mY << ")";
    return oss.str();
}
