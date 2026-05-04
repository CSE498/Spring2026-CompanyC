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
#include <random>

using namespace cse498;

int main()
{
    InteractionHeavyWorld world;

    world.AddAgent<TrashInterface>("Player")
        .SetSymbol('@')
        .SetLocation(world.GetStartPosition());

    world.Run();
}