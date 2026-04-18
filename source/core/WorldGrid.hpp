/**
 * @author Jose Hernandez
 * @brief A simple 2D Grid container
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

#include "Location.hpp"
#include "../tools/io_utils.hpp"
#include "../tools/SharedKnowledge.hpp"

namespace cse498
{

  /// @brief Simple data structure to hold info about a Cell.
  struct CellType
  {
    std::string name;        ///< Unique name for this type of cell (e.g., "wall", "tree", "moon")
    std::string desc;        ///< Full description of what this type of cell is
    char symbol;             ///< Symbol for text representations (files or interface)
    bool traversable = true; ///< Can agents move through it.
  };

  /// @class WorldGrid
  /// @brief Represents a 2D grid of cells.
  /// This class provides utilities to manage, access, and modify cells within a grid.

  class WorldGrid
  {
  protected:
    size_t width = 0;  ///< Number of cells in each row of the grid.
    size_t height = 0; ///< Number of rows of cells in the grid.

    // Track all of the possible types of cells that can be in this grid.
    // A "cell ID" refers to the index in this cell_types vector.
    std::vector<CellType> cell_types; ///< Types of cells possible in this grid.

    // Cell name -> id
    // Translate a name to an ID
    // Fast lookup from cell name -> id
    std::unordered_map<std::string, size_t> name_to_id;

    std::vector<size_t> cells; ///< Matrix of world cells, by rows, top to bottom; value is cell ID.

    

    // -- Helper functions --

    ///  Convert an X and a Y value to the index in the vector.
    [[nodiscard]] size_t ToIndex(size_t x, size_t y) const
    {
      return x + y * width;
    }


    /// Convert a WorldPosition to the index in the vector.
    [[nodiscard]] size_t ToIndex(WorldPosition pos) const
    {
      return pos.CellX() + pos.CellY() * width;
    }

    /// @brief Build symbol -> ID map for text loading.
    [[nodiscard]] std::unordered_map<char, size_t> BuildSymbolMap() const
    {
      std::unordered_map<char, size_t> symbol_map;
      for (size_t i = 0; i < cell_types.size(); ++i)
      {
        symbol_map[cell_types[i].symbol] = i;
      }
      return symbol_map;
    }

    // -- Serialize and Deserialize functions --
    // Mechanisms to efficiently save and load the exact state of the grid.
    // File format is width and height followed by all
    // values in the grid on each line thereafter.

    std::string GetTypeName() const { return "cse498::WorldGrid"; }

    /// Write the current state of this grid into the provided stream.
    void Serialize(std::ostream & os) const {
      os << width << " " << height;
      for (size_t state : cells) os << ' ' << state;
      os << std::endl;
    }

    /// Read the state of the grid out of the provided stream. 
    void Deserialize(std::istream & is) {
      is >> width >> height;
      cells.resize(width * height);
      for (size_t & state : cells) is >> state;
    }
  
  public:
    WorldGrid(size_t width, size_t height, size_t default_type = 0)
        : width(width), height(height), cells(width * height, default_type)
    {
      // The first cell type (ID 0) is reserved for errors or non-existant
      // positions in a cell grid (e.g., for a non-rectangular world).
      AddCellType("Unknown", "This is an invalid cell type and should not be reachable.", '?', false);

    }

    WorldGrid() : WorldGrid(0, 0, 0) {}
    WorldGrid(const WorldGrid &) = default;
    WorldGrid(WorldGrid &&) = default;

    WorldGrid &operator=(const WorldGrid &) = default;
    WorldGrid &operator=(WorldGrid &&) = default;

    SharedKnowledge sharedKnowledge;



    SharedKnowledge& GetSharedKnowledge() {
        return sharedKnowledge;
    }

    const SharedKnowledge& GetSharedKnowledge() const {
        return sharedKnowledge;
    }

    // -- Accessors --
    [[nodiscard]] size_t GetWidth() const { return width; }
    [[nodiscard]] size_t GetHeight() const { return height; }
    [[nodiscard]] size_t GetNumCells() const { return cells.size(); }

    /// Test if specific coordinates are in range for this GridWorld.
    [[nodiscard]] bool IsValid(double x, double y) const
    {
      return x >= 0.0 && x < width && y >= 0.0 && y < height;
    }

    /// Test if a WorldPosition is in range for this GridWorld.
    [[nodiscard]] bool IsValid(WorldPosition pos) const
    {
      return IsValid(pos.X(), pos.Y());
    }

    /// @return The grid state at the provided x and y coordinates
    [[nodiscard]] size_t operator[](size_t x, size_t y) const
    {
      assert(IsValid(x, y));
      return cells[ToIndex(x, y)];
    }

    /// @return A reference to the grid state at the provided x and y coordinates
    [[nodiscard]] size_t &operator[](size_t x, size_t y)
    {
      assert(IsValid(x, y));
      return cells[ToIndex(x, y)];
    }

    /// @brief Read a cell by its position
    ///  Added bounds asserts for WorldPosition access too
    [[nodiscard]] size_t operator[](WorldPosition pos) const
    {
      assert(IsValid(pos));
      return cells[ToIndex(pos)];
    }

    /// @brief Write a cell by position.
    [[nodiscard]] size_t &operator[](WorldPosition pos)
    {
      assert(IsValid(pos));
      return cells[ToIndex(pos)];
    }

    // ===========================
    //   Cell type management...
    // ===========================

    [[nodiscard]] const std::vector<CellType> &GetCellTypes() const { return cell_types; }

    /// @brief  Configure a new type of cell that can be in this grid.
    /// @param name A unique name for this cell type
    /// @param desc A longer description of the cell type
    /// @param symbol An (optional) unique symbol for text IO (files, command line)
    /// @return A unique ID associated with this cell type (position in type_options vector)
    size_t AddCellType(const std::string &name,
                       const std::string &desc = "",
                       char symbol = '\0',
                       bool traversable = true)
    {
      auto it = name_to_id.find(name);
      if (it != name_to_id.end())
        return it->second;

      const size_t id = cell_types.size();
      cell_types.push_back(CellType{name, desc, symbol, traversable});
      name_to_id[name] = id;
      return id;
    }
    /// @brief Return the ID associated with the cell type name.
    /// @param name The unique name of the cell type
    /// @return The unique ID of the cell type (or 0 if it doesn't exist.)
    [[nodiscard]] size_t GetCellTypeID(const std::string &name) const
    {
      // O(1)-style map lookup instead of looping through vector
      auto it = name_to_id.find(name);
      return (it == name_to_id.end()) ? 0 : it->second;
    }

    [[nodiscard]] const std::string &GetCellTypeName(size_t id) const
    {
      if (id >= cell_types.size())
        return cell_types[0].name;
      return cell_types[id].name;
    }

    [[nodiscard]] const CellType &GetCellInfo(size_t id) const
    {
      if (id >= cell_types.size())
        return cell_types[0];
      return cell_types[id];
    }
    // CHANGED: added desc getter
    [[nodiscard]] const std::string &GetCellTypeDesc(size_t id) const
    {
      return GetCellInfo(id).desc;
    }

    [[nodiscard]] char GetCellTypeSymbol(size_t id) const
    {
      return GetCellInfo(id).symbol;
    }

    // CHANGED: added traversable getter
    [[nodiscard]] bool IsTraversable(size_t id) const
    {
      return GetCellInfo(id).traversable;
    }

    // Size adjustments.
    void Resize(size_t new_width, size_t new_height, size_t default_type = 0)
    {
      // Create a new vector of the correct size.
      std::vector<size_t> new_cells(new_width * new_height, default_type);

      // Copy the overlapping portions of the two grids.
      const size_t min_width = std::min(width, new_width);
      const size_t min_height = std::min(height, new_height);
      for (size_t x = 0; x < min_width; ++x)
      {
        for (size_t y = 0; y < min_height; ++y)
        {
          new_cells[x + y * new_width] = cells[ToIndex(x, y)];
        }
      }

      // Swap the new grid in; let the old grid be deallocated in its place.
      std::swap(cells, new_cells);
      width = new_width;
      height = new_height;
    }

    [[nodiscard]] char GetSymbol(WorldPosition pos) const
    {
      assert(IsValid(pos));
      const size_t cell_id = cells[ToIndex(pos)];
      return cell_types[cell_id].symbol;
    }

    // -- Read and Write functions --
    // These are the same idea as Save and Load, but they are human readable, but they
    // also require that each state has been assigned a unique character symbol.

    /// @brief Write out a human-readable version of the current WorldGrid
    /// @param os Stream to write to
    /// @param types A vector of CellTypes for symbol identification
    void Print(std::ostream &os) const
    {
      size_t cell_id = 0;
      for (size_t y = 0; y < height; ++y)
      {
        for (size_t x = 0; x < width; ++x)
        {
          os << cell_types[cells[cell_id++]].symbol;
        }
        os << '\n';
      }
      os.flush();
    }
    void Load(const std::vector<std::string> &str_grid)
    {
      height = str_grid.size();
      width = height ? str_grid[0].size() : 0;
      cells.assign(width * height, 0);

      auto symbol_map = BuildSymbolMap();
      size_t cell_id = 0;

      for (size_t y = 0; y < height; ++y)
      {
        for (size_t x = 0; x < width; ++x)
        {
          if (x < str_grid[y].size())
          {
            // CHANGED: safer lookup; unknown symbols map to 0
            const auto it = symbol_map.find(str_grid[y][x]);
            cells[cell_id++] = (it == symbol_map.end()) ? 0 : it->second;
          }
          else
          {
            cells[cell_id++] = 0;
          }
        }
      }
    }

    void Load(std::istream &is)
    {
      auto str_grid = LoadStream(is);
      Load(str_grid);
    }
  };

} // End of namespace cse498
