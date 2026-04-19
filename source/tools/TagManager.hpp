/*
  Author: Shashank Papani
  File: TagManager.hpp
  This is the header file for the TagManager class.
*/
#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cse498 {

class TagManager {
 public:
  // Use a stable integer ID so references do not break if objects move or get removed.
  using ObjectId = std::uint64_t;
  using TagSet = std::unordered_set<std::string>;
  using IdSet = std::unordered_set<ObjectId>;
  using IdList = std::vector<ObjectId>;

  // Minimum length a normalized tag needs to be valid.
  static constexpr std::size_t kMinValidTagLength = 1;

  /*
    Default constructor:
    Starts with no registered objects and no tags stored.
  */
  TagManager();

  /*
    Default destructor.
  */
  ~TagManager() = default;

  // Functions for registering objects
  void RegisterObject(ObjectId id);
  void UnregisterObject(ObjectId id);
  bool IsRegistered(ObjectId id) const noexcept;

  // Functions for adding, removing, and checking tags
  bool AddTag(ObjectId id, std::string_view tag);  // true if the tag was added
  bool RemoveTag(ObjectId id, std::string_view tag);  // true if the tag was removed
  bool HasTag(ObjectId id, std::string_view tag) const;

  // Read the tags on an object
  const TagSet& GetTags(ObjectId id) const;

  /*
    TryGetTags(id):
      Returns a pointer to the object's tag set if the id is registered.
      Returns nullptr if the object is not registered.

    Purpose:
      Gives other code a safe read-only way to check tags without assuming
      the object is already there.
  */
  const TagSet* TryGetTags(ObjectId id) const noexcept;

  // Query functions for tags
  IdList FindAny(const std::vector<std::string>& tags) const;
  IdList FindAll(const std::vector<std::string>& must_have) const;
  IdList FindAllExcept(const std::vector<std::string>& must_have,
                       const std::vector<std::string>& must_not_have) const;

  // Count how many objects have a tag
  std::size_t Count(std::string_view tag) const;

  /*
    ObjectCount():
      Returns the current number of registered objects.
  */
  std::size_t ObjectCount() const noexcept;

  // Check if a tag is valid
  static bool IsValidTag(std::string_view tag);

 private:
  // Main storage
  std::unordered_map<ObjectId, TagSet> id_to_tags_;
  std::unordered_map<std::string, IdSet> tag_to_ids_;

  // Helper functions
  static std::string NormalizeTag(std::string_view tag);
  static void DedupStringsInPlace(std::vector<std::string>& v);
  static void SortIdsInPlace(IdList& ids);

  /*
    NormalizeTags(tags):
      Template helper used by the query functions.
      It normalizes each input string, removes empty results, and removes duplicates.

    Purpose:
      This keeps the query code shorter and avoids repeating the same
      normalization steps in different places.
  */
  template <typename Range>
  static std::vector<std::string> NormalizeTags(const Range& tags) {
    std::vector<std::string> normalized_tags;
    for (const auto& tag : tags) {
      std::string clean_tag = NormalizeTag(tag);
      if (!clean_tag.empty()) {
        normalized_tags.push_back(std::move(clean_tag));
      }
    }
    DedupStringsInPlace(normalized_tags);
    return normalized_tags;
  }

  // Used to check tag buckets quickly during queries
  bool BucketContainsId(const std::string& tag, ObjectId id) const;

  /*
    CheckInvariant():
      Debug helper that checks if both internal maps match each other.

    It checks:
      if id_to_tags_[id] contains tag, then tag_to_ids_[tag] contains id
      if tag_to_ids_[tag] contains id, then id_to_tags_[id] contains tag

    Purpose:
      Used with assert after changes to catch internal bugs early.
  */
  bool CheckInvariant() const;
};

}  // namespace cse498
