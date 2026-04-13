#include <cassert>
#include <stdexcept>
#include <utility>

#include "tools/WebCanvas.hpp"

using cse498::WebCanvas;

static void TestConstructorAndGetters() {
  WebCanvas canvas("demo_canvas", 640, 480);
  assert(canvas.GetElementID() == "demo_canvas");
  assert(canvas.GetWidth() == 640);
  assert(canvas.GetHeight() == 480);
  assert(canvas.IsReady());
}

static void TestResize() {
  WebCanvas canvas("resize_canvas", 100, 100);
  canvas.Resize(320, 240);
  assert(canvas.GetWidth() == 320);
  assert(canvas.GetHeight() == 240);

  bool threw = false;
  try {
    canvas.Resize(0, 5);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}

static void TestPixelToCell() {
  WebCanvas canvas("cell_canvas", 200, 200);
  std::pair<int, int> cell = canvas.PixelToCell(37, 58, 20, 20);
  assert(cell.first == 1);
  assert(cell.second == 2);

  cell = canvas.PixelToCell(-1, 12, 20, 20);
  assert(cell.first == -1);
  assert(cell.second == -1);
}

static void TestClickHandler() {
  WebCanvas canvas("click_canvas", 100, 100);
  int seen_x = -1;
  int seen_y = -1;
  canvas.SetClickHandler([&seen_x, &seen_y](int x, int y) {
    seen_x = x;
    seen_y = y;
  });
  canvas.HandleClick(42, 17);
  assert(seen_x == 42);
  assert(seen_y == 17);
}

static void TestDrawValidation() {
  WebCanvas canvas("draw_canvas", 200, 200);

  bool threw = false;
  try {
    canvas.DrawCell(0, 0, 0, 20, "#000000");
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    canvas.DrawEntity(50.0f, 50.0f, 0.0f, "#ff0000");
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}

static void TestDrawGridValidation() {
  WebCanvas canvas("grid_canvas", 200, 200);

  bool threw = false;
  try {
    canvas.DrawGrid(0, 10, 20, 20, [](int, int) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    canvas.DrawGrid(10, 10, 0, 20, [](int, int) {});
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    canvas.DrawGrid(10, 10, 20, 20, std::function<void(int, int)>());
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}

static void TestHighlightCellValidation() {
  WebCanvas canvas("highlight_canvas", 200, 200);

  bool threw = false;
  try {
    canvas.HighlightCell(-1, 0, 20, 20);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    canvas.HighlightCell(0, 0, 0, 20);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);

  threw = false;
  try {
    canvas.HighlightCell(0, 0, 20, 20, "#ffff00", 0.0f);
  } catch (const std::invalid_argument&) {
    threw = true;
  }
  assert(threw);
}


int main() {
  TestConstructorAndGetters();
  TestResize();
  TestPixelToCell();
  TestClickHandler();
  TestDrawValidation();
  TestDrawGridValidation();
  TestHighlightCellValidation();
  return 0;
}
