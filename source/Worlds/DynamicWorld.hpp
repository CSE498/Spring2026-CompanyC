#pragma once

#include <random>
#include <iostream>
#include <algorithm>

#include "../core/WorldBase.hpp"
#include "../core/Building.hpp"
#include "../Agents/PacingAgent.hpp"

namespace cse498 {

/**
 * @class DynamicWorld
 * @brief A concrete implementation of WorldBase representing a dynamic resource world.
 *
 * Handles agent actions, world updates, resource generation, and building logic.
 */
class DynamicWorld : public WorldBase {
protected:

  /**
   * @enum ActionType
   * @brief Enumerates all possible actions an agent can take in the world.
   */
  enum ActionType {
    REMAIN_STILL = 0,
    MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
    MOVE_UP_LEFT, MOVE_UP_RIGHT, MOVE_DOWN_LEFT, MOVE_DOWN_RIGHT,
    COLLECT,
    BUILD_LUMBERYARD, BUILD_QUARRY, BUILD_SPAWNER, BUILD_FARM, BUILD_TOWNHALL
  };

  /**
   * @brief Execute an agent action.
   *
   * @param agent The acting agent.
   * @param action_id The selected action ID.
   * @return Result/status code of the action.
   */
  int DoAction(AgentBase & agent, size_t action_id) override;

public:

  /**
   * @brief Construct a DynamicWorld with default size and configured cell types.
   */
  DynamicWorld() : DynamicWorld(80, 80) { }
  /**
   * @brief Construct a DynamicWorld with default size and configured cell types.
   * @param width the width of the world
   * @param height the height of the world
   */
  DynamicWorld(size_t width, size_t height) {
    mWorldResourceNames = {"wood", "stone", "steel", "wheat"};
    mWorldResourceCounts = {0, 0, 0, 0};

    ConfigureCellTypes();
    GenerateWorld(width, height);
  }

  /**
   * @brief Destroy the DynamicWorld.
   */
  ~DynamicWorld() {};

  /**
   * @brief Run all agents repeatedly until an end condition is met.
   *
   * Executes agent actions and updates the world state each tick.
   * Periodically prints global resource counts.
   */
  void Run() override {
    run_over = false;
    while (!run_over) {
      RunAgents();
      UpdateWorld();

      if (mUpdateCounter % 500 == 0) {
        for (size_t i = 0; i < mWorldResourceNames.size(); ++i) {
          std::cout << mWorldResourceNames[i] << ": " << mWorldResourceCounts[i] << " ";
        }
        std::cout << std::endl;
      }
    }
  }

  /**
   * @brief Run the agents and update the world one time
   */
  void Tick() override {
      RunAgents();
      UpdateWorld();
  }

  /**
   * @brief Provides a mapping of resource name to index of that resource
   */
  size_t ResourceIndex(const std::string & resource_name) const {
    assert(resource_name == "wood" || resource_name == "stone" || resource_name == "steel" || resource_name == "wheat");
    if (resource_name == "wood") return 0;
    if (resource_name == "stone") return 1;
    if (resource_name == "steel") return 2;
    // if its not wood, stone, or steel, it must be wheat since we assert that above
    return 3;
  }

  /**
   * @brief Initialize an agent that only has the ability to move in order to be used by UI team.
  */
  void AddGhostAgent() {
    auto agent_ptr = std::make_unique<AgentBase>(agent_set.size(), "ghost", *this);
    
    if (agent_ptr->Initialize() == false) {
      std::cerr << "Failed to initialize ghost agent." << std::endl;
    }

    mGhostAgent = std::move(agent_ptr);
    AddMovementFunctions(*mGhostAgent);
  }

  /**
   * @brief Calls DoAction on the metaparameter GhostAgent given an action_id
  */
  void PerformGhostAction(size_t action_id) {
    DoAction(*mGhostAgent, action_id);
  }

  /**
   * @brief Getters for Ids
   */
  size_t GetGrassId()      const { return mGrassId; }
  size_t GetTreeId()       const { return mTreeId; }
  size_t GetStoneId()      const { return mStoneId; }
  size_t GetWheatId()      const { return mWheatId; }
  size_t GetQuarryId()     const { return mQuarryId; }
  size_t GetLumberyardId() const { return mLumberyardId; }
  size_t GetFarmId()       const { return mFarmId; }
  size_t GetSpawnerId()    const { return mSpawnerId; }
  size_t GetTownhallId()   const { return mTownhallId; }

private:

  /// @brief Counts how many times UpdateWorld() has been called.
  size_t mUpdateCounter = 0;

  /// @brief The tick counter cutoff for ending the run
  size_t mCutoffTime = 15000;

  /// @brief Stores all buildings currently in the world.
  std::vector<Building> mBuildings;

  /// @brief Tracks spawner positions and their last spawn tick.
  std::vector<std::pair<WorldPosition, size_t>> mSpawners;

  /// @brief Indicates whether the leader agent has been assigned.
  bool mLeaderAgentSet = false;

