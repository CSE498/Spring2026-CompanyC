#pragma once

#include <random>
#include "../core/WorldBase.hpp"
#include "../core/Building.hpp"
#include "../Agents/PacingAgent.hpp"

namespace cse498 {

class DynamicWorld : public WorldBase {
public:
  DynamicWorld() {
    ConfigureCellTypes();
    GenerateWorld(100, 100);
  }

protected:
  enum ActionType {
    REMAIN_STILL = 0,
    MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
    MOVE_UP_LEFT, MOVE_UP_RIGHT, MOVE_DOWN_LEFT, MOVE_DOWN_RIGHT,
    COLLECT,
    BUILD_LUMBERYARD, BUILD_QUARRY, BUILD_SPAWNER, BUILD_FARM, BUILD_TOWNHALL
  };

  // Counter for amount of times UpdateWorld has been called
  size_t mUpdateCounter = 0;

  // vector of all the buildings in the world
  std::vector<Building> mBuildings;

  // spawner positions and their build times (for 60-tick spawn interval)
  std::vector<std::pair<WorldPosition, size_t>> mSpawners;

  // CellType IDs
  size_t mGrassId = 0;
  size_t mTreeId = 0;
  size_t mStoneId = 0;
  size_t mWheatId = 0;
  size_t mQuarryId = 0;
  size_t mLumberyardId = 0;
  size_t mFarmId = 0;
  size_t mSpawnerId = 0;
  size_t mTownhallId = 0;

  /// Provide the agent with all possible DynamicWorld actions.
  void ConfigAgent(AgentBase & agent) override {
    agent.AddAction("up", MOVE_UP);
    agent.AddAction("down", MOVE_DOWN);
    agent.AddAction("left", MOVE_LEFT);
    agent.AddAction("right", MOVE_RIGHT);
    agent.AddAction("up_left", MOVE_UP_LEFT);
    agent.AddAction("up_right", MOVE_UP_RIGHT);
    agent.AddAction("down_left", MOVE_DOWN_LEFT);
    agent.AddAction("down_right", MOVE_DOWN_RIGHT);
    agent.AddAction("collect", COLLECT);
    agent.AddAction("build_lumberyard", BUILD_LUMBERYARD);
    agent.AddAction("build_quarry", BUILD_QUARRY);
    agent.AddAction("build_spawner", BUILD_SPAWNER);
    agent.AddAction("build_farm", BUILD_FARM);
    agent.AddAction("build_townhall", BUILD_TOWNHALL);
  }

  // Helper to configure the main_grid
  void ConfigureCellTypes() {
    mGrassId = mMainGrid.AddCellType("grass", "Open buildable terrain.", '.');
    mTreeId  = mMainGrid.AddCellType("tree", "Wood resource.", 'T');
    mStoneId = mMainGrid.AddCellType("stone", "Stone resource.", 'S');
    mWheatId = mMainGrid.AddCellType("wheat", "Wheat resource.", 'W');
    mQuarryId = mMainGrid.AddCellType("quarry", "Produces stone and steel.", 'Q');
    mLumberyardId = mMainGrid.AddCellType("lumberyard", "Produces wood.", 'L');
    mFarmId = mMainGrid.AddCellType("farm", "Produces wheat.", 'F');
    mSpawnerId = mMainGrid.AddCellType("spawner", "Spawns agents.", 'A');
    mTownhallId = mMainGrid.AddCellType("townhall", "Win condition.", 'H');
  }

  // helper function to place clusters materials
  void PlaceCluster(size_t type_id, int center_x, int center_y, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dx = -radius; dx <= radius; ++dx) {
        int x = center_x + dx;
        int y = center_y + dy;

        if (!mMainGrid.IsValid(x, y)) continue;

        if (dx * dx + dy * dy <= radius * radius) {
          WorldPosition pos(x, y);
          if (mMainGrid[pos] == mGrassId) {
            mMainGrid[pos] = type_id;
          }
        }
      }
    }
  }
  // Reconfigure main_grid to generate the world with DynamicWorld parameters.
  // Future implementations should guarantee enough resources to make winning possible, as well
  // as implement cluster generation rather than random generation
  void GenerateWorld(size_t width, size_t height) {
    mMainGrid.Resize(width, height, mGrassId);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> x_dist(2, static_cast<int>(width) - 3);
    std::uniform_int_distribution<int> y_dist(2, static_cast<int>(height) - 3);

    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 3);

    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 3);

    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 3);
  }

  // Temporary: any in-bounds tile is currently walkable.
  // Later, building occupancy / blocked-tile rules can be added here.
  int DoAction(AgentBase & agent, size_t action_id) override;

  // This will handle resource generation from buildings, as well as any other future autonomous world actions
  void UpdateWorld() override;

};

} // namespace cse498