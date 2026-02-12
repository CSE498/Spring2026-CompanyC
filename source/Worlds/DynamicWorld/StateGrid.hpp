#include <vector>

// A grid representation of the environment that the agent simulation will run in.

class StateGrid {
public:
    struct StateGridPosition {
      size_t X;
      size_t Y;
    };

    struct State {
      int stateID;
    };

    // Custom Constructor
    StateGrid(int height, int width)
      : mGrid(height*width) {
    }

    // -- Accessors --

    size_t GetHeight() const { return mHeight; }
    size_t GetWidth() const { return mWidth; }

    // Public Member Functions
    State const &GetState(StateGridPosition pos) { 
      return mGrid[ToIndex(pos.X, pos.Y)];
    };
    
    void SetState(StateGridPosition pos, State state) {
      mGrid[ToIndex(pos.X, pos.Y)] = state;
    };
    
    bool InBounds(StateGridPosition pos) {
      return pos.X < mWidth && pos.Y < mHeight;
    };

private:
    // Dimensions
    size_t mHeight;
    size_t mWidth;

    // Main data structure to manage states
    std::vector<State> mGrid;
    
    // Converts x and y coordinates to position in the grid
    size_t const ToIndex(size_t x, size_t y) {
      return x + y * mWidth;
    };

    // Resize the current grid (Unsure of how to implement)
    void Resize(int newHeight, int newWidth) {
      // Create new grid with new sizing
      std::vector<State> newGrid(newHeight*newWidth);

      // Copy old elements into new grid instance
    }
};
