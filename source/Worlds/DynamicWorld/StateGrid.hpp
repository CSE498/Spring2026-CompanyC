#include <vector>

// A grid representation of the environment that the agent simulation will run in.

class StateGrid {
public:
    struct StateGridPosition {

    };

    struct State {
      int stateID;
    };

    // Custom Constructor
    StateGrid(int width, int height);

    // Public Member Functions
    State GetState(StateGridPosition pos);
    void SetState(StateGridPosition pos, State stateID);
    bool InBounds(StateGridPosition pos);

private:
    // Main data structure to manage states
    std::vector<std::vector<State> > mGrid;
        
    // Resize the current grid without resetting any filled positions
    void Resize(int width, int height);
};
