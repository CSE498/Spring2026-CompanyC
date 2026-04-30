#include <cstdlib>
#include <iostream>
#include <string>

#include "Worlds/StubWorld.hpp"
#include "core/WorldPosition.hpp"
#include "tools/WebInterface.hpp"

namespace {

using cse498::StubWorld;
using cse498::WebInterface;
using cse498::WorldPosition;

void Fail(const std::string& message) {
  std::cerr << "FAIL: " << message << std::endl;
  std::exit(1);
}

void ExpectTrue(bool condition, const std::string& message) {
  if (!condition) {
    Fail(message);
  }
}

template <typename T, typename U>
void ExpectEq(const T& actual, const U& expected, const std::string& message) {
  if (!(actual == expected)) {
    std::cerr << "FAIL: " << message
              << "\n  expected: " << expected
              << "\n  actual:   " << actual << std::endl;
    std::exit(1);
  }
}

}  // namespace

int main() {
  StubWorld world;
  auto& ui = world.AddAgent<WebInterface>("Player");
  ui.SetSymbol('P');
  ui.SetLocation(WorldPosition{1, 1});

  // Populate both resources and panel text. WebApp should prefer panel text
  // when it exists, but resource storage should still remain intact.
  ui.Notify("wood:4", "resource");
  ui.Notify("stone:2", "resource");
  ui.Notify("", "panel_clear");
  ui.Notify("Tile: Wheat", "panel_line");
  ui.Notify("Harvestable: Yes", "panel_line");

  const auto hud = ui.GetHudState();

  ExpectEq(hud.resources.at("wood"), 4,
           "resource map should still contain wood while panel text is active");
  ExpectEq(hud.resources.at("stone"), 2,
           "resource map should still contain stone while panel text is active");
  ExpectEq(ui.GetPanelText(), std::string("Tile: Wheat\nHarvestable: Yes"),
           "panel text should preserve appended lines in order");

  // Malformed resource input should not corrupt existing resource state.
  ui.Notify("not_a_key_value_pair", "resource");
  const auto malformed_hud = ui.GetHudState();
  ExpectEq(malformed_hud.resources.at("wood"), 4,
           "malformed resource input should not change existing wood value");
  ExpectEq(malformed_hud.resources.at("stone"), 2,
           "malformed resource input should not change existing stone value");
  ExpectEq(malformed_hud.resources.size(), hud.resources.size(),
           "malformed resource input without a delimiter should be ignored");

  ui.Notify("", "panel_clear");
  ExpectTrue(ui.GetPanelText().empty(),
             "panel_clear should empty the panel after it was previously populated");

  // Re-clearing an already empty panel should remain safe and stable.
  ui.Notify("", "panel_clear");
  ExpectTrue(ui.GetPanelText().empty(),
             "panel_clear should be safe to call repeatedly on an empty panel");

  // After appending lines, panel_text should still replace the full contents.
  ui.Notify("Wood: 4", "panel_line");
  ui.Notify("Stone: 2", "panel_line");
  ExpectEq(ui.GetPanelText(), std::string("Wood: 4\nStone: 2"),
           "panel_line should rebuild panel contents after a clear");

  ui.Notify("Direct replacement", "panel_text");
  ExpectEq(ui.GetPanelText(), std::string("Direct replacement"),
           "panel_text should replace previously appended lines");

  std::cout << "WebInterfacePanelRegressionTest passed.\n";
  return 0;
}