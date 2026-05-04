/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that allows you to play a version of the game Sokoban.
 **/

#pragma once

#include <cassert>
#include <string>

#include "../core/Database.hpp"
#include "../core/WorldBase.hpp"
#include "../core/WorldHelpers.hpp"
#include "../core/WorldPosition.hpp"

namespace cse498 {

  class SokobanWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t floor_id;   ///< Easy access to floor CellType ID
    size_t wall_id;    ///< Easy access to wall CellType ID
    size_t button_id;  ///< Button to press with a crate
    size_t boulder_id; ///< Crate that can be pushed
    size_t pressed_id; ///< Crate ON button
    size_t exit_id;    ///< Exit will open up when all buttons are pressed.

    /// Where will the exit appear when the buttons are pressed?
    WorldPosition exit_pos{0,1};

    size_t level_num = 0;
    size_t cur_pressed = 0;
    size_t target_buttons = 0;

    Database* db_ = nullptr;  ///< Non-owning pointer to the persistence database (set by caller).

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

    bool HasBoulder(const WorldPosition & pos) {
      if (!main_grid.IsValid(pos)) return false; // Invalid position
      return main_grid[pos] == boulder_id ||
             main_grid[pos] == pressed_id;
    }

    bool IsOpen(const WorldPosition & pos) {
      if (!main_grid.IsValid(pos)) return false; // Invalid position
      return main_grid[pos] == floor_id ||
             main_grid[pos] == button_id ||
             main_grid[pos] == exit_id;
    }

    void AddBoulder(const WorldPosition & pos) {
      assert(IsOpen(pos));
      if (main_grid[pos] == button_id) {
        main_grid[pos] = pressed_id;
        ++cur_pressed;
      }
      else main_grid[pos] = boulder_id;
    }

    void RemoveBoulder(const WorldPosition & pos) {
      assert(HasBoulder(pos));
      if (main_grid[pos] == pressed_id) {
        main_grid[pos] = button_id;
        --cur_pressed;
      }
      else main_grid[pos] = floor_id;
    }

    WorldPosition FindOffset(const WorldPosition & from_pos, size_t action_id) {
      switch (action_id) {
      case MOVE_UP:      return from_pos.Up();
      case MOVE_DOWN:    return from_pos.Down();
      case MOVE_LEFT:    return from_pos.Left();
      case MOVE_RIGHT:   return from_pos.Right();
      }
      return from_pos;
    }

    void LoadLevel() {
      switch (level_num) {
      case 0:
        main_grid.Load(std::vector<std::string>{
          "########",
          "#      #",
          "# O  o #",
          "#      #",
          "# o  O #",
          "#      #",
          "########"} );
        exit_pos = WorldPosition{0,1};
        break;
      case 1:
        main_grid.Load(std::vector<std::string>{
          " ####   ",
          " #  ### ",
          " # O  # ",
          "### # ##",
          "#o# #  #",
          "#oO  # #",
          "#o   O #",
          "########"} );
        agent_set[0]->SetLocation(WorldPosition{2,1});
        exit_pos = WorldPosition{2,7};
        break;
      case 2:
        main_grid.Load(std::vector<std::string>{
          "  ##### ",
          "###   # ",
          "#o O  # ",
          "### Oo# ",
          "#o##O # ",
          "# # o ##",
          "#O XOOo#",
          "#   o  #",
          "########"} );
        agent_set[0]->SetLocation(WorldPosition{2,2});
        exit_pos = WorldPosition{0,6};
        break;
      case 3:
        main_grid.Load(std::vector<std::string>{
          "   #####   ",
          "####   #   ",
          "#  #O  ####",
          "# OO      #",
          "#   #O O# #",
          "### #   # #",
          " #  ##### #",
          " #  ooooo #",
          " ##########"} );
        agent_set[0]->SetLocation(WorldPosition{2,2});
        exit_pos = WorldPosition{0,4};
        break;
      case 4:
        main_grid.Load(std::vector<std::string>{
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "# #  ###  # #    O  O  O  ooo  X  X  #",
          "# #  # #  # #    O  O  O   o   XX X  #",
          " #   # #  # #     O O O    o   X XX   ",
          " #   ###  ###      O O    ooo  X  X  #",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      ",
          "                                      "} );
        agent_set[0]->SetLocation(WorldPosition{0,0});
        exit_pos = WorldPosition{0,6};
        break;
      }

      cur_pressed = main_grid.CountCells(pressed_id);
      target_buttons = main_grid.CountCells(button_id) + cur_pressed;
    }

