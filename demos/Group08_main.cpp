/**
 * Example main driver file to demonstrate integration.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 * 
 * How to run this file:
 * cd to the root of the project and run:
 * c++ -std=c++23 demos/Group08_main.cpp -Wall -Wextra -pedantic -o group08
 * ./group08
 **/

// Include the modules that we will be using.
#include "../source/Agents/PacingAgent.hpp"
#include "../source/Interfaces/TrashInterface.hpp"
#include "../source/Worlds/InteractionHeavyWorld.hpp"

using namespace cse498;

int main()
{
  InteractionHeavyWorld world;

  world.AddAgent<PacingAgent>("Pacer 1")
        .SetLocation(WorldPosition{3,1});

  world.AddAgent<PacingAgent>("Pacer 2")
        .SetLocation(WorldPosition{6,1});

  world.AddAgent<PacingAgent>("Guard 1")
        .SetHorizontal()
        .SetLocation(WorldPosition{10,10});

  world.AddAgent<PacingAgent>("Guard 2")
        .SetHorizontal()
        .ToggleDirection()
        .SetLocation(WorldPosition{15,15});

  world.AddAgent<TrashInterface>("Interface")
        .SetSymbol('@')
        .SetLocation(WorldPosition{1,1});

  world.Run();
}