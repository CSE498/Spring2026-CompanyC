/**
 * @file MazeSolverAgent.cpp
 * @author Shashank Papani, Ahmed Ezaz Labib
 *
 * BFS maze-solving agent for MazeWorld.
 **/

 #include "MazeSolverAgent.hpp"
 #include <climits>
 #include <iostream>
 #include <unordered_map>
 #include <unordered_set>
 
 namespace cse498
 {
 
     bool MazeSolverAgent::Initialize()
     {
         return SupportsAction("up") &&
                SupportsAction("down") &&
                SupportsAction("left") &&
                SupportsAction("right");
     }
 
     /** Tracks consecutive failures and invalidates the current path after two in a row. */
     void MazeSolverAgent::Notify(const std::string & /*message*/,
                                  const std::string &msg_type)
     {
         if (msg_type == "action_failed")
         {
             ++mFailStreak;
             if (mFailStreak >= 2)
             {
                 mPathValid = false;
                 mPathIndex = 0;
                 mPath.clear();
                 mFailStreak = 0;
             }
         }
         else
         {
             mFailStreak = 0;
         }
     }
 
     size_t MazeSolverAgent::SelectAction(WorldGrid &grid)
     {
         /** Initialize the visited map on the first call. */
         if (mGridW == 0)
         {
             mGridW = grid.GetWidth();
             mGridH = grid.GetHeight();
             mVisited.assign(mGridW * mGridH, false);
         }
 
         /** Mark the current cell as visited. */
         const WorldPosition cur = GetLocation().AsWorldPosition();
         const size_t cx = cur.CellX();
         const size_t cy = cur.CellY();
         if (cx < mGridW && cy < mGridH)
             mVisited[cy * mGridW + cx] = true;
 
         /** Replan if the path is invalid or fully consumed. */
         if (!mPathValid || mPathIndex >= mPath.size())
         {
             PlanPath(grid);
             mPathIndex = 0;
         }
 
         /** Fallback: wander if no path could be planned. */
         if (mPath.empty())
         {
             for (const std::string &a : {"up", "down", "left", "right"})
             {
                 if (!SupportsAction(a))
                     continue;
                 const WorldPosition npos = [&]() -> WorldPosition
                 {
                     if (a == "up")
                         return cur.Up();
                     if (a == "down")
                         return cur.Down();
                     if (a == "left")
                         return cur.Left();
                     return cur.Right();
                 }();
                 if (IsWalkable(grid, npos))
                     return LookupActionID(a);
             }
             return LookupActionID("up");
         }
 
         return LookupActionID(mPath[mPathIndex++]);
     }
 
     /**
      * BFS from the current position. Goal priority:
      *   1. Any "exit" cell — navigate directly there and stop.
      *   2. Nearest unvisited floor — explore the maze systematically.
      *   3. Any reachable floor — fallback when fully visited.
      */
     void MazeSolverAgent::PlanPath(const WorldGrid &grid)
     {
         mPath.clear();
         mPathValid = false;
 
         const WorldPosition start = GetLocation().AsWorldPosition();
         const int sx = static_cast<int>(start.CellX());
         const int sy = static_cast<int>(start.CellY());
 
         struct Node
         {
             int x, y;
             size_t parent_key;
             std::string action;
         };
 
         std::unordered_map<size_t, Node> came_from;
         std::queue<Node> frontier;
 
         const size_t root_key = Pack(sx, sy);
         came_from[root_key] = {sx, sy, SIZE_MAX, ""};
         frontier.push({sx, sy, SIZE_MAX, ""});
 
         static const std::vector<std::tuple<int, int, std::string>> kDirs = {
             {0, -1, "up"},
             {0, 1, "down"},
             {-1, 0, "left"},
             {1, 0, "right"}};
 
         size_t exit_key = SIZE_MAX;
         size_t unvisited_key = SIZE_MAX;
 
         while (!frontier.empty())
         {
             const Node cur_node = frontier.front();
             frontier.pop();
 
             const int cx = cur_node.x;
             const int cy = cur_node.y;
             const size_t cur_key = Pack(cx, cy);
 
             const WorldPosition cur_pos(static_cast<size_t>(cx),
                                         static_cast<size_t>(cy));
             if (grid.IsValid(cur_pos))
             {
                 const std::string type = grid.GetCellTypeName(grid[cur_pos]);
                 if (type == "exit")
                 {
                     exit_key = cur_key;
                     break;
                 }
 
                 /** Track the nearest unvisited floor cell in BFS order. */
                 const bool visited = (static_cast<size_t>(cx) < mGridW &&
                                       static_cast<size_t>(cy) < mGridH &&
                                       mVisited[static_cast<size_t>(cy) * mGridW +
                                                static_cast<size_t>(cx)]);
                 if (!visited && cur_key != root_key && unvisited_key == SIZE_MAX)
                     unvisited_key = cur_key;
             }
 
             for (const auto &[dx, dy, action_name] : kDirs)
             {
                 if (!SupportsAction(action_name))
                     continue;
 
                 const int nx = cx + dx;
                 const int ny = cy + dy;
                 if (nx < 0 || ny < 0)
                     continue;
 
                 const WorldPosition npos(static_cast<size_t>(nx),
                                          static_cast<size_t>(ny));
                 if (!grid.IsValid(npos) || !IsWalkable(grid, npos))
                     continue;
 
                 const size_t nkey = Pack(nx, ny);
                 if (came_from.count(nkey))
                     continue;
 
                 came_from[nkey] = {nx, ny, cur_key, action_name};
                 frontier.push({nx, ny, cur_key, action_name});
             }
         }
 
         /** Select target: exit takes priority over nearest unvisited. */
         size_t target_key = SIZE_MAX;
         if (exit_key != SIZE_MAX)
             target_key = exit_key;
         else if (unvisited_key != SIZE_MAX)
             target_key = unvisited_key;
 
         if (target_key == SIZE_MAX)
         {
             std::cout << "[MazeSolverAgent] Maze fully explored.\n";
             mPathValid = true;
             return;
         }
 
         /** Reconstruct path by walking came_from back to the root. */
         std::vector<std::string> reversed_path;
         size_t key = target_key;
         while (came_from.count(key) && came_from[key].parent_key != SIZE_MAX)
         {
             reversed_path.push_back(came_from[key].action);
             key = came_from[key].parent_key;
         }
 
         mPath.assign(reversed_path.rbegin(), reversed_path.rend());
         mPathValid = true;
 
         std::cout << "[MazeSolverAgent] Path planned: " << mPath.size() << " steps.\n";
     }
 
     /** Returns true if pos is valid and traversable. */
     bool MazeSolverAgent::IsWalkable(const WorldGrid &grid, const WorldPosition &pos) const
     {
         if (!grid.IsValid(pos))
             return false;
         const std::string type = grid.GetCellTypeName(grid[pos]);
         return type != "wall";
     }
 
     bool MazeSolverAgent::SupportsAction(const std::string &name) const
     {
         return HasAction(name);
     }
 
     size_t MazeSolverAgent::LookupActionID(const std::string &name) const
     {
         return GetActionID(name);
     }
 
 } // namespace cse498