  /// @brief An agent that only has movement capabilities (For UI purposes)
  std::unique_ptr<AgentBase> mGhostAgent;

  /// @brief Cell type IDs for terrain and structures.
  size_t mGrassId = 0;
  size_t mTreeId = 0;
  size_t mStoneId = 0;
  size_t mWheatId = 0;
  size_t mQuarryId = 0;
  size_t mLumberyardId = 0;
  size_t mFarmId = 0;
  size_t mSpawnerId = 0;
  size_t mTownhallId = 0;

  void AddMovementFunctions(AgentBase & agent) {
    agent.AddAction("up", MOVE_UP);
    agent.AddAction("down", MOVE_DOWN);
    agent.AddAction("left", MOVE_LEFT);
    agent.AddAction("right", MOVE_RIGHT);
    agent.AddAction("up_left", MOVE_UP_LEFT);
    agent.AddAction("up_right", MOVE_UP_RIGHT);
    agent.AddAction("down_left", MOVE_DOWN_LEFT);
    agent.AddAction("down_right", MOVE_DOWN_RIGHT);
  }

   /** @brief Configure an agent with available actions.
   *
   * The first agent added becomes the "leader" and gains build abilities.
   *
   * @param agent The agent to configure.
   */
  void ConfigAgent(AgentBase & agent) override {
    if (!mLeaderAgentSet) {
      agent.AddAction("build_lumberyard", BUILD_LUMBERYARD);
      agent.AddAction("build_quarry", BUILD_QUARRY);
      agent.AddAction("build_spawner", BUILD_SPAWNER);
      agent.AddAction("build_farm", BUILD_FARM);
      agent.AddAction("build_townhall", BUILD_TOWNHALL);
      mLeaderAgentSet = true;
    }

    AddMovementFunctions(agent);
    agent.AddAction("collect", COLLECT);
  }

  /**
   * @brief Initialize all cell types used in the world grid.
   */
  void ConfigureCellTypes() {
    mGrassId = main_grid.AddCellType("grass", "Open buildable terrain.", '.');
    mTreeId  = main_grid.AddCellType("tree", "Wood resource.", 'T');
    mStoneId = main_grid.AddCellType("stone", "Stone resource.", 'S');
    mWheatId = main_grid.AddCellType("wheat", "Wheat resource.", 'W');
    mQuarryId = main_grid.AddCellType("quarry", "Produces stone and steel.", 'Q', false);
    mLumberyardId = main_grid.AddCellType("lumberyard", "Produces wood.", 'L', false);
    mFarmId = main_grid.AddCellType("farm", "Produces wheat.", 'F', false);
    mSpawnerId = main_grid.AddCellType("spawner", "Spawns agents.", 'A', false);
    mTownhallId = main_grid.AddCellType("townhall", "Win condition.", 'H', false);
  }

  /**
   * @brief Place a circular cluster of a given resource type.
   *
   * @param type_id Cell type to place.
   * @param center_x X-coordinate of cluster center.
   * @param center_y Y-coordinate of cluster center.
   * @param radius Radius of the cluster.
   */
  void PlaceCluster(size_t type_id, int center_x, int center_y, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dx = -radius; dx <= radius; ++dx) {
        int x = center_x + dx;
        int y = center_y + dy;

        if (!main_grid.IsValid(x, y)) continue;

        if (dx * dx + dy * dy <= radius * radius) {
          WorldPosition pos(x, y);
          if (main_grid[pos] == mGrassId) {
            main_grid[pos] = type_id;
          }
        }
      }
    }
  }

  /**
   * @brief Generate the world grid with clustered resource placement.
   *
   * @param width Width of the world.
   * @param height Height of the world.
   */
  void GenerateWorld(size_t width, size_t height) {

    assert(mGrassId != 0 && mTreeId != 0 && mStoneId != 0 && mWheatId != 0); // Ensure cell types are configured before generating world.
    assert(mQuarryId != 0 && mLumberyardId != 0 && mFarmId != 0 && mSpawnerId != 0 && mTownhallId != 0); // Ensure all building types are configured.

    main_grid.Resize(width, height, mGrassId);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> x_dist(2, static_cast<int>(width) - 3);
    std::uniform_int_distribution<int> y_dist(2, static_cast<int>(height) - 3);

    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 2);
    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 4);
    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 4);
    PlaceCluster(mTreeId, x_dist(gen), y_dist(gen), 5);

    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 2);
    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 4);
    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 4);
    PlaceCluster(mStoneId, x_dist(gen), y_dist(gen), 5);

    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 2);
    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 4);
    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 4);
    PlaceCluster(mWheatId, x_dist(gen), y_dist(gen), 5);
  }

  /**
   * @brief Update the world state each tick.
   *
   * Handles resource production, building updates, and global events.
   */
  void UpdateWorld() override;

  /** 
   * @brief  Override to return the current tick count for use in agents or buildings.
   * @return the current tick count since the world started running.
   */
  [[nodiscard]] size_t GetTickCount() const override { return mUpdateCounter; }

};

} // namespace cse498