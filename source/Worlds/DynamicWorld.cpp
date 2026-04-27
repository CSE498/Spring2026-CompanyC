#include <algorithm>
#include "./DynamicWorld.hpp"

const std::unordered_map<std::string, size_t> ResourceTick = {
  {"wood", 20},
  {"steel", 10},
  {"stone", 10},
  {"wheat", 10}
};

// Number of ticks between each spawner agent spawn.
constexpr size_t kSpawnerTickInterval = 60;

// Amount of wood required to build a town hall (win condition).
constexpr size_t townHallWoodWinCondition = 500;
// Amount of stone required to build a town hall (win condition).
constexpr size_t townHallStoneWinCondition = 500;
// Amount of steel required to build a town hall (win condition).
constexpr size_t townHallSteelWinCondition = 500;
// Amount of wheat required to build a town hall (win condition).
constexpr size_t townHallWheatWinCondition = 500;

// Amount of wood required to build a lumberyard.
constexpr size_t lumberyardWoodBuildCondition = 20;
// Amount of steel required to build a lumberyard.
constexpr size_t lumberyardSteelBuildCondition = 20;
// Amount of stone required to build a quarry.
constexpr size_t quarryStoneBuildCondition = 20;
// Amount of wood required to build a quarry.
constexpr size_t quarryWoodBuildCondition = 20;
// Amount of stone required to build a spawner.
constexpr size_t spawnerStoneBuildCondition = 30;
// Amount of wheat required to build a spawner.
constexpr size_t spawnerWheatBuildCondition = 30;
// Amount of wheat required to build a farm.
constexpr size_t farmWheatBuildCondition = 20;
// Amount of wood required to build a farm.
constexpr size_t farmWoodBuildCondition = 20;

int cse498::DynamicWorld::DoAction(AgentBase &agent, size_t action_id) {
  const WorldPosition cur = agent.GetLocation().AsWorldPosition();
  WorldPosition next = cur;

  switch (action_id) {
  case REMAIN_STILL:
    next = cur;
    break;
  case MOVE_UP:
    next = cur.Up();
    break;
  case MOVE_DOWN:
    next = cur.Down();
    break;
  case MOVE_LEFT:
    next = cur.Left();
    break;
  case MOVE_RIGHT:
    next = cur.Right();
    break;
  case MOVE_UP_LEFT:
    next = cur.Up().Left();
    break;
  case MOVE_UP_RIGHT:
    next = cur.Up().Right();
    break;
  case MOVE_DOWN_LEFT:
    next = cur.Down().Left();
    break;
  case MOVE_DOWN_RIGHT:
    next = cur.Down().Right();
    break;
  case COLLECT: {
    size_t cell = main_grid[cur];
    if (cell == mTreeId) {
      ++mWorldResourceCounts[ResourceIndex("wood")]; // wood
      main_grid[cur] = mGrassId;
      return 1;
    } else if (cell == mStoneId) {
      ++mWorldResourceCounts[ResourceIndex("stone")]; // stone
      main_grid[cur] = mGrassId;
      return 1;
    } else if (cell == mWheatId) {
      ++mWorldResourceCounts[ResourceIndex("wheat")]; // wheat
      main_grid[cur] = mGrassId;
      return 1;
    }
    return 0;
  }
  case BUILD_LUMBERYARD: {
    if (main_grid[cur] != mGrassId) return 0;
    if (!HasResources({{"wood", lumberyardWoodBuildCondition}, {"steel", lumberyardSteelBuildCondition}})) return 0;
    SpendResources({{"wood", lumberyardWoodBuildCondition}, {"steel", lumberyardSteelBuildCondition}});
    main_grid[cur] = mLumberyardId;
    Building lumberyard(mUpdateCounter);
    lumberyard.AddResource("wood", ResourceTick.at("wood"));
    mBuildings.push_back(lumberyard);
    return 1;
  }
  case BUILD_QUARRY: {
    if (main_grid[cur] != mGrassId) return 0;
    if (!HasResources({{"stone", quarryStoneBuildCondition}, {"wood", quarryWoodBuildCondition}})) return 0;
    SpendResources({{"stone", quarryStoneBuildCondition}, {"wood", quarryWoodBuildCondition}});
    main_grid[cur] = mQuarryId;
    Building quarry(mUpdateCounter);
    quarry.AddResource("steel", ResourceTick.at("steel"));
    quarry.AddResource("stone", ResourceTick.at("stone"));
    mBuildings.push_back(quarry);
    return 1;
  }
  case BUILD_SPAWNER: {
    if (main_grid[cur] != mGrassId) return 0;
    if (!HasResources({{"stone", spawnerStoneBuildCondition}, {"wheat", spawnerWheatBuildCondition}})) return 0;
    SpendResources({{"stone", spawnerStoneBuildCondition}, {"wheat", spawnerWheatBuildCondition}});
    main_grid[cur] = mSpawnerId;
    mSpawners.push_back({cur, mUpdateCounter});
    return 1;
  }
  case BUILD_FARM: {
    if (main_grid[cur] != mGrassId) return 0;
    if (!HasResources({{"wheat", farmWheatBuildCondition}, {"wood", farmWoodBuildCondition}})) return 0;
    SpendResources({{"wheat", farmWheatBuildCondition}, {"wood", farmWoodBuildCondition}});
    main_grid[cur] = mFarmId;
    Building farm(mUpdateCounter);
    farm.AddResource("wheat", ResourceTick.at("wheat"));
    mBuildings.push_back(farm);
    return 1;
  }
  case BUILD_TOWNHALL: {
    if (main_grid[cur] != mGrassId) return 0;
    if (!HasResources({{"wood", townHallWoodWinCondition}, {"stone", townHallStoneWinCondition},
                       {"steel", townHallSteelWinCondition}, {"wheat", townHallWheatWinCondition}})) return 0;
    SpendResources({{"wood", townHallWoodWinCondition}, {"stone", townHallStoneWinCondition},
                    {"steel", townHallSteelWinCondition}, {"wheat", townHallWheatWinCondition}});
    main_grid[cur] = mTownhallId;
    run_over = true;
    return 1;
  }
  default:
    break;
  }

  if (action_id >= REMAIN_STILL && action_id <= MOVE_DOWN_RIGHT) {
    if (!main_grid.IsValid(next))
      return 0;
    if (!main_grid.IsTraversable(main_grid[next]) && agent.GetName() != "Player")
      return 0;
    agent.SetLocation(next);
    return 1;
  }

  return 0;
}

