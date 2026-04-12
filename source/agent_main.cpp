/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/ClassicAgent.hpp"

#include "Interfaces/TrashInterface.hpp"
#include "Worlds/MazeWorld.hpp"

using namespace cse498;

int main()
{
  MazeWorld world;
  world.AddAgent<ClassicAgent>("Classic 1").SetLocation(WorldPosition{3,1});
  world.AddAgent<TrashInterface>("Interface").SetSymbol('@').SetLocation(WorldPosition{1,1});
}