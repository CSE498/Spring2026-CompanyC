#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/InterfaceBase.hpp"
#include "tools/IWorldUiAdapter.hpp"
#include "tools/WebPopup.hpp"

namespace cse498 {

/// @brief Human-player interface bridging WorldBase/AgentBase to the web UI.
///
/// WebInterface is added to a WorldBase-derived world as an agent via
/// WorldBase::AddAgent<WebInterface>().  On each world turn, SelectAction()
/// returns whatever action the human most recently submitted via SubmitAction()
/// and resets the pending action to 0 (no action).
///
/// The rendering helpers (GetRenderableCells, GetEntities, GetLegend, etc.)
/// read world state through the inherited const WorldBase& reference, so any
/// WorldBase-derived world can be rendered without a separate adapter class.
///
/// Cell colors and action metadata are not part of the simulation engine and
/// must be configured by the caller (e.g., WebMain) via SetCellVisual() and
/// RegisterActionMeta() before the first render.
class WebInterface : public InterfaceBase {
 public:
  /// Rendering properties for a single cell type (visual layer only).
  struct CellVisual {
    std::string fill_css;  ///< CSS background color (e.g., "#8fd17f")
    std::string glyph;     ///< Symbol drawn on the cell (e.g., ".")
  };

  /// UI metadata for one action button (visual / UX layer only).
  struct ActionMeta {
    std::string label;          ///< Button label shown to the user (e.g., "Up")
    std::string hotkey;         ///< Keyboard shortcut hint (e.g., "W")
    bool requires_live = true;  ///< If true, the button is disabled in Setup mode
  };

  WebInterface(size_t id, const std::string& name, const WorldBase& world);
  ~WebInterface() override = default;

  // -- AgentBase overrides --

  /// Returns the pending action ID and resets it to 0 (no action).
  [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;

  /// Called by the world to deliver status messages and resource/mode updates.
  /// @param message  Notification payload (format depends on msg_type).
  /// @param msg_type One of: "status", "mode_change", "tick", "tick_reset",
  ///                         "resource" ("key:value"), "world_name",
  ///                         "popup", "popup_timed", "popup_timed_ok".
  /// For "popup_timed" / "popup_timed_ok", message format is optional
  /// "<milliseconds>|<body>" (default 1500 ms if no pipe).
  void Notify(const std::string& message,
              const std::string& msg_type) override;

  /// Removes and returns all queued popups (message + options). FIFO.
  [[nodiscard]] std::vector<WebPopupRequest> TakePendingPopups();

  // -- UI control --

  /// Queue the next action by its string ID as registered by the world.
  void SubmitAction(const std::string& action_id);

  /// Record which cell the human has highlighted (call on canvas click).
  void SelectCell(int x, int y);

  /// Return the currently highlighted cell coordinates.
  [[nodiscard]] std::pair<int, int> GetSelectedCell() const;

  /// Return custom text, if any, that the world wants shown in the single
  /// resources/details panel.
  [[nodiscard]] const std::string& GetPanelText() const;

  // -- Visual configuration (set up after AddAgent, before first render) --

  /// Assign CSS color and glyph to a named cell type.
  void SetCellVisual(const std::string& cell_type_name,
                     std::string fill_css, std::string glyph);

  /// Assign a sprite image URL to an agent glyph character.
  /// When an entity's glyph matches, its image_url is set instead of drawing
  /// the default filled circle.
  void SetEntityVisual(char glyph, std::string image_url);

  /// Register display label, hotkey hint, and enabled-mode for an action.
  void RegisterActionMeta(const std::string& action_id, ActionMeta meta);

  /// Hide (false) or show (true) the player entity in GetEntities().
  /// Set false for an invisible observer/camera agent.
  void SetPlayerVisible(bool visible);

  // -- Rendering data accessors (called by WebMain each frame) --

  [[nodiscard]] std::vector<LegendEntry>      GetLegend()           const;
  [[nodiscard]] std::vector<RenderableCell>   GetRenderableCells()  const;
  [[nodiscard]] std::vector<RenderableEntity> GetEntities()         const;
  [[nodiscard]] std::vector<UiAction>         GetAvailableActions() const;
  [[nodiscard]] HudState                      GetHudState()         const;

  [[nodiscard]] int GetGridWidth()  const;
  [[nodiscard]] int GetGridHeight() const;

 private:
  size_t      pending_action_ = 0;
  UiMode      mode_           = UiMode::Setup;
  int         tick_           = 0;
  int         selected_x_     = 0;
  int         selected_y_     = 0;
  std::string status_message_;
  std::string world_name_     = "World";
  std::map<std::string, int>                  resources_;
  std::string                                 panel_text_;
  std::unordered_map<std::string, CellVisual> cell_visuals_;
  std::unordered_map<std::string, ActionMeta> action_metas_;
  std::unordered_map<char, std::string>       entity_visuals_;  // glyph → image URL
  bool                                        player_visible_ = true;
  std::vector<WebPopupRequest>                popup_queue_;
};

}  // namespace cse498
