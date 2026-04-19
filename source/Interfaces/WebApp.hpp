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
std::string GetUrlParam(const std::string& name,
                        const std::string& default_value = "");

class WebApp {
 public:
  /// Build the UI shell (canvas, layout, textboxes).
  /// Must be followed by Initialize<WorldT>() before Render().
  WebApp();

  /// Create the simulation world and add the human-player agent at start_pos.
  /// Returns a typed reference to the world so additional agents can be added.
  /// @tparam WorldT  Any WorldBase-derived type (must be complete at call site).
  template <typename WorldT>
  WorldT& Initialize(cse498::WorldPosition start_pos = {1.0, 1.0}) {
    world_     = std::make_unique<WorldT>();
    interface_ = &world_->AddAgent<cse498::WebInterface>("Player");
    interface_->SetLocation(start_pos);
    interface_->SetSymbol('P');
    return static_cast<WorldT&>(*world_);
  }

  /// Configure the CSS fill color and glyph for a named cell type.
  void SetCellVisual(const std::string& cell_type_name,
                     std::string fill_css, std::string glyph);

  /// Register display label, hotkey, and live-mode requirement for an action.
  void RegisterActionMeta(const std::string& action_id,
                          cse498::WebInterface::ActionMeta meta);

  /// Build the legend cache and draw the first frame.
  /// Must be called after Initialize() and all SetCellVisual()/RegisterActionMeta() calls.
  void Render();

  /// Dispatch an action code received from a JS button or keyboard event.
  void HandleAction(int action_code);

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

 private:
  // Action codes shared between C++ dispatch and the JavaScript bridge.
  // NOTE: these integer values are part of the JS/C++ ABI and must also match
  // the world's ActionType enum — any change here must be reflected in both
  // the EM_JS key/button maps in WebApp.cpp and the world's enum.
  enum ActionCode {
    kActionStart   = 1,
    kActionReset   = 2,
    kActionSave    = 3,
    kActionLoad    = 4,
    kActionUp      = 5,
    kActionDown    = 6,
    kActionLeft    = 7,
    kActionRight   = 8,
    kActionCollect = 9,
    kActionBuild   = 10,
    kActionZoomIn  = 11,
    kActionZoomOut = 12,
    kActionZoomReset = 13
  };

  // ViewPort Struct for the Zoom Functionality
  struct ViewPort {
    int x0 = 0;       // Position of the camera relative to the Canvas
    int y0 = 0;
    int width = 0;    // Width & Height of the ViewPort (controlled by the User)
    int height = 0;
  };

  int zoom_level_ = 1;
  static constexpr int kMinZoomLevel = 1;
  static constexpr int kMaxZoomLevel = 4;

  [[nodiscard]] ViewPort GetViewPort(int grid_w, int grid_h) const;
  void ChangeZoom(int delta);
  void ResetZoom() { zoom_level_ = kMinZoomLevel; };


  static std::string ActionIdForCode(int code);
  static int CodeForActionId(const std::string& action_id);

  void RenderWorld();

  // Two-layer simulation/rendering architecture:
  //   world_     — owns the grid, game rules, and all agents (including interface_)
  //   interface_ — the human-player agent inside world_; provides rendering data
  std::unique_ptr<cse498::WorldBase>   world_;
  cse498::WebInterface*                interface_ = nullptr;  // non-owning; owned by world_

  std::unique_ptr<cse498::WebLayout>   layout_;
  std::unique_ptr<cse498::WebCanvas>   canvas_;
  std::unique_ptr<cse498::WebTextbox>  hud_text_;
  std::unique_ptr<cse498::WebTextbox>  log_text_;

  // Legend is built once in Render() and reused on every subsequent frame.
  std::unordered_map<int, cse498::LegendEntry> legend_by_id_;

  // --- Persistence stack (Phase 8a) ---
  cse498::Database db_;
  cse498::WebSocketConnection ws_client_;
  cse498::SyncManager sync_{db_, ws_client_};

  std::function<void()> save_callback_;
  std::function<void()> load_callback_;

  long poll_timer_id_ = 0;

  void PerformSave();
  void PerformLoad();
};

/// Single application instance — defined in WebApp.cpp, used by
/// HandleAction (WebApp.cpp) and main() (web_main.cpp).
extern std::unique_ptr<WebApp> g_app;
