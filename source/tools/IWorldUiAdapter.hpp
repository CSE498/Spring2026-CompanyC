#pragma once

#include <map>
#include <string>
#include <vector>

namespace cse498 {

enum class UiMode {
  Setup,
  Live,
  Observe
};

struct LegendEntry {
  int id = 0;
  std::string key;
  std::string label;
  std::string fill_css;
  std::string glyph;
  bool blocks_movement = false;
};

struct RenderableCell {
  int x = 0;
  int y = 0;
  int legend_id = 0;
  bool visible = true;
  bool selected = false;
};

struct RenderableEntity {
  int id = 0;
  int x = 0;
  int y = 0;
  std::string label;
  std::string fill_css;
  std::string glyph;
  std::string image_url;  ///< Optional sprite path, empty means use fill_css circle
  bool visible = true;
};

struct UiAction {
  std::string id;
  std::string label;
  std::string hotkey;
  bool enabled = true;
};

struct HudState {
  std::string world_name;
  std::string mode;
  std::string status_message;
  std::string selected_cell;
  int tick = 0;
  std::map<std::string, int> resources;
};

class IWorldUiAdapter {
 public:
  virtual ~IWorldUiAdapter() = default;

  virtual std::string GetWorldName() const = 0;
  virtual UiMode GetMode() const = 0;
  virtual int GetWidth() const = 0;
  virtual int GetHeight() const = 0;

  virtual std::vector<LegendEntry> GetLegend() const = 0;
  virtual std::vector<RenderableCell> GetRenderableCells() const = 0;
  virtual std::vector<RenderableEntity> GetEntities() const = 0;
  virtual std::vector<UiAction> GetAvailableActions() const = 0;

  virtual void SubmitAction(const std::string& action_id) = 0;
  virtual void Tick() = 0;
  virtual HudState GetHudState() const = 0;

  virtual void SaveWorld() = 0;
  virtual void LoadWorld() = 0;

  virtual void Reset() = 0;
  virtual void SelectCell(int x, int y) = 0;
};

}  // namespace cse498
