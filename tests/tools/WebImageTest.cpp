#include <cassert>
#include <iostream>
#include <utility>

#include "tools/WebImage.h"
#include "tools/WebAssetKeys.hpp"

using cse498::WebImage;

static void TestDefaults() {
  WebImage img;
  assert(img.GetSource().empty());
  assert(img.GetAltText().empty());
  assert(img.GetLeftPx() == 0.0);
  assert(img.GetTopPx() == 0.0);
  assert(img.GetWidthPx() == 0.0);
  assert(img.GetHeightPx() == 0.0);
  assert(img.IsVisible());
  assert(img.GetOpacity() == 1.0);
  assert(img.GetZIndex() == 0);
  assert(img.GetParentElementId().empty());
  assert(img.GetElementId().empty());
  assert(!img.IsCreated());
}

static void TestSetters() {
  WebImage img;
  img.SetSource(cse498::web_assets::AssetPath(cse498::web_assets::kPlayer));
  img.SetAltText("a player icon");
  img.SetPositionPx(12.5, 99.0);
  img.SetSizePx(320.0, 240.0);
  img.SetVisible(false);
  img.SetOpacity(0.25);
  img.SetZIndex(42);
  img.SetParentElementId("root");
  img.SetElementId("hero-image");

  assert(img.GetSource() == "assets/player.png");
  assert(img.GetAltText() == "a player icon");
  assert(img.GetLeftPx() == 12.5);
  assert(img.GetTopPx() == 99.0);
  assert(img.GetWidthPx() == 320.0);
  assert(img.GetHeightPx() == 240.0);
  assert(!img.IsVisible());
  assert(img.GetOpacity() == 0.25);
  assert(img.GetZIndex() == 42);
  assert(img.GetParentElementId() == "root");
  assert(img.GetElementId() == "hero-image");
}

static void TestOpacityClamp() {
  WebImage img;
  img.SetOpacity(-10.0);
  assert(img.GetOpacity() == 0.0);
  img.SetOpacity(2.0);
  assert(img.GetOpacity() == 1.0);
}

static void TestClassesAndStyles() {
  WebImage img;
  assert(!img.HasCssClass("rounded"));
  img.AddCssClass("rounded");
  assert(img.HasCssClass("rounded"));
  img.AddCssClass("rounded");
  assert(img.HasCssClass("rounded"));
  img.RemoveCssClass("rounded");
  assert(!img.HasCssClass("rounded"));

  img.SetStyle("border", "1px solid red");
  img.SetStyle("pointerEvents", "none");
  img.ClearStyle("border");
  img.SetStyle("", "ignored");
  img.ClearStyle("");
}

static void TestLifecycleMoveCtor() {
  WebImage a("a.png", "a");
  assert(!a.IsCreated());
  a.EnsureCreated();
  assert(a.IsCreated());
  assert(a.GetHandle() != 0);

  WebImage b(std::move(a));
  assert(b.IsCreated());
  assert(b.GetHandle() != 0);
  assert(!a.IsCreated());
  assert(a.GetHandle() == 0);

  b.Destroy();
  assert(!b.IsCreated());
}

static void TestRemoveFromDomAndDestroyRecreate() {
  WebImage img("assets/test.png", "demo");
  img.SetParentElementId("root");
  img.SetElementId("demo-img");

  img.EnsureCreated();
  assert(img.IsCreated());

  const auto h1 = img.GetHandle();
  assert(h1 != 0);

  img.RemoveFromDom();
  assert(img.IsCreated());
  assert(img.GetHandle() == h1);

  img.Destroy();
  assert(!img.IsCreated());
  assert(img.GetHandle() == 0);

  img.EnsureCreated();
  assert(img.IsCreated());
  const auto h2 = img.GetHandle();
  assert(h2 != 0);
}

static void TestMoveAssign() {
  WebImage a("a.png", "a");
  a.EnsureCreated();
  assert(a.IsCreated());
  const auto h = a.GetHandle();
  assert(h != 0);

  WebImage b("b.png", "b");
  b.SetPositionPx(10, 20);
  b.SetOpacity(0.5);
  b.EnsureCreated();
  assert(b.IsCreated());

  b = std::move(a);

  assert(b.IsCreated());
  assert(b.GetHandle() == h);
  assert(!a.IsCreated());
  assert(a.GetHandle() == 0);

  b.Destroy();
  assert(!b.IsCreated());
}

static void TestSizeAutoEdgeCases() {
  WebImage img;

  img.SetSizePx(320.0, 240.0);
  assert(img.GetWidthPx() == 320.0);
  assert(img.GetHeightPx() == 240.0);

  // 0 means auto.
  img.SetSizePx(0.0, 0.0);
  assert(img.GetWidthPx() == 0.0);
  assert(img.GetHeightPx() == 0.0);

  img.SetSizePx(0.0, 123.0);
  assert(img.GetWidthPx() == 0.0);
  assert(img.GetHeightPx() == 123.0);

  img.SetSizePx(456.0, 0.0);
  assert(img.GetWidthPx() == 456.0);
  assert(img.GetHeightPx() == 0.0);
}

static void TestAssetHelper() {
  using namespace cse498::web_assets;
  assert(AssetPath(kPlayer) == "assets/player.png");
  assert(AssetPath(kTownHall) == "assets/townhall.png");
  assert(AssetPath(kFragileWall) == "assets/fragile_wall.png");
}

int main() {
  TestDefaults();
  TestSetters();
  TestSizeAutoEdgeCases();
  TestOpacityClamp();
  TestClassesAndStyles();
  TestLifecycleMoveCtor();
  TestRemoveFromDomAndDestroyRecreate();
  TestMoveAssign();
  TestAssetHelper();

  std::cout << "WebImage tests passed.\n";
  return 0;
}
