#include "./DynamicWorld.hpp"

int cse498::DynamicWorld::DoAction(AgentBase &agent, size_t action_id) {
  WorldPosition cur = agent.GetLocation().AsWorldPosition();
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
    size_t cell = mMainGrid[cur];
    if (cell == mTreeId) {
      mWorldGlobalCounts["wood"] += 1;
      mMainGrid[cur] = mGrassId;
      return true;
    } else if (cell == mStoneId) {
      mWorldGlobalCounts["stone"] += 1;
      mMainGrid[cur] = mGrassId;
      return true;
    } else if (cell == mWheatId) {
      mWorldGlobalCounts["wheat"] += 1;
      mMainGrid[cur] = mGrassId;
      return true;
    }
    return false;
  }
  case BUILD_LUMBERYARD: {
    if (mMainGrid[cur] != mGrassId)
      return false;
    if (mWorldGlobalCounts["wood"] < 20 || mWorldGlobalCounts["steel"] < 20)
      return false;
    mWorldGlobalCounts["wood"] -= 20;
    mWorldGlobalCounts["steel"] -= 20;
    mMainGrid[cur] = mLumberyardId;
    Building b(mUpdateCounter);
    b.AddResource("wood", 20);
    mBuildings.push_back(b);
    return true;
  }
  case BUILD_QUARRY: {
    if (mMainGrid[cur] != mGrassId)
      return false;
    if (mWorldGlobalCounts["stone"] < 20 || mWorldGlobalCounts["wood"] < 20)
      return false;
    mWorldGlobalCounts["stone"] -= 20;
    mWorldGlobalCounts["wood"] -= 20;
    mMainGrid[cur] = mQuarryId;
    Building b(mUpdateCounter);
    b.AddResource("steel", 40);
    b.AddResource("stone", 10);
    mBuildings.push_back(b);
    return true;
  }
  case BUILD_SPAWNER: {
    if (mMainGrid[cur] != mGrassId)
      return false;
    if (mWorldGlobalCounts["stone"] < 30 || mWorldGlobalCounts["wheat"] < 30)
      return false;
    mWorldGlobalCounts["stone"] -= 30;
    mWorldGlobalCounts["wheat"] -= 30;
    mMainGrid[cur] = mSpawnerId;
    mSpawners.push_back({cur, mUpdateCounter});
    return true;
  }
  case BUILD_FARM: {
    if (mMainGrid[cur] != mGrassId)
      return false;
    if (mWorldGlobalCounts["wheat"] < 20 || mWorldGlobalCounts["wood"] < 20)
      return false;
    mWorldGlobalCounts["wheat"] -= 20;
    mWorldGlobalCounts["wood"] -= 20;
    mMainGrid[cur] = mFarmId;
    Building b(mUpdateCounter);
    b.AddResource("wheat", 10);
    mBuildings.push_back(b);
    return true;
  }
  case BUILD_TOWNHALL: {
    if (mMainGrid[cur] != mGrassId)
      return false;
    if (mWorldGlobalCounts["wood"] < 500 || mWorldGlobalCounts["stone"] < 500 ||
        mWorldGlobalCounts["steel"] < 500 || mWorldGlobalCounts["wheat"] < 500)
      return false;
    mWorldGlobalCounts["wood"] -= 500;
    mWorldGlobalCounts["stone"] -= 500;
    mWorldGlobalCounts["steel"] -= 500;
    mWorldGlobalCounts["wheat"] -= 500;
    mMainGrid[cur] = mTownhallId;
    mRunOver = true;
    return true;
  }
  default:
    break;
  }

  if (action_id >= MOVE_UP && action_id <= MOVE_DOWN_RIGHT) {
    if (!mMainGrid.IsValid(next))
      return false;
    if (!mMainGrid.IsTraversable(mMainGrid[next]))
      return false;
    agent.SetLocation(next);
    return true;
  }

  return false;
}

void cse498::DynamicWorld::UpdateWorld() {

  mUpdateCounter++;

  for (auto building : mBuildings) {
    // each building can start producing the resources after it's built,
    // and produces them at a rate determined by the building type.
    for (auto resource : building.GetResources()) {
      size_t ticks_since_built = mUpdateCounter - building.GetBuiltTime();
      if (ticks_since_built % resource.second == 0) {
        mWorldGlobalCounts[resource.first] += 1;
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
            if (!mMainGrid.IsValid(nx, ny)) continue;
            WorldPosition spawn_pos(nx, ny);
            if (mMainGrid[spawn_pos] == mGrassId) {
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