void cse498::DynamicWorld::UpdateWorld() {

  mUpdateCounter++;

  for(const auto& building : mBuildings){
    if (building.GetResources().size() == 0) continue;
    for (const auto& [name, tick_rate] : building.GetResources()){
      const size_t ticks_since_built = mUpdateCounter - building.GetBuiltTime();
      if (tick_rate > 0 && ticks_since_built % tick_rate == 0) {
        ++mWorldResourceCounts[ResourceIndex(name)];
      }
    }
  }

  // Spawner logic: spawn a TendencyAgent at the closest grass cell every 60 ticks
  for (const auto & [pos, built_time] : mSpawners) {
    size_t ticks_since_built = mUpdateCounter - built_time;
    if (ticks_since_built > 0 && ticks_since_built % kSpawnerTickInterval == 0) {
      // Search outward for the nearest grass cell to place the new agent
      int sx = static_cast<int>(pos.X());
      int sy = static_cast<int>(pos.Y());
      bool placed = false;
      for (int radius = 1; radius <= 10 && !placed; ++radius) {
        for (int dy = -radius; dy <= radius && !placed; ++dy) {
          for (int dx = -radius; dx <= radius && !placed; ++dx) {
            if (abs(dx) != radius && abs(dy) != radius) continue;
            int nx = sx + dx;
            int ny = sy + dy;
            if (!main_grid.IsValid(nx, ny)) continue;
            WorldPosition spawn_pos(nx, ny);
            if (main_grid[spawn_pos] == mGrassId) {
              auto & agent = AddAgent<TendencyAgent>("spawned_agent");
              agent.SetLocation(spawn_pos);
              placed = true;
            }
          }
        }
      }
    }
  }

  if (mCutoffTime <= mUpdateCounter) {
    run_over = true;
    this->GetTimer().Stop("Game::Session");
  }
}
