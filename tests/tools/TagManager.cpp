/*
  Author: Shashank Papani
  File: test_tagmanager.cpp
  Tests for TagManager class
*/

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/TagManager.hpp"

#include <set>
#include <vector>
#include <string>

using cse498::TagManager;

static std::set<TagManager::ObjectId> ToSet(const std::vector<TagManager::ObjectId>& v) {
  return std::set<TagManager::ObjectId>(v.begin(), v.end());
}

/*
  Testing basic functionality of TagManager class
*/
TEST_CASE("BaseFunctionality-TagManager", "[tools]") {
  TagManager tm;

  CHECK(tm.IsRegistered(1) == false);
  CHECK(tm.Count("missing") == 0);
  CHECK(tm.FindAny({}).empty());
  CHECK(ToSet(tm.FindAll({})) == std::set<TagManager::ObjectId>{}); // no objects registered
}

/*
  Test case for Registering and unregistering objects in TagManager class
*/
TEST_CASE("RegisterUnregister-TagManager", "[tools]") {
  TagManager tm;

  tm.RegisterObject(1);
  CHECK(tm.IsRegistered(1) == true);

  tm.RegisterObject(1); // double-register should be safe
  CHECK(tm.IsRegistered(1) == true);

  tm.UnregisterObject(1);
  CHECK(tm.IsRegistered(1) == false);
}

TEST_CASE("AddRemoveHasGetTags-TagManager", "[tools]") {
  TagManager tm;
  tm.RegisterObject(10);

  CHECK(tm.AddTag(10, "a") == true);
  CHECK(tm.AddTag(10, "a") == false);
  CHECK(tm.HasTag(10, "a") == true);
  CHECK(tm.HasTag(10, "b") == false);

  const auto& tags = tm.GetTags(10);
  CHECK(tags.find("a") != tags.end());

  CHECK(tm.RemoveTag(10, "a") == true);
  CHECK(tm.RemoveTag(10, "a") == false);
  CHECK(tm.HasTag(10, "a") == false);
}

TEST_CASE("Count-TagManager", "[tools]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.RegisterObject(2);
  tm.RegisterObject(3);

  tm.AddTag(1, "x");
  tm.AddTag(2, "x");
  tm.AddTag(3, "y");

  CHECK(tm.Count("x") == 2);
  CHECK(tm.Count("y") == 1);
  CHECK(tm.Count("z") == 0);

  tm.RemoveTag(2, "x");
  CHECK(tm.Count("x") == 1);
  tm.RemoveTag(1, "x");
  CHECK(tm.Count("x") == 0);
}

TEST_CASE("Queries-TagManager", "[tools]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.RegisterObject(2);
  tm.RegisterObject(3);
  tm.RegisterObject(4);
  tm.RegisterObject(5);

  // 1: a, b
  // 2: a, c
  // 3: a
  // 4: b
  // 5: none
  tm.AddTag(1, "a"); tm.AddTag(1, "b");
  tm.AddTag(2, "a"); tm.AddTag(2, "c");
  tm.AddTag(3, "a");
  tm.AddTag(4, "b");

  // FindAny (OR)
  CHECK(ToSet(tm.FindAny({"a"})) == std::set<TagManager::ObjectId>{1,2,3});
  CHECK(ToSet(tm.FindAny({"b"})) == std::set<TagManager::ObjectId>{1,4});
  CHECK(ToSet(tm.FindAny({"a","b"})) == std::set<TagManager::ObjectId>{1,2,3,4});
  CHECK(tm.FindAny({"zzz"}).empty());

  // FindAll (AND)
  CHECK(ToSet(tm.FindAll({"a"})) == std::set<TagManager::ObjectId>{1,2,3});
  CHECK(ToSet(tm.FindAll({"a","b"})) == std::set<TagManager::ObjectId>{1});
  CHECK(tm.FindAll({"a","zzz"}).empty());

  // FindAllExcept (AND + NOT)
  CHECK(ToSet(tm.FindAllExcept({"a"}, {"c"})) == std::set<TagManager::ObjectId>{1,3});
  CHECK(ToSet(tm.FindAllExcept({}, {"a"})) == std::set<TagManager::ObjectId>{4,5});
  CHECK(tm.FindAllExcept({"a"}, {"a"}).empty());
}

TEST_CASE("UnregisterCleanup-TagManager", "[tools]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.RegisterObject(2);

  tm.AddTag(1, "a");
  tm.AddTag(1, "b");
  tm.AddTag(2, "a");

  CHECK(tm.Count("a") == 2);
  CHECK(tm.Count("b") == 1);

  tm.UnregisterObject(1);

  CHECK(tm.IsRegistered(1) == false);
  CHECK(tm.Count("a") == 1);
  CHECK(tm.Count("b") == 0);
  CHECK(ToSet(tm.FindAll({"a"})) == std::set<TagManager::ObjectId>{2});
  CHECK(tm.FindAll({"b"}).empty());
}

TEST_CASE("Normalization-TagManager", "[tools]") {
  TagManager tm;
  tm.RegisterObject(1);

  // AddTag should treat whitespace around tags as the same tag
  tm.AddTag(1, "  dog  ");

  CHECK(tm.HasTag(1, "dog") == true);
  CHECK(tm.HasTag(1, "  dog") == true);
  CHECK(tm.HasTag(1, "dog  ") == true);

  CHECK(tm.Count("dog") == 1);
  CHECK(tm.Count("  dog  ") == 1);

  // Queries should also normalize inputs.
  CHECK(ToSet(tm.FindAny({" dog "})) == std::set<TagManager::ObjectId>{1});
  CHECK(ToSet(tm.FindAll({" dog "})) == std::set<TagManager::ObjectId>{1});

  // Whitespace-only query tags should behave like no tag
  CHECK(tm.FindAny({"   "}).empty());
}

TEST_CASE("WhitespaceTagsRejected-TagManager", "[tools][edge]") {
  TagManager tm;
  tm.RegisterObject(1);

  // Whitespace only tags normalize to empty is invalid
  CHECK(tm.AddTag(1, "   ") == false);
  CHECK(tm.AddTag(1, "\t\n") == false);

  // Nothing should have been added
  CHECK(tm.GetTags(1).empty());
  CHECK(tm.Count(" ") == 0);
}

TEST_CASE("TagNormalization-TagManager", "[tools][edge]") {
  TagManager tm;
  tm.RegisterObject(1);

  tm.AddTag(1, "  apple  ");

  // Normalized lookup should work everywhere
  CHECK(tm.HasTag(1, "apple") == true);
  CHECK(tm.HasTag(1, "  apple ") == true);

  CHECK(tm.Count("apple") == 1);
  CHECK(tm.Count("  apple ") == 1);

  auto tags = tm.GetTags(1);
  CHECK(tags.size() == 1);
  CHECK(tags.count("apple") == 1);
}
