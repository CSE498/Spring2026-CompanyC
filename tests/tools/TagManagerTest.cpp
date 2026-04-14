/*
  Author: Shashank Papani

  File: TagManager.cpp

  Test file for the TagManager class.
*/

#include "catch2/catch.hpp"
#include "../../source/tools/TagManager.hpp"

#include <set>
#include <string>
#include <vector>

using cse498::TagManager;

namespace {

/*
  ToSet(v):
    Converts a vector of ids into a set so test results are easier to compare.
*/
std::set<TagManager::ObjectId> ToSet(const std::vector<TagManager::ObjectId>& v) {
  return std::set<TagManager::ObjectId>(v.begin(), v.end());
}

}  // namespace

/*
  Tests the default state and basic behavior of TagManager.
*/
TEST_CASE("DefaultConstruction-TagManager", "[TagManager][base]") {
  TagManager tm;

  CHECK(tm.ObjectCount() == 0);
  CHECK(tm.IsRegistered(1) == false);
  CHECK(tm.Count("missing") == 0);
  CHECK(tm.Count("   ") == 0);
  CHECK(tm.FindAny({}).empty());
  CHECK(tm.FindAll({}).empty());
  CHECK(tm.FindAllExcept({}, {}).empty());
}

/*
  Tests RegisterObject(id) and IsRegistered(id).
*/
TEST_CASE("RegisterObject-TagManager", "[TagManager][register]") {
  TagManager tm;

  tm.RegisterObject(1);
  tm.RegisterObject(2);

  CHECK(tm.IsRegistered(1) == true);
  CHECK(tm.IsRegistered(2) == true);
  CHECK(tm.IsRegistered(3) == false);
  CHECK(tm.ObjectCount() == 2);

  // Registering the same object again should be safe and not change the count.
  tm.RegisterObject(1);
  CHECK(tm.ObjectCount() == 2);
}

/*
  Tests UnregisterObject(id) with empty and missing objects.
*/
TEST_CASE("UnregisterObjectBasic-TagManager", "[TagManager][register][edge]") {
  TagManager tm;

  // A missing object should just be ignored safely.
  tm.UnregisterObject(99);
  CHECK(tm.ObjectCount() == 0);

  tm.RegisterObject(10);
  CHECK(tm.ObjectCount() == 1);

  // Removing an object with no tags should still work.
  tm.UnregisterObject(10);
  CHECK(tm.IsRegistered(10) == false);
  CHECK(tm.ObjectCount() == 0);
}

/*
  Tests AddTag(id, tag), HasTag(id, tag), RemoveTag(id, tag), and GetTags(id).
*/
TEST_CASE("AddRemoveHasGetTags-TagManager", "[TagManager][tags]") {
  TagManager tm;
  tm.RegisterObject(10);

  CHECK(tm.AddTag(10, "a") == true);
  CHECK(tm.AddTag(10, "a") == false);
  CHECK(tm.HasTag(10, "a") == true);
  CHECK(tm.HasTag(10, "b") == false);

  const auto& tags = tm.GetTags(10);
  CHECK(tags.size() == 1);
  CHECK(tags.count("a") == 1);

  CHECK(tm.RemoveTag(10, "a") == true);
  CHECK(tm.RemoveTag(10, "a") == false);
  CHECK(tm.HasTag(10, "a") == false);
  CHECK(tm.GetTags(10).empty());
}

