#include <catch2/catch_test_macros.hpp>
#include <unordered_set>
#include <vector>

#include "MemoryFactory.hpp"

namespace {

struct Foo {
  int x;
  int y;
  Foo(int a, int b) : x(a), y(b) {}
};

struct Tiny {
  char c;
  explicit Tiny(char ch) : c(ch) {}
};

} // namespace

TEST_CASE("MemoryFactory constructs objects and returns valid pointers",
          "[MemoryFactory]") {
  MemoryFactory<Foo, 8> pool(1);

  Foo *f = pool.Make(1, 2);
  REQUIRE(f != nullptr);
  REQUIRE(f->x == 1);
  REQUIRE(f->y == 2);

  pool.Delete(f);
}

TEST_CASE("MemoryFactory reuses freed slots", "[MemoryFactory]") {
  MemoryFactory<Foo, 8> pool(1);

  Foo *a = pool.Make(1, 10);
  Foo *b = pool.Make(2, 20);

  pool.Delete(a);

  Foo *c = pool.Make(3, 30);
  REQUIRE(c == a);

  pool.Delete(b);
  pool.Delete(c);
}

TEST_CASE("MemoryFactory grows when it runs out of slots", "[MemoryFactory]") {
  MemoryFactory<Foo, 4> pool(1);

  std::vector<Foo *> ptrs;
  ptrs.reserve(9);

  for (int i = 0; i < 9; ++i) {
    Foo *p = pool.Make(i, i * 10);
    REQUIRE(p != nullptr);
    ptrs.push_back(p);
  }

  std::unordered_set<Foo *> unique(ptrs.begin(), ptrs.end());
  REQUIRE(unique.size() == ptrs.size());

  for (Foo *p : ptrs) {
    pool.Delete(p);
  }
}

TEST_CASE("MemoryFactory works for types smaller than a pointer",
          "[MemoryFactory]") {
  MemoryFactory<Tiny, 8> pool(1);

  std::vector<Tiny *> ptrs;
  ptrs.reserve(16);

  for (int i = 0; i < 16; ++i) {
    char ch = static_cast<char>('A' + i);
    Tiny *t = pool.Make(ch);
    REQUIRE(t != nullptr);
    REQUIRE(t->c == ch);
    ptrs.push_back(t);
  }

  for (Tiny *t : ptrs) {
    pool.Delete(t);
  }
}
