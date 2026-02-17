/*
  Author: Shashank Papani
  File: TagManager.hpp
  Header file for TagManager class
*/
#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstddef>
#include <cctype>

class TagManager {
public:

  /*
    Constructor class:
    Initializes an empty TagManager with no registered objects and no tag buckets
  */
  TagManager();

  /*
    Default destructor:
  */
  ~TagManager() = default;

  // Using stable integer ID to avoid dangling references when objects are moved or removed.
  using ObjectId = std::uint64_t;
  
  // Registration operations
  void RegisterObject(ObjectId id);
  void UnregisterObject(ObjectId id);
  bool IsRegistered(ObjectId id) const;

  // Tag operations (require object is registered)
  bool AddTag(ObjectId id, std::string_view tag); //true if it is newly added
  bool RemoveTag(ObjectId id, std::string_view tag); //true if it is removed
  bool HasTag(ObjectId id, std::string_view tag) const;

  // Read tags
  const std::unordered_set<std::string>& GetTags(ObjectId id) const;

  // Tag queries
  std::vector<ObjectId> FindAny(const std::vector<std::string>& tags) const;
  std::vector<ObjectId> FindAll(const std::vector<std::string>& must_have) const;
  std::vector<ObjectId> FindAllExcept(const std::vector<std::string>& must_have,
                                      const std::vector<std::string>& must_not_have) const;
                                      
  // Count tags
  std::size_t Count(std::string_view tag) const;

  // Checks if tag is valid
  bool IsValidTag(std::string_view tag) const;

private:
  using TagSet = std::unordered_set<std::string>;
  using IdSet = std::unordered_set<ObjectId>;

  // Core Storage (using maps)
  std::unordered_map<ObjectId, TagSet> id_to_tags_;
  std::unordered_map<std::string, IdSet> tag_to_ids_;

  // Helpers
  static std::string NormalizeTag(std::string_view tag);
  static void DedupStringsInPlace(std::vector<std::string>& v);

  // For fast filtering in queries
  bool BucketContainsId(const std::string& tag, ObjectId id) const;
};