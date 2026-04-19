/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>

#include "../core/WorldBase.hpp"
#include "../core/WorldScore.hpp"

namespace cse498 {

class MazeWorld : public WorldBase {
 protected:
  enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

  size_t floor_id = 0; ///< Easy access to floor CellType ID.
  size_t wall_id = 0;  ///< Easy access to wall CellType ID.
  size_t exit_id = 0;  ///< Goal tile for maze scoring.

  /// Tick counter advanced in UpdateWorld() each simulation step (web: each action round).
  size_t maze_ticks_ = 0;
  bool maze_exit_reached_ = false;
  size_t maze_exit_completion_ticks_ = 0;

  /// Provide the agent with movement actions.
  void ConfigAgent(AgentBase & agent) override {
    agent.AddAction("up", MOVE_UP);
    agent.AddAction("down", MOVE_DOWN);
    agent.AddAction("left", MOVE_LEFT);
    agent.AddAction("right", MOVE_RIGHT);
  }

  void UpdateWorld() override {
    if (!maze_exit_reached_) {
      ++maze_ticks_;
    }
  }

  [[nodiscard]] WorldScoreDisplay GetWorldScoreDisplay() const override;

 public:
  MazeWorld() {
    floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
    wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",             '#');
    exit_id  = main_grid.AddCellType("exit",  "Maze exit",                       'E');

    main_grid.Load(std::vector<std::string>{"#######################",
                                            "# #            ##     #",
                                            "# #  #  ######    ### #",
                                            "# #  #  #     #  #  # #",
                                            "# #  #  #  #  #  #  # #",
                                            "#    #     #     #    #",
                                            "##################  # #",
                                            "#                    ##",
                                            "#                    ##",
                                            "#  ####################",
                                            "#######################"} );
    // Goal: reachable floor tile on the east corridor (row 7).
    const WorldPosition exit_pos{20, 7};
    assert(main_grid.IsValid(exit_pos));
    main_grid[exit_pos] = exit_id;
  }
  ~MazeWorld() override = default;

  /// Allow the agents to move around the maze.
  int DoAction(AgentBase & agent, size_t action_id) override {
    WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
    WorldPosition new_position;
    switch (action_id) {
    case REMAIN_STILL: new_position = cur_position; break;
    case MOVE_UP:      new_position = cur_position.Up(); break;
    case MOVE_DOWN:    new_position = cur_position.Down(); break;
    case MOVE_LEFT:    new_position = cur_position.Left(); break;
    case MOVE_RIGHT:   new_position = cur_position.Right(); break;
    default:           new_position = cur_position; break;
    }

    if (!main_grid.IsValid(new_position)) { return false; }
    if (main_grid[new_position] == wall_id) { return false; }

    agent.SetLocation(new_position);

    if (!maze_exit_reached_ && main_grid[new_position] == exit_id) {
      maze_exit_reached_ = true;
      maze_exit_completion_ticks_ = maze_ticks_ + 1;
    }

    return true;
  }

};

inline WorldScoreDisplay MazeWorld::GetWorldScoreDisplay() const {
  auto maze_efficiency_score = [](size_t ticks) -> int {
    if (ticks == 0) return 0;
    const double base = 200000.0;
    const double scaled = base / (1.0 + 0.03 * static_cast<double>(ticks));
    return static_cast<int>(std::lround(std::max(0.0, scaled)));
  };

  WorldScoreDisplay out;
  if (!maze_exit_reached_) {
    out.lines.emplace_back("Maze ticks (elapsed)",
                           std::to_string(maze_ticks_));
    out.lines.emplace_back(
        "",
        "Reach the exit — no score until the maze is completed.");
    return out;
  }
  out.headline = "Maze complete";
  out.lines.emplace_back("Total ticks to exit",
                         std::to_string(maze_exit_completion_ticks_));
  out.numeric_score = maze_efficiency_score(maze_exit_completion_ticks_);
  out.numeric_score_is_final = true;
  return out;
}

} // End of namespace cse498
