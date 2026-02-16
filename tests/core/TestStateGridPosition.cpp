#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/Worlds/StateGridPosition.hpp"


TEST_CASE("Test StateGridPosition Construction + getters", "[worlds]") {
  StateGridPosition p1;          // Default ctor
  StateGridPosition p2(3, 4);    // Direct ctor
  StateGridPosition p3(0, 0);    // Another direct
  StateGridPosition p4(p2);      // Copy ctor

  // Default should be (0,0)
  CHECK(p1.GetX() == 0);
  CHECK(p1.GetY() == 0);

  // Direct ctor
  CHECK(p2.GetX() == 3);
  CHECK(p2.GetY() == 4);

  // Sanity
  CHECK(p3.GetX() == 0);
  CHECK(p3.GetY() == 0);

  // Copy ctor copied values
  CHECK(p4.GetX() == 3);
  CHECK(p4.GetY() == 4);
}

TEST_CASE("Test StateGridPosition Comparisons", "[worlds]") {
  StateGridPosition a(1, 2);
  StateGridPosition b(1, 2);
  StateGridPosition c(2, 2);
  StateGridPosition d(1, 3);

  CHECK(a == b);
  CHECK_FALSE(a != b);

  CHECK(a < c);   // x differs
  CHECK(a < d);   // y differs
  CHECK(c > a);
}

TEST_CASE("Test StateGridPosition Move* returns new object (non-mutating)", "[worlds]") {
  StateGridPosition p(5, 5);

  auto up = p.MoveUp();
  CHECK(up.GetX() == 5);
  CHECK(up.GetY() == 6);

  auto down = p.MoveDown();
  CHECK(down.GetX() == 5);
  CHECK(down.GetY() == 4);

  auto left = p.MoveLeft();
  CHECK(left.GetX() == 4);
  CHECK(left.GetY() == 5);

  auto right = p.MoveRight();
  CHECK(right.GetX() == 6);
  CHECK(right.GetY() == 5);

  // Original unchanged
  CHECK(p.GetX() == 5);
  CHECK(p.GetY() == 5);
}

TEST_CASE("Test StateGridPosition Move* by assignment", "[worlds]") {
  StateGridPosition p(5, 5);

  p = p.MoveUp();
  CHECK(p.GetX() == 5);
  CHECK(p.GetY() == 6);

  p = p.MoveDown();
  CHECK(p.GetX() == 5);
  CHECK(p.GetY() == 5);

  p = p.MoveLeft();
  CHECK(p.GetX() == 4);
  CHECK(p.GetY() == 5);

  p = p.MoveRight();
  CHECK(p.GetX() == 5);
  CHECK(p.GetY() == 5);
}

TEST_CASE("Test StateGridPosition DistanceTo", "[worlds]") {
  StateGridPosition a(0, 0);
  StateGridPosition b(3, 4);

  CHECK(a.DistanceTo(b) == 7);
  CHECK(b.DistanceTo(a) == 7);

  StateGridPosition c(10, 10);
  StateGridPosition d(10, 10);
  CHECK(c.DistanceTo(d) == 0);
}

TEST_CASE("Test StateGridPosition ToString", "[worlds]") {
  StateGridPosition p(7, 8);
  std::string s = p.ToString();

  CHECK(s.find("7") != std::string::npos);
  CHECK(s.find("8") != std::string::npos);
  CHECK(s.find("x=") != std::string::npos);
  CHECK(s.find("y=") != std::string::npos);
}

TEST_CASE("Test StateGridPosition Neighbors", "[worlds]") {
  StateGridPosition p(5, 5);
  auto n = p.Neighbors();

  // Expected order: Up, Down, Left, Right
  CHECK(n[0] == StateGridPosition(5, 6));
  CHECK(n[1] == StateGridPosition(5, 4));
  CHECK(n[2] == StateGridPosition(4, 5));
  CHECK(n[3] == StateGridPosition(6, 5));

  // Original unchanged
  CHECK(p.GetX() == 5);
  CHECK(p.GetY() == 5);
}
