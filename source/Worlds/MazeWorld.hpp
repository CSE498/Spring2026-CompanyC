/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>

#include "../core/WorldBase.hpp"

namespace cse498 {

  class MazeWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id;  ///< Easy access to wall CellType ID.
    size_t exit_id;  ///< Exit cell — stepping on it ends the game.

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    MazeWorld() {
      floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
      wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",             '#');
      exit_id  = main_grid.AddCellType("exit",  "Exit — reach it to win.",        'E');

      main_grid.Load(std::vector<std::string>{"#######################",
                                              "# #            ##     #",
                                              "# #  #  ######    ### #",
                                              "# #  #  #     #  #  # #",
                                              "# #  #  #  #  #  #  # #",
                                              "#    #     #     #    #",
                                              "##################  # #",
                                              "#                    ##",
                                              "#                    ##",
                                              "#E ####################",
                                              "#######################"} );
    }
    ~MazeWorld() = default;

    /// Allow the agents to move around the maze.
    int DoAction(AgentBase & agent, size_t action_id) override {
      // Determine where the agent is trying to move.
      WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
      WorldPosition new_position;
      switch (action_id) {
      case REMAIN_STILL: new_position = cur_position; break;
      case MOVE_UP:      new_position = cur_position.Up(); break;
      case MOVE_DOWN:    new_position = cur_position.Down(); break;
      case MOVE_LEFT:    new_position = cur_position.Left(); break;
      case MOVE_RIGHT:   new_position = cur_position.Right(); break;
      }

      // Don't let the agent move off the world or into a wall.
      if (!main_grid.IsValid(new_position)) { return false; }
      if (main_grid[new_position] == wall_id) { return false; }

      // Set the agent to its new position.
      agent.SetLocation(new_position);

      // Win condition: the interface (player) steps onto the exit cell.
      if (agent.IsInterface() && main_grid[new_position] == exit_id) {
        agent.Notify("You reached the exit!", "status");
        run_over = true;
      }

      return true;
    }

  };

} // End of namespace cse498
