/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/ClassicAgent.hpp"
#include "Agents/ClassicDynamicAgent.hpp"
#include "Worlds/DynamicWorld.hpp"

using namespace cse498;

int main()
{
  DynamicWorld world;

  world.AddAgent<ClassicDynamicAgent>("Dynamic Classic Agent").SetLocation(WorldPosition{1,1});

  world.Run();
}
