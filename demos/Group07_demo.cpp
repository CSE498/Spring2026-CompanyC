/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a DynamicWorld simulation.
 * @note Status: PROPOSAL
 **/

#include "../source/Interfaces/TrashInterface.hpp"
#include "../source/Worlds/DynamicWorld.hpp"
#include <random>

// Minimal agent that always returns a fixed action ID for controlled testing.
class StubAgent : public cse498::AgentBase {
public:
  size_t next_action = 0;

  StubAgent(size_t id, const std::string & name, const cse498::WorldBase & world)
    : AgentBase(id, name, world) { }

  size_t SelectAction(const cse498::WorldGrid &) override { 
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> choice(0, this->action_map.size()-1);

    return choice(gen); 
    }
};

int main(){
    cse498::DynamicWorld world;

    size_t basicAgentCount= 80;

    int agentPositionX = 0;
    int agentPositionY = 0;

    std::cout << "Leader position X: ";
    std::cin >> agentPositionX;
    std::cout << "Leader position Y: ";
    std::cin >> agentPositionY;

    world.AddAgent<StubAgent>("Leader").SetLocation(cse498::WorldPosition{agentPositionX, agentPositionY});

    for (int i = 0; i < basicAgentCount; i++) {
        std::string name = "Basic Agent " + std::to_string(i+1);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> x_pos(0, 79);
        std::uniform_int_distribution<int> y_pos(0, 79);
        world.AddAgent<StubAgent>(name).SetLocation(cse498::WorldPosition{x_pos(gen), y_pos(gen)});
    }

    world.Run();
    
    return 0;
}