/**
 * @file MazeSolverAgent.hpp
 * @author Shashank Papani, Ahmed Ezaz Labib
 *
 * BFS maze-solving agent for MazeWorld.
 *
 * Behavior:
 *   Runs BFS from the current position each time the path is exhausted or
 *   invalidated. Targets the nearest "exit" cell if one exists, otherwise
 *   the nearest unvisited floor cell, then falls back to any reachable floor.
 *
 * Supported actions: up, down, left, right
 **/

 #pragma once

 #include <queue>
 #include <string>
 #include <unordered_map>
 #include <vector>
 #include "../core/AgentBase.hpp"
 
 namespace cse498
 {
 
     class MazeSolverAgent : public AgentBase
     {
     public:
         MazeSolverAgent(size_t id, const std::string &name, WorldBase &world)
             : AgentBase(id, name, world) {}
 
         ~MazeSolverAgent() = default;
 
         bool Initialize() override;
         size_t SelectAction(WorldGrid &grid) override;
         void Notify(const std::string &message,
                     const std::string &msg_type = "none") override;
 
     private:
         /** Planned path as a sequence of action names; consumed one step per tick. */
         std::vector<std::string> mPath;
         size_t mPathIndex = 0;
         bool mPathValid = false;
 
         /** Visited-cell map indexed by [cy * mGridW + cx]; populated on first tick. */
         std::vector<bool> mVisited;
         size_t mGridW = 0;
         size_t mGridH = 0;
 
         /** Consecutive failed moves; triggers a replan at threshold. */
         int mFailStreak = 0;
 
         /** Runs BFS from the current position and populates mPath.
          *  Targets "exit" if present, otherwise the nearest unvisited floor cell. */
         void PlanPath(const WorldGrid &grid);
 
         /** Returns true if the cell at pos is valid and traversable. */
         bool IsWalkable(const WorldGrid &grid, const WorldPosition &pos) const;
 
         /** Packs (x, y) into a single integer key for use in hash maps. */
         size_t Pack(size_t x, size_t y) const { return x * 10000 + y; }
 
         bool SupportsAction(const std::string &name) const;
         size_t LookupActionID(const std::string &name) const;
     };
 
 } // namespace cse498