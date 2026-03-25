#pragma once

#include <map>
#include <string>
#include <vector>

#include "tools/IWorldUiAdapter.hpp"

namespace cse498 {

class StubWorldAdapter : public IWorldUiAdapter {
 public:
  StubWorldAdapter();
  ~StubWorldAdapter() override = default;

  std::string GetWorldName() const override;
  UiMode GetMode() const override;
  int GetWidth() const override;
  int GetHeight() const override;

  std::vector<LegendEntry> GetLegend() const override;
  std::vector<RenderableCell> GetRenderableCells() const override;
  std::vector<RenderableEntity> GetEntities() const override;
  std::vector<UiAction> GetAvailableActions() const override;

  void SubmitAction(const std::string& action_id) override;
  void Tick() override;
  HudState GetHudState() const override;

  void SaveWorld() override;
  void LoadWorld() override;

  void Reset() override;
  void SelectCell(int x, int y) override;

 private:
  void InitializeWorld_();
  bool InBounds_(int x, int y) const;
  bool IsWalkable_(int x, int y) const;
  int CellAt_(int x, int y) const;
  int& CellAt_(int x, int y);
  bool TryCollectAt_(int x, int y);
  std::string FormatSelectedCell_() const;
  void SetStatus_(const std::string& message);

  static constexpr int kWidth = 10;
  static constexpr int kHeight = 10;

  // Legend ids
  static constexpr int kGrass = 0;
  static constexpr int kTree = 1;
  static constexpr int kStone = 2;
  static constexpr int kWheat = 3;
  static constexpr int kWall = 4;
  static constexpr int kBuilt = 5;

  std::vector<int> grid_;
  int player_x_ = 1;
  int player_y_ = 1;
  int selected_x_ = 1;
  int selected_y_ = 1;
  UiMode mode_ = UiMode::Setup;
  int tick_ = 0;
  std::string status_message_;
  std::map<std::string, int> resources_;
};

}  // namespace cse498
