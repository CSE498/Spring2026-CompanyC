#include <algorithm>
#include <cmath>
#include "./DynamicWorld.hpp"

int cse498::DynamicWorld::DoAction(AgentBase &agent, size_t action_id) {
  const WorldPosition cur = agent.GetLocation().AsWorldPosition();
  WorldPosition next = cur;

  switch (action_id) {
  case REMAIN_STILL:
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
      world_global_counts["wood"] += 1;
      main_grid[cur] = mGrassId;
      return true;
    } else if (cell == mStoneId) {
      world_global_counts["stone"] += 1;
      main_grid[cur] = mGrassId;
      return true;
    } else if (cell == mWheatId) {
      world_global_counts["wheat"] += 1;
      main_grid[cur] = mGrassId;
      return true;
    }
    return false;
  }
  case BUILD_LUMBERYARD: {
    if (main_grid[cur] != mGrassId)
      return false;
    if (world_global_counts["wood"] < 20 || world_global_counts["steel"] < 20)
      return false;
    world_global_counts["wood"] -= 20;
    world_global_counts["steel"] -= 20;
    main_grid[cur] = mLumberyardId;
    Building lumberyard(mUpdateCounter);
    lumberyard.AddResource("wood", 20);
    mBuildings.push_back(lumberyard);
    return true;
  }
  case BUILD_QUARRY: {
    if (main_grid[cur] != mGrassId)
      return false;
    if (world_global_counts["stone"] < 20 || world_global_counts["wood"] < 20)
      return false;
    world_global_counts["stone"] -= 20;
    world_global_counts["wood"] -= 20;
    main_grid[cur] = mQuarryId;
    Building quarry(mUpdateCounter);
    quarry.AddResource("steel", 40);
    quarry.AddResource("stone", 10);
    mBuildings.push_back(quarry);
    return true;
  }
  case BUILD_SPAWNER: {
    if (main_grid[cur] != mGrassId)
      return false;
    if (world_global_counts["stone"] < 30 || world_global_counts["wheat"] < 30)
      return false;
    world_global_counts["stone"] -= 30;
    world_global_counts["wheat"] -= 30;
    main_grid[cur] = mSpawnerId;
    mSpawners.push_back({cur, mUpdateCounter});
    return true;
  }
  case BUILD_FARM: {
    if (main_grid[cur] != mGrassId)
      return false;
    if (world_global_counts["wheat"] < 20 || world_global_counts["wood"] < 20)
      return false;
    world_global_counts["wheat"] -= 20;
    world_global_counts["wood"] -= 20;
    main_grid[cur] = mFarmId;
    Building farm(mUpdateCounter);
    farm.AddResource("wheat", 10);
    mBuildings.push_back(farm);
    return true;
  }
  case BUILD_TOWNHALL: {
    if (main_grid[cur] != mGrassId)
      return false;
    if (world_global_counts["wood"] < 500 || world_global_counts["stone"] < 500 ||
        world_global_counts["steel"] < 500 || world_global_counts["wheat"] < 500)
      return false;
    world_global_counts["wood"] -= 500;
    world_global_counts["stone"] -= 500;
    world_global_counts["steel"] -= 500;
    world_global_counts["wheat"] -= 500;
    main_grid[cur] = mTownhallId;
    mTownhallBuilt = true;
    mTownhallCompletionTicks = mUpdateCounter + 1;
    run_over = true;
    return true;
  }
  default:
    break;
  }

  if (action_id >= MOVE_UP && action_id <= MOVE_DOWN_RIGHT) {
    if (!main_grid.IsValid(next))
      return false;
    if (!main_grid.IsTraversable(main_grid[next]))
      return false;
    agent.SetLocation(next);
    return true;
  }

  return false;
}

namespace {

[[nodiscard]] int DynamicEfficiencyScore_(size_t ticks) {
  if (ticks == 0) return 0;
  // Fewer ticks => higher score; decays smoothly as ticks grow.
  const double base = 250000.0;
  const double scaled = base / (1.0 + 0.02 * static_cast<double>(ticks));
  return static_cast<int>(std::lround(std::max(0.0, scaled)));
}

}  // namespace

cse498::WorldScoreDisplay cse498::DynamicWorld::GetWorldScoreDisplay() const {
  WorldScoreDisplay out;
  out.lines.emplace_back("Simulation ticks (elapsed)",
                         std::to_string(mUpdateCounter));
  if (!mTownhallBuilt) {
    out.lines.emplace_back(
        "",
        "Build the Townhall to finish — no score until then.");
    return out;
  }
  out.headline = "Simulation complete — Townhall built.";
  out.lines.emplace_back("Ticks to build Townhall",
                         std::to_string(mTownhallCompletionTicks));
  out.numeric_score = DynamicEfficiencyScore_(mTownhallCompletionTicks);
  out.numeric_score_is_final = true;
  return out;
}

void cse498::DynamicWorld::UpdateWorld() {

  mUpdateCounter++;

  for(auto building : mBuildings){
    if (building.GetResources().size() == 0) continue;
    for (auto resource : building.GetResources()){
      const size_t ticks_since_built = mUpdateCounter - building.GetBuiltTime();
      if (ticks_since_built % resource.second == 0) {
        world_global_counts[resource.first] += 1;
      }
    }
  }

  // Spawner logic: spawn a PacingAgent at the closest grass cell every 60 ticks
  for (auto & [pos, built_time] : mSpawners) {
    size_t ticks_since_built = mUpdateCounter - built_time;
    if (ticks_since_built > 0 && ticks_since_built % 60 == 0) {
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
              auto & agent = AddAgent<PacingAgent>("spawned_agent");
              agent.SetLocation(spawn_pos);
              placed = true;
            }
          }
        }
      }
    }
  }
}
