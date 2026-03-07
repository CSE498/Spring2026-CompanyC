#include "../../source/tools/RobinHoodMap.hpp"
#include <string>
#include <stdexcept>

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
using cse498::RobinHoodMap;

TEST_CASE("RobinHoodMap: insert/find/contains basics") {
  RobinHoodMap<std::string, int> m;

  REQUIRE(m.Empty());
  REQUIRE(m.Size() == 0);

  auto [p1, ins1] = m.Insert("banana", 7);
  REQUIRE(ins1);
  REQUIRE(p1 != nullptr);
  REQUIRE(*p1 == 7);

  REQUIRE(!m.Empty());
  REQUIRE(m.Size() == 1);
  REQUIRE(m.Contains("banana"));

  const int* fp = m.Find("banana");
  REQUIRE(fp != nullptr);
  REQUIRE(*fp == 7);

  REQUIRE(m.Find("kiwi") == nullptr);
  REQUIRE(!m.Contains("kiwi"));

  // Duplicate insert should not overwrite in your current semantics
  auto [p2, ins2] = m.Insert("banana", 99);
  REQUIRE(!ins2);
  REQUIRE(p2 != nullptr);
  REQUIRE(*p2 == 7);

  REQUIRE(m.Size() == 1);
}

TEST_CASE("RobinHoodMap: At returns value and throws on missing") {
  RobinHoodMap<int, int> m;
  m.Insert(1, 10);

  REQUIRE(m.At(1) == 10);
  REQUIRE_THROWS_AS(m.At(999), std::out_of_range);
}

TEST_CASE("RobinHoodMap: rehash/reserve keeps all items") {
  RobinHoodMap<int, int> m;
  m.MaxLoadFactor(0.5f);

  // Force growth/rehash by inserting many items.
  for (int i = 0; i < 5000; ++i) {
    auto [p, inserted] = m.Insert(i, i * 3);
    REQUIRE(inserted);
    REQUIRE(p != nullptr);
    REQUIRE(*p == i * 3);
  }
  REQUIRE(m.Size() == 5000);

  for (int i = 0; i < 5000; i += 17) {
    const int* p = m.Find(i);
    REQUIRE(p != nullptr);
    REQUIRE(*p == i * 3);
  }

  // Reserve larger and verify still valid
  m.Reserve(20000);
  REQUIRE(m.Size() == 5000);
  for (int i = 0; i < 5000; i += 23) {
    const int* p = m.Find(i);
    REQUIRE(p != nullptr);
    REQUIRE(*p == i * 3);
  }
}

TEST_CASE("RobinHoodMap: Clear empties the table") {
  RobinHoodMap<int, int> m;
  m.Insert(1, 2);
  m.Insert(3, 4);
  REQUIRE(m.Size() == 2);

  m.Clear();
  REQUIRE(m.Empty());
  REQUIRE(m.Size() == 0);
  REQUIRE(m.Find(1) == nullptr);
  REQUIRE(!m.Contains(3));
}
