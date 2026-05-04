// Native smoke test for DynamicWorld.
// Replicates the agent setup used in web_main.cpp (one "builder" + 15
// StubAgents) and runs N ticks, reporting whether actions actually fire.

#include <algorithm>
#include <iostream>
#include <random>

#include "Agents/TendencyAgent.hpp"
#include "Worlds/DynamicWorld.hpp"

using namespace cse498;

class StubAgent : public AgentBase {
public:
  StubAgent(size_t id, const std::string & name, const WorldBase & world)
    : AgentBase(id, name, world) {}

  size_t SelectAction(WorldGrid &) override {
    if (action_map.empty()) return 0;
    static thread_local std::mt19937 gen{std::random_device{}()};
    std::uniform_int_distribution<size_t> choice(0, action_map.size() - 1);
    auto it = action_map.begin();
    std::advance(it, choice(gen));
    return it->second;
  }
};

int main(int argc, char **argv) {
  const size_t num_ticks = (argc >= 2) ? std::stoul(argv[1]) : 500;

  DynamicWorld world(40, 40);
  auto & builder = world.AddAgent<StubAgent>("builder");
  builder.SetLocation(WorldPosition{1, 1});

  std::mt19937 gen{42};
  std::uniform_int_distribution<int> x_pos(0, 39);
  std::uniform_int_distribution<int> y_pos(0, 39);
  for (int i = 0; i < 15; ++i) {
    auto & a = world.AddAgent<StubAgent>("Basic " + std::to_string(i + 1));
    a.SetLocation(WorldPosition{x_pos(gen), y_pos(gen)});
  }

  std::cout << "Starting smoke run: " << num_ticks << " ticks, "
            << world.GetNumAgents() << " agents.\n";

  WorldPosition builder_start = builder.GetLocation().AsWorldPosition();

  for (size_t t = 0; t < num_ticks && !world.IsRunOver(); ++t) {
    world.Tick();
  }

  const auto & names = world.GetWorldResourceNames();
  const auto & counts = world.GetWorldResourceCounts();

  std::cout << "\n=== After ticks ===\n";
  std::cout << "Resources: ";
  for (size_t i = 0; i < names.size(); ++i) {
    std::cout << names[i] << "=" << counts[i] << " ";
  }
  std::cout << "\n";

  WorldPosition builder_end = builder.GetLocation().AsWorldPosition();
  std::cout << "Builder moved: (" << builder_start.X() << "," << builder_start.Y()
            << ") -> (" << builder_end.X() << "," << builder_end.Y() << ")\n";
  std::cout << "Final agent count: " << world.GetNumAgents()
            << " (spawners may have produced new agents)\n";
  std::cout << "Run over: " << (world.IsRunOver() ? "yes (win or cutoff)" : "no")
            << "\n";

  return 0;
}
