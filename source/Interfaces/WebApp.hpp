// WebApp.hpp
//
// Generic web application class that wires together WebCanvas, WebLayout,
// WebTextbox, and a WebInterface-equipped WorldBase world.
//
// Intended usage (from a world-specific web_main.cpp):
//   g_app = std::make_unique<WebApp>();            // build UI shell
//   auto& world = g_app->Initialize<MyWorld>();    // create world + agent
//   g_app->SetCellVisual("grass", "#8fd17f", ".");  // configure visuals
//   g_app->RegisterActionMeta("up", {...});          // configure actions
//   g_app->Render();                                 // draw first frame

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "tools/IWorldUiAdapter.hpp"
#include "tools/WebCanvas.hpp"
#include "tools/WebInterface.hpp"
#include "tools/WebLayout.hpp"
#include "tools/WebTextbox.hpp"
#include "core/WorldBase.hpp"
#include "core/WorldPosition.hpp"

#include <functional>

#include "core/Database.hpp"
#include "core/SyncManager.hpp"
#include "tools/WebSocketConnection.hpp"

/// Read a URL query parameter by name.  Returns the parameter value, or
/// default_value if the parameter is absent.  Available to any main driver
/// that includes WebApp.hpp.
std::string GetUrlParam(const std::string &name,
                        const std::string &default_value = "");

class WebApp
{
public:
  /// Build the UI shell (canvas, layout, textboxes).
  /// Must be followed by Initialize<WorldT>() before Render().
  WebApp();

  /// Create the simulation world and add the human-player agent at start_pos.
  /// Returns a typed reference to the world so additional agents can be added.
  /// @tparam WorldT  Any WorldBase-derived type (must be complete at call site).
  template <typename WorldT>
  WorldT &Initialize(cse498::WorldPosition start_pos = {1.0, 1.0})
  {
    world_ = std::make_unique<WorldT>();
    interface_ = &world_->AddAgent<cse498::WebInterface>("Player");
    interface_->SetLocation(start_pos);
    interface_->SetSymbol('P');
    return static_cast<WorldT &>(*world_);
  }

  /// Configure the CSS fill color and glyph for a named cell type.
  void SetCellVisual(const std::string &cell_type_name,
                     std::string fill_css, std::string glyph);

  /// Register display label, hotkey, and live-mode requirement for an action.
  void RegisterActionMeta(const std::string &action_id,
                          cse498::WebInterface::ActionMeta meta);

  /// Assign a sprite image URL to agents whose glyph character matches.
  /// Call after Initialize() and before Render().
  void RegisterEntityVisual(char glyph, std::string image_url);

  /// Register a background image to draw beneath a named cell type's sprite.
  /// e.g., RegisterCellBackground("tree", "assets/grass.png") draws grass
  /// under every tree cell before the tree sprite is drawn on top.
  void RegisterCellBackground(const std::string& cell_type_name,
                              std::string bg_image_url);

  /// Build the legend cache and draw the first frame.
  /// Must be called after Initialize() and all SetCellVisual()/RegisterActionMeta() calls.
  void Render();

  /// Dispatch an action code received from a JS button or keyboard event.
  void HandleAction(int action_code);

  /// Hide or show the player entity on the canvas.
  /// Pass false for an invisible observer/camera that only controls the viewport.
  void SetPlayerVisible(bool visible);

  /// Switch to viewport (camera) mode with a fixed cell pixel size.
  /// The viewport is centered on the player position each frame.
  /// Call after Initialize() and before Render().
  void EnableViewport(int cell_px_size = 32);

  void ShowEndGameHtml(const std::string& html);
  void HideEndGameHtml();

  /// Access the persistence Database.
  cse498::Database& GetDatabase() { return db_; }

  /// Connect to a SaveServer at the given WebSocket URL.
  /// Starts the SyncManager client and a 100ms poll timer.
  void ConnectToServer(const std::string& url);

  /// Set the callback invoked when "save" is triggered.
  /// The callback should write world state to the Database.
  void SetSaveCallback(std::function<void()> cb) { save_callback_ = std::move(cb); }

  /// Set the callback invoked when a load response arrives.
  /// The callback should read world state from the Database.
  void SetLoadCallback(std::function<void()> cb) { load_callback_ = std::move(cb); }

