#pragma once

#include <random>
#include "../core/WorldBase.hpp"
#include "../core/Building.hpp"

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
  size_t update_counter = 0;

  // vector of all the buildings in the world
  std::vector<Building> buildings;

  // CellType IDs
  size_t grass_id = 0;
  size_t tree_id = 0;
  size_t stone_id = 0;
  size_t wheat_id = 0;
  size_t quarry_id = 0;
  size_t lumberyard_id = 0;
  size_t farm_id = 0;
  size_t spawner_id = 0;
  size_t townhall_id = 0;

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
    grass_id = main_grid.AddCellType("grass", "Open buildable terrain.", '.');
    tree_id  = main_grid.AddCellType("tree", "Wood resource.", 'T');
    stone_id = main_grid.AddCellType("stone", "Stone resource.", 'S');
    wheat_id = main_grid.AddCellType("wheat", "Wheat resource.", 'W');
    quarry_id = main_grid.AddCellType("quarry", "Produces stone and steel.", 'Q');
    lumberyard_id = main_grid.AddCellType("lumberyard", "Produces wood.", 'L');
    farm_id = main_grid.AddCellType("farm", "Produces wheat.", 'F');
    spawner_id = main_grid.AddCellType("spawner", "Spawns agents.", 'A');
    townhall_id = main_grid.AddCellType("townhall", "Win condition.", 'H');
  }

  // helper function to place clusters materials
  void PlaceCluster(size_t type_id, int center_x, int center_y, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dx = -radius; dx <= radius; ++dx) {
        int x = center_x + dx;
        int y = center_y + dy;

        if (!main_grid.IsValid(x, y)) continue;

        if (dx * dx + dy * dy <= radius * radius) {
          WorldPosition pos(x, y);
          if (main_grid[pos] == grass_id) {
            main_grid[pos] = type_id;
          }
        }
      }
    }
  }
  // Reconfigure main_grid to generate the world with DynamicWorld parameters.
  // Future implementations should guarantee enough resources to make winning possible, as well
  // as implement cluster generation rather than random generation
  void GenerateWorld(size_t width, size_t height) {
    main_grid.Resize(width, height, grass_id);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> x_dist(2, static_cast<int>(width) - 3);
    std::uniform_int_distribution<int> y_dist(2, static_cast<int>(height) - 3);

    PlaceCluster(tree_id, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(tree_id, x_dist(gen), y_dist(gen), 3);

    PlaceCluster(stone_id, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(stone_id, x_dist(gen), y_dist(gen), 3);

    PlaceCluster(wheat_id, x_dist(gen), y_dist(gen), 3);
    PlaceCluster(wheat_id, x_dist(gen), y_dist(gen), 3);
  }

  // Temporary: any in-bounds tile is currently walkable.
  // Later, building occupancy / blocked-tile rules can be added here.
  int DoAction(AgentBase & agent, size_t action_id) override {
    WorldPosition cur = agent.GetLocation().AsWorldPosition();
    WorldPosition next = cur;

    switch (action_id) {
      case REMAIN_STILL: break;
      case MOVE_UP:         next = cur.Up(); break;
      case MOVE_DOWN:       next = cur.Down(); break;
      case MOVE_LEFT:       next = cur.Left(); break;
      case MOVE_RIGHT:      next = cur.Right(); break;
      case MOVE_UP_LEFT:    next = cur.Up().Left(); break;
      case MOVE_UP_RIGHT:   next = cur.Up().Right(); break;
      case MOVE_DOWN_LEFT:  next = cur.Down().Left(); break;
      case MOVE_DOWN_RIGHT: next = cur.Down().Right(); break;
      case COLLECT: break;
      case BUILD_LUMBERYARD: break;
      case BUILD_QUARRY: break;
      case BUILD_SPAWNER: break;
      case BUILD_FARM: break;
      case BUILD_TOWNHALL: break;
      default: break;
    }

    if (action_id >= MOVE_UP && action_id <= MOVE_DOWN_RIGHT) {
      if (!main_grid.IsValid(next)) return false;
      agent.SetLocation(next);
      return true;
    }

    return false;
  }

  // This will handle resource generation from buildings, as well as any other future autonomous world actions
  void UpdateWorld() override;

};

} // namespace cse498