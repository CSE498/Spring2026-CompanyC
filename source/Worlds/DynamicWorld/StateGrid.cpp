#include "StateGrid.hpp"

StateGrid::StateGrid(int width, int height) {
    Resize(width, height);
}


void StateGrid::Resize(int width, int height) {

}

void StateGrid::SetState(StateGridPosition pos, State stateID) {

}

StateGrid::State StateGrid::GetState(StateGridPosition pos) {
  return State();
}

bool StateGrid::InBounds(StateGridPosition pos) {
  return false;
}