  /// Set the callback invoked once when the world reports IsRunOver().
  /// The callback should call ShowEndGameHtml() with the appropriate HTML.
  void SetGameOverCallback(std::function<void()> cb) { game_over_callback_ = std::move(cb); }

  /// Configure automatic simulation tick interval in milliseconds.
  /// Values are clamped to a small positive minimum for safety.
  void SetAutoTickMs(int ms);

 private:
  // Action codes shared between C++ dispatch and the JavaScript bridge.
  // NOTE: these integer values are part of the JS/C++ ABI and must also match
  // the world's ActionType enum — any change here must be reflected in both
  // the EM_JS key/button maps in WebApp.cpp and the world's enum.
  enum ActionCode {
    kActionStart     = 1,
    kActionReset     = 2,
    kActionSave      = 3,
    kActionLoad      = 4,
    kActionUp        = 5,
    kActionDown      = 6,
    kActionLeft      = 7,
    kActionRight     = 8,
    kActionCollect   = 9,
    kActionBuild     = 10,
    kActionUpLeft    = 11,
    kActionUpRight   = 12,
    kActionDownLeft  = 13,
    kActionDownRight = 14,
    kActionZoomIn    = 15,
    kActionZoomOut   = 16,
    // Group 8 (InteractionHeavyWorld) actions
    kActionPay           = 17,
    kActionBreakBoulder  = 18,
    kActionThrowUp       = 19,
    kActionThrowDown     = 20,
    kActionThrowLeft     = 21,
    kActionThrowRight    = 22,
    kActionInventory     = 23,
  };

  static std::string ActionIdForCode(int code);
  static int CodeForActionId(const std::string &action_id);

  bool IsMovementAction(int action_code) const;
  bool TryGetPlayerCell(std::pair<int, int>& out_cell) const;
  std::pair<int, int> GetAttemptedMoveCell(
      int action_code, const std::pair<int, int>& from_cell) const;
  
  // Auto tick loop
  void AdvanceSimulationTick();
  void StartAutoTickLoop();
  void StopAutoTickLoop();
  void ScheduleAutoTickStart(int delay_ms);

  void RenderWorld();
  void CenterViewportOnPlayer();
  void CenterViewportOnCell(int cell_x, int cell_y);

  // Two-layer simulation/rendering architecture:
  //   world_     — owns the grid, game rules, and all agents (including interface_)
  //   interface_ — the human-player agent inside world_; provides rendering data
  std::unique_ptr<cse498::WorldBase> world_;
  cse498::WebInterface *interface_ = nullptr; // non-owning; owned by world_

  std::unique_ptr<cse498::WebLayout>   layout_;
  std::unique_ptr<cse498::WebCanvas>   canvas_;
  std::unique_ptr<cse498::WebTextbox>  hud_text_;
  std::unique_ptr<cse498::WebTextbox>  log_text_;
  std::unique_ptr<cse498::WebTextbox>  analysis_text_;

  // Legend is built once in Render() and reused on every subsequent frame.
  std::unordered_map<int, cse498::LegendEntry> legend_by_id_;

  // Optional background image per cell type name (drawn beneath the cell sprite).
  std::unordered_map<std::string, std::string> cell_backgrounds_;

  // Viewport (camera) mode: render a fixed-cell-size window centered on player.
  bool use_viewport_      = false;
  int  focus_col_         = -1;
  int  focus_row_         = -1;
  int  viewport_cell_px_  = 32;  // pixels per cell when viewport mode is active
  int  viewport_x_        = 0;   // top-left grid column currently in view
  int  viewport_y_        = 0;   // top-left grid row currently in view

  // --- Persistence stack (Phase 8a) ---
  cse498::Database db_;
  cse498::WebSocketConnection ws_client_;
  cse498::SyncManager sync_{db_, ws_client_};

  std::function<void()> save_callback_;
  std::function<void()> load_callback_;
  std::function<void()> game_over_callback_;

  long poll_timer_id_ = 0;
  long auto_tick_timer_id_ = 0;
  long auto_tick_start_timeout_id_ = 0;
  int auto_tick_ms_ = 500;
  bool auto_tick_running_ = false;

  void PerformSave();
  void PerformLoad();
};

/// Single application instance — defined in WebApp.cpp, used by
/// HandleAction (WebApp.cpp) and main() (web_main.cpp).
extern std::unique_ptr<WebApp> g_app;