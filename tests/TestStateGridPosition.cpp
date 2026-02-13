#include <cassert>
#include <iostream>
#include <string>

#include "Worlds/StateGridPosition.hpp"   
static void Header(const char* name) {
  std::cout << "[TEST] " << name << "\n";
}

int main() {
  // 1) Construction + getters
  Header("Construction + getters");
  {
    StateGridPosition p(3, 4);
    assert(p.GetX() == 3);
    assert(p.GetY() == 4);

    StateGridPosition d;
    assert(d.GetX() == 0);
    assert(d.GetY() == 0);
  }

  // 2) Comparisons (via <=> default)
  Header("Comparisons");
  {
    StateGridPosition a(1, 2);
    StateGridPosition b(1, 2);
    StateGridPosition c(2, 2);
    StateGridPosition d(1, 3);

    assert(a == b);
    assert(!(a != b));

    // Using <, > comes from <=> in C++20
    assert(a < c);   // x differs
    assert(a < d);   // y differs
    assert(c > a);
  }

  // 3) MoveUp/Down/Left/Right returns NEW object (does not mutate original)
  Header("Moves return new object");
  {
    StateGridPosition p(5, 5);

    StateGridPosition up = p.MoveUp();
    assert(up.GetX() == 5);
    assert(up.GetY() == 6);

    StateGridPosition down = p.MoveDown();
    assert(down.GetX() == 5);
    assert(down.GetY() == 4);

    StateGridPosition left = p.MoveLeft();
    assert(left.GetX() == 4);
    assert(left.GetY() == 5);

    StateGridPosition right = p.MoveRight();
    assert(right.GetX() == 6);
    assert(right.GetY() == 5);

    // Original unchanged
    assert(p.GetX() == 5);
    assert(p.GetY() == 5);
  }

  // 4) MoveUp/Down/Left/Right
  Header("Moves position");
  {
    StateGridPosition p(5, 5);

    p = p.MoveUp();
    assert(p.GetX() == 5);
    assert(p.GetY() == 6);

    p = p.MoveDown();
    assert(p.GetX() == 5);
    assert(p.GetY() == 5);

    p = p.MoveLeft();
    assert(p.GetX() == 4);
    assert(p.GetY() == 5);

    p = p.MoveRight();
    assert(p.GetX() == 5);
    assert(p.GetY() == 5);

    // Original unchanged
    assert(p.GetX() == 5);
    assert(p.GetY() == 5);
  }

  // 5) DistanceTo 
  Header("DistanceTo");
  {
    StateGridPosition a(0, 0);
    StateGridPosition b(3, 4);
    assert(a.DistanceTo(b) == 7);
    assert(b.DistanceTo(a) == 7);

    StateGridPosition c(10, 10);
    StateGridPosition d(10, 10);
    assert(c.DistanceTo(d) == 0);
  }

  // 6) ToString sanity 
  Header("ToString");
  {
    StateGridPosition p(7, 8);
    std::string s = p.ToString();

    // Check it contains the numbers 
    assert(s.find("7") != std::string::npos);
    assert(s.find("8") != std::string::npos);
    assert(s.find("x=") != std::string::npos);
    assert(s.find("y=") != std::string::npos);
  }

  Header("Neighbors()");
{
  StateGridPosition p(5, 5);
  auto n = p.Neighbors();

  // Expected order: Up, Down, Left, Right 
  assert(n[0] == StateGridPosition(5, 6));
  assert(n[1] == StateGridPosition(5, 4));
  assert(n[2] == StateGridPosition(4, 5));
  assert(n[3] == StateGridPosition(6, 5));

  // original unchanged
  assert(p.GetX() == 5 && p.GetY() == 5);
}

  std::cout << "\nAll StateGridPosition tests passed ✅\n";
  return 0;
}