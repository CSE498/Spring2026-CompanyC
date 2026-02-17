#pragma once

#include <vector>
#include "StateGridPosition.h"

namespace cse498 {

// The state of a cell in the stategrid
struct State {
  int stateID = 0;
  bool isAccessible = false;
};


// A grid representation of the environment that the agent simulation will run in.

class StateGrid {
public:

    // Constructors
    StateGrid(int height, int width)
      : mHeight(height), mWidth(width), mGrid(height*width) {
    }
    StateGrid() = default;

    // -- Accessors --
    [[nodiscard]] size_t GetHeight() const { return mHeight; }
    [[nodiscard]] size_t GetWidth() const { return mWidth; }

    [[nodiscard]] State const &GetState(StateGridPosition pos) const { 
      return mGrid[ToIndex(pos.X(), pos.Y())];
    };
   
    // Public Member Functions
    void SetState(StateGridPosition pos, State state) {
      mGrid[ToIndex(pos.X(), pos.Y())] = state;
    };
    
    bool InBounds(StateGridPosition pos) const {
      return pos.X() < mWidth && pos.Y() < mHeight;
    };

private:
    // Dimensions
    int mHeight = 0;
    int mWidth = 0;

    // Main data structure to manage states
    std::vector<State> mGrid;
    
    // Converts x and y coordinates to position in the grid
    size_t ToIndex(size_t x, size_t y) const {
      return x + y * mWidth;
    };

    // Resize the current grid (Currently implementing - unfinished)
    void Resize(int newHeight, int newWidth) {
      // Create new grid with new sizing
      std::vector<State> newGrid(newHeight*newWidth);

      // Copy old elements into new grid instance
    }
};

}