  public:
    SokobanWorld() {
      floor_id   = main_grid.AddCellType("floor",   "Floor that agents can walk on.", ' ');
      wall_id    = main_grid.AddCellType("wall",    "Impenetrable wall.",             '#');
      button_id  = main_grid.AddCellType("button",  "Button to press", 'o');
      boulder_id = main_grid.AddCellType("boulder", "Boulder to push", 'O');
      pressed_id = main_grid.AddCellType("pressed", "Pressed button", 'X');
      exit_id    = main_grid.AddCellType("exit",    "Exit", 'E');

      LoadLevel();
    }
    ~SokobanWorld() = default;

    /// Attach the persistence database. Must be called before SaveState/LoadState.
    void SetDatabase(Database* db) { db_ = db; }

    /// Serialize current game state (grid, agent position, level progress) to the database.
    void SaveState(const std::string& world_name) {
      if (!db_) return;
      (void)SaveWorld(*db_, world_name, *this);
      (void)db_->Store("world:" + world_name + ":level_num",      static_cast<int>(level_num));
      (void)db_->Store("world:" + world_name + ":cur_pressed",    static_cast<int>(cur_pressed));
      (void)db_->Store("world:" + world_name + ":target_buttons", static_cast<int>(target_buttons));
      (void)db_->Store("world:" + world_name + ":exit_pos_x",     exit_pos.X());
      (void)db_->Store("world:" + world_name + ":exit_pos_y",     exit_pos.Y());
    }

    /// Restore game state from the database. No-op if no save exists.
    void LoadState(const std::string& world_name) {
      if (!db_) return;
      auto result = LoadWorld(*db_, world_name, *this);
      if (!result) return;

      if (auto v = db_->Load<int>("world:" + world_name + ":level_num"))
        level_num = static_cast<size_t>(*v);
      if (auto v = db_->Load<int>("world:" + world_name + ":cur_pressed"))
        cur_pressed = static_cast<size_t>(*v);
      if (auto v = db_->Load<int>("world:" + world_name + ":target_buttons"))
        target_buttons = static_cast<size_t>(*v);
      if (auto x = db_->Load<double>("world:" + world_name + ":exit_pos_x"))
        if (auto y = db_->Load<double>("world:" + world_name + ":exit_pos_y"))
          exit_pos = WorldPosition{*x, *y};
    }

    /// Allow the agents to move around the maze.
    int DoAction(AgentBase & agent, size_t action_id) override {
      // Determine where the agent is trying to move.
      WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
      WorldPosition new_position = FindOffset(cur_position, action_id);

      // If there was a boulder and it's moveable, move it!  Otherwise stop.
      if (HasBoulder(new_position)) {
        WorldPosition target_position = FindOffset(new_position, action_id);
        if (IsOpen(target_position)) { // Can push boulder!
          AddBoulder(target_position);
          RemoveBoulder(new_position);

          // If we hit the threshold, open the exit!
          if (cur_pressed == target_buttons) {
            main_grid[exit_pos] = exit_id;
          }

        } else return false;
      }

      // If position is open (after boulder is moved, if any) go there!
      if (IsOpen(new_position)) {
        agent.SetLocation(new_position);
        if (main_grid[new_position] == exit_id) {
          // Win level!
          ++level_num;
          LoadLevel();
        }
        return true;
      }      

      return false;
    }

  };

} // End of namespace cse498