/*
  Tests Count(tag) with multiple objects.
*/
TEST_CASE("Count-TagManager", "[TagManager][count]") {
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

/*
  Tests normalization behavior for AddTag, HasTag, Count, and queries.
  Right now NormalizeTag removes all whitespace characters.
*/
TEST_CASE("Normalization-TagManager", "[TagManager][normalize]") {
  TagManager tm;
  tm.RegisterObject(1);

  CHECK(tm.AddTag(1, "  dog  ") == true);

  CHECK(tm.HasTag(1, "dog") == true);
  CHECK(tm.HasTag(1, " dog") == true);
  CHECK(tm.HasTag(1, "dog ") == true);
  CHECK(tm.HasTag(1, " d o g ") == true);

  CHECK(tm.Count("dog") == 1);
  CHECK(tm.Count(" d o g ") == 1);

  CHECK(ToSet(tm.FindAny({" dog "})) == std::set<TagManager::ObjectId>{1});
  CHECK(ToSet(tm.FindAll({" d o g "})) == std::set<TagManager::ObjectId>{1});
}

/*
  Tests that tags with only whitespace are rejected.
*/
TEST_CASE("WhitespaceOnlyTagsRejected-TagManager", "[TagManager][edge][normalize]") {
  TagManager tm;
  tm.RegisterObject(1);

  CHECK(tm.AddTag(1, "   ") == false);
  CHECK(tm.AddTag(1, "\t\n") == false);
  CHECK(tm.AddTag(1, " \t \n ") == false);

  CHECK(tm.GetTags(1).empty());
  CHECK(tm.Count(" ") == 0);
  CHECK(tm.Count("\t\n") == 0);
}

/*
  Tests duplicate tags after normalization.
*/
TEST_CASE("DuplicateNormalizedTags-TagManager", "[TagManager][edge][normalize]") {
  TagManager tm;
  tm.RegisterObject(1);

  CHECK(tm.AddTag(1, " apple ") == true);
  CHECK(tm.AddTag(1, "apple") == false);
  CHECK(tm.AddTag(1, " a p p l e ") == false);

  CHECK(tm.GetTags(1).size() == 1);
  CHECK(tm.GetTags(1).count("apple") == 1);
  CHECK(tm.Count("apple") == 1);
}

/*
  Tests FindAny(tags) as an OR-style query.
*/
TEST_CASE("FindAny-TagManager", "[TagManager][query]") {
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
  tm.AddTag(1, "a");
  tm.AddTag(1, "b");
  tm.AddTag(2, "a");
  tm.AddTag(2, "c");
  tm.AddTag(3, "a");
  tm.AddTag(4, "b");

  CHECK(tm.FindAny({}).empty());
  CHECK(ToSet(tm.FindAny({"a"})) == std::set<TagManager::ObjectId>{1, 2, 3});
  CHECK(ToSet(tm.FindAny({"b"})) == std::set<TagManager::ObjectId>{1, 4});
  CHECK(ToSet(tm.FindAny({"a", "b"})) == std::set<TagManager::ObjectId>{1, 2, 3, 4});
  CHECK(tm.FindAny({"zzz"}).empty());
}

/*
  Tests FindAll(tags) as an AND-style query.
*/
TEST_CASE("FindAll-TagManager", "[TagManager][query]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.RegisterObject(2);
  tm.RegisterObject(3);
  tm.RegisterObject(4);

  tm.AddTag(1, "a");
  tm.AddTag(1, "b");
  tm.AddTag(2, "a");
  tm.AddTag(2, "c");
  tm.AddTag(3, "a");
  tm.AddTag(4, "b");

  CHECK(ToSet(tm.FindAll({"a"})) == std::set<TagManager::ObjectId>{1, 2, 3});
  CHECK(ToSet(tm.FindAll({"a", "b"})) == std::set<TagManager::ObjectId>{1});
  CHECK(tm.FindAll({"a", "zzz"}).empty());
}

/*
  Tests FindAllExcept(must_have, must_not_have) as an AND + NOT query.
*/
TEST_CASE("FindAllExcept-TagManager", "[TagManager][query]") {
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
  tm.AddTag(1, "a");
  tm.AddTag(1, "b");
  tm.AddTag(2, "a");
  tm.AddTag(2, "c");
  tm.AddTag(3, "a");
  tm.AddTag(4, "b");

  CHECK(ToSet(tm.FindAllExcept({"a"}, {"c"})) == std::set<TagManager::ObjectId>{1, 3});
  CHECK(ToSet(tm.FindAllExcept({}, {"a"})) == std::set<TagManager::ObjectId>{4, 5});
  CHECK(ToSet(tm.FindAllExcept({}, {"b"})) == std::set<TagManager::ObjectId>{2, 3, 5});
  CHECK(tm.FindAllExcept({"a"}, {"a"}).empty());
  CHECK(ToSet(tm.FindAllExcept({}, {})) == std::set<TagManager::ObjectId>{1, 2, 3, 4, 5});
}

/*
  Tests duplicate and normalized tags in the query functions.
*/
TEST_CASE("QueryNormalizationAndDedup-TagManager", "[TagManager][query][edge]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.RegisterObject(2);
  tm.RegisterObject(3);

  tm.AddTag(1, "apple");
  tm.AddTag(2, "banana");
  tm.AddTag(3, "apple");
  tm.AddTag(3, "banana");

  CHECK(ToSet(tm.FindAny({" apple ", "apple", " a p p l e "})) ==
        std::set<TagManager::ObjectId>{1, 3});

  CHECK(ToSet(tm.FindAll({" apple ", "apple"})) ==
        std::set<TagManager::ObjectId>{1, 3});

  CHECK(ToSet(tm.FindAllExcept({" banana ", "banana"}, {"  "})) ==
        std::set<TagManager::ObjectId>{2, 3});
}

/*
  Tests that query results come back sorted.
*/
TEST_CASE("SortedQueryResults-TagManager", "[TagManager][query][edge]") {
  TagManager tm;
  tm.RegisterObject(42);
  tm.RegisterObject(7);
  tm.RegisterObject(19);

  tm.AddTag(42, "x");
  tm.AddTag(7, "x");
  tm.AddTag(19, "x");

  CHECK(tm.FindAny({"x"}) == std::vector<TagManager::ObjectId>{7, 19, 42});
  CHECK(tm.FindAll({"x"}) == std::vector<TagManager::ObjectId>{7, 19, 42});
  CHECK(tm.FindAllExcept({"x"}, {}) == std::vector<TagManager::ObjectId>{7, 19, 42});
}

/*
  Tests cleanup after an object gets unregistered.
*/
TEST_CASE("UnregisterCleanup-TagManager", "[TagManager][cleanup]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.RegisterObject(2);

  tm.AddTag(1, "a");
  tm.AddTag(1, "b");
  tm.AddTag(2, "a");

  CHECK(tm.Count("a") == 2);
  CHECK(tm.Count("b") == 1);

  tm.UnregisterObject(1);
  
  const auto* removed_tags = tm.TryGetTags(1);
  CHECK(removed_tags == nullptr);

  CHECK(tm.IsRegistered(1) == false);
  CHECK(tm.IsRegistered(2) == true);
  CHECK(tm.Count("a") == 1);
  CHECK(tm.Count("b") == 0);
  CHECK(ToSet(tm.FindAll({"a"})) == std::set<TagManager::ObjectId>{2});
  CHECK(tm.FindAll({"b"}).empty());
}

/*
  Tests that removing the last tag bucket fully updates Count and queries.
*/
TEST_CASE("LastTagBucketCleanup-TagManager", "[TagManager][cleanup][edge]") {
  TagManager tm;
  tm.RegisterObject(1);

  tm.AddTag(1, "solo");
  CHECK(tm.Count("solo") == 1);

  CHECK(tm.RemoveTag(1, "solo") == true);
  CHECK(tm.Count("solo") == 0);
  CHECK(tm.FindAny({"solo"}).empty());
  CHECK(tm.FindAll({"solo"}).empty());
}

/*
  Tests TryGetTags(id) when the function is publicly available in the header.
*/
TEST_CASE("TryGetTags-TagManager", "[TagManager][api]") {
  TagManager tm;
  tm.RegisterObject(1);
  tm.AddTag(1, "blue");

  const auto* found = tm.TryGetTags(1);
  REQUIRE(found != nullptr);
  CHECK(found->size() == 1);
  CHECK(found->count("blue") == 1);

  const auto* missing = tm.TryGetTags(999);
  CHECK(missing == nullptr);
}
