#pragma once

#include <vector>
#include <cassert>

#include "StateGridPosition.hpp"

namespace cse498 {

// The state of a cell in the stategrid
struct State {
  int mStateID = 0;
  bool mIsAccessible = false;
};


// A grid representation of the environment that the agent simulation will run in.

class StateGrid {
public:

    // Constructors
    StateGrid(int height, int width) {
      assert(height >= 0 && width >= 0);

      mHeight = height;
      mWidth = width;
      mGrid.resize(height*width); 
    }
    StateGrid() = default;

    // -- Accessors --
    [[nodiscard]] size_t GetHeight() const { return mHeight; }
    [[nodiscard]] size_t GetWidth() const { return mWidth; }

    [[nodiscard]] State const &GetState(StateGridPosition pos) const {
      assert(InBounds(pos));
      return mGrid[ToIndex(pos.GetX(), pos.GetY())];
    };
   
    // Public Member Functions
    void SetState(StateGridPosition pos, State state) {
      assert(InBounds(pos));

      mGrid[ToIndex(pos.GetX(), pos.GetY())] = state;
    };
    
    bool InBounds(StateGridPosition pos) const {
      return pos.GetX() < mWidth &&
        pos.GetX() >= 0 && 
        pos.GetY() < mHeight &&
        pos.GetY() >= 0;
    };

private:
    // Dimensions
    size_t mHeight = 0;
    size_t mWidth = 0;

    // Main data structure to manage states
    std::vector<State> mGrid;
    
    // Converts x and y coordinates to position in the grid
    size_t ToIndex(size_t x, size_t y) const {
      return x + y * mWidth;
    };
};

}
