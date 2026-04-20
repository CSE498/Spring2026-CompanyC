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

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> coin(0, 1);

    // Use the H positions from the map directly
    auto spawnPositions = world.GetEnemySpawnPositions();
    for (size_t i = 0; i < spawnPositions.size(); ++i)
    {
        auto& agent = world.AddAgent<PacingAgent>("Pacer " + std::to_string(i));
        agent.SetLocation(spawnPositions[i]);

        if (coin(gen) == 0)
            agent.SetHorizontal();
        else
            agent.SetVertical();
    }

    world.Run();
}