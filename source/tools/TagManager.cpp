/*
  Author: Shashank Papani

  File: TagManager.cpp

  CLass TagManager: a double directional mapping between objects and string tags

  Design:
    id_to_tags_: ObjectId -> set of tags on that object
    tag_to_ids_: tag -> set of ObjectIds that have that tag
*/

#include "TagManager.hpp"

#include <limits>

/*
  Constructor: Uses default construction
  Both maps start empty
*/
TagManager::TagManager() = default;

/*
  NormalizeTag(tag):
    Trims leading and trailing whitespace

    Returns:
      "" if tag is empty or all whitespace
      otherwise, trimmed tag string
*/

std::string TagManager::NormalizeTag(std::string_view tag) {
  auto begin = tag.find_first_not_of(" \t\n\r\f\v");
  if (begin == std::string_view::npos) return ""; // all whitespace
  auto end = tag.find_last_not_of(" \t\n\r\f\v");
  tag = tag.substr(begin, end - begin + 1);

  return std::string(tag);
}

/*
  IsValidTag(tag): A tag is valid if, it is not empty after normalization.
*/
bool TagManager::IsValidTag(std::string_view tag) const {
  return !NormalizeTag(tag).empty();
}

/*
  RegisterObject(id) class:
    Ensures the object exists in id_to_tags_ with an empty tag set.
*/
void TagManager::RegisterObject(ObjectId id) {
  id_to_tags_.try_emplace(id, TagSet{});
}

/*
  UnregisterObject(id) Class:
    If a id is registered:
      removes id from every tag bucket it belongs to
      deletes any now-empty tag buckets from tag_to_ids_
      removes id from id_to_tags_
*/
void TagManager::UnregisterObject(ObjectId id) {
  auto it = id_to_tags_.find(id);
  if (it == id_to_tags_.end()) return;

  // Removing id from every tag bucket it belonged to
  for (const std::string& tag : it->second) {
    auto bucket_it = tag_to_ids_.find(tag);
    if (bucket_it != tag_to_ids_.end()) {
      bucket_it->second.erase(id);
      if (bucket_it->second.empty()) {
        tag_to_ids_.erase(bucket_it);
      }
    }
  }

  id_to_tags_.erase(it);
}

/*
  IsRegistered(id) class:
    Returns true iff id exists in id_to_tags_
*/
bool TagManager::IsRegistered(ObjectId id) const {
  return id_to_tags_.find(id) != id_to_tags_.end();
}

/*
  AddTag(id, tag) class:
    inserts normalized tag into id_to_tags_[id]
    if newly inserted, it also inserts the id into tag_to_ids_[tag]
    returns true if tag is newly added, false if it is already present

    Preconditions:
      id must be registered
      tag must be valid after normalization
      
*/
bool TagManager::AddTag(ObjectId id, std::string_view tag_view) {
  assert(IsRegistered(id) && "AddTag called for unregistered object");

  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return false;

  TagSet& tags = id_to_tags_.at(id);
  auto [it, inserted] = tags.insert(tag);
  if (!inserted) return false;

  tag_to_ids_[tag].insert(id);
  return true;
}

/*
  RemoveTag(id, tag) class:
    removes normalized tag from id_to_tags_[id]

    if removal happened:
      removes id from tag_to_ids_[tag]
      deletes the tag bucket if it becomes empty

    returns true if something was removed, false if tag was not present

    Preconditions:
      id must be registered
      tag must be valid after normalization  
*/
bool TagManager::RemoveTag(ObjectId id, std::string_view tag_view) {
  assert(IsRegistered(id) && "RemoveTag called for unregistered object");

  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return false;

  TagSet& tags = id_to_tags_.at(id);
  auto erased = tags.erase(tag);
  if (erased == 0) return false;

  auto bucket_it = tag_to_ids_.find(tag);
  if (bucket_it != tag_to_ids_.end()) {
    bucket_it->second.erase(id);
    if (bucket_it->second.empty()) tag_to_ids_.erase(bucket_it);
  }
  return true;
}

/*
  HasTag(id, tag) class:
    Returns true iff the normalized tag exists in the object tag set

    Preconditiond:
      id must be registered
      tag must be valid after normalization

*/
bool TagManager::HasTag(ObjectId id, std::string_view tag_view) const {
  assert(IsRegistered(id) && "HasTag called for unregistered object");

  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return false;

  const TagSet& tags = id_to_tags_.at(id);
  return tags.find(tag) != tags.end();
}

/*
  GetTags(id):
    Returns the set of tags on the object

    Preconditions:
      id must be registered
*/
const std::unordered_set<std::string>& TagManager::GetTags(ObjectId id) const {
  assert(IsRegistered(id) && "GetTags called for unregistered object");
  return id_to_tags_.at(id);
}

/*
  Count(tag):
    Returns the number of registered objects in the bucket for that tag
    If the tag bucket does not exist, it returns 0

    Preconditions :
      tag must be valid after normalization
*/
std::size_t TagManager::Count(std::string_view tag_view) const {
  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return 0;

  auto it = tag_to_ids_.find(tag);
  if (it == tag_to_ids_.end()) return 0;
  return it->second.size();
}

/*
  BucketContainsId(tag, id):
    Helper function used by query operations
    Returns true if tag_to_ids_ has an entry for 'tag' and it contains 'id'
*/
bool TagManager::BucketContainsId(const std::string& tag, ObjectId id) const {
  auto it = tag_to_ids_.find(tag);
  if (it == tag_to_ids_.end()) return false;
  return it->second.find(id) != it->second.end();
}

/*
  DedupStringsInPlace(v):
    Sorts and removes duplicates from a vector of strings
    Used to make query behavior stable when duplicate tags are passed in
*/
void TagManager::DedupStringsInPlace(std::vector<std::string>& v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

/*
  FindAny(tags):
    OR query:
      Returns all object ids that contain at least one of the provided tags

    Returns:
      empty vector if input list is empty or no tags match
      otherwise, ids
*/
std::vector<TagManager::ObjectId> TagManager::FindAny(const std::vector<std::string>& tags) const {
  if (tags.empty()) return {};

  std::unordered_set<ObjectId> result_set;
  for (const std::string& raw_tag : tags) {

    const std::string tag = NormalizeTag(raw_tag);
    if (tag.empty()) continue;

    auto it = tag_to_ids_.find(tag);
    if (it == tag_to_ids_.end()) continue;

    for (ObjectId id : it->second) {
      result_set.insert(id);
    }

  }

  return std::vector<ObjectId>(result_set.begin(), result_set.end());
}

/*
  FindAll(must_have):
    Returns ids that contain all required tags.
*/
std::vector<TagManager::ObjectId> TagManager::FindAll(const std::vector<std::string>& must_have) const {
  return FindAllExcept(must_have, {});
}

/*
  FindAllExcept(must_have, must_not_have):
    Returns ids that contain all required tags and none of the forbidden tags.

    Cases:
      If must_have is empty:
        it returns all registered object ids
    Otherwise:
      chooses the smallest required tag bucket as the candidate set,
      then checks remaining required tags and forbidden tags per candidate
  (Used LLM to write this function)
*/
std::vector<TagManager::ObjectId> TagManager::FindAllExcept(
    const std::vector<std::string>& must_have,
    const std::vector<std::string>& must_not_have) const {
  // If must_have is empty, return all registered objects (common + useful).
  if (must_have.empty()) {
    std::vector<ObjectId> all_ids;
    all_ids.reserve(id_to_tags_.size());
    for (const auto& [id, _] : id_to_tags_) all_ids.push_back(id);

    // Filter out forbidden tags
    if (!must_not_have.empty()) {
      std::vector<std::string> forbid = must_not_have;
      DedupStringsInPlace(forbid);

      std::vector<ObjectId> filtered;
      filtered.reserve(all_ids.size());
      for (ObjectId id : all_ids) {
        bool bad = false;
        for (const std::string& t : forbid) {
          if (!t.empty() && BucketContainsId(NormalizeTag(t), id)) {
            bad = true;
            break;
          }
        }
        if (!bad) filtered.push_back(id);
      }
      return filtered;
    }

    return all_ids;
  }

  // Copy + dedup must_have for stable behavior
  std::vector<std::string> required = must_have;
  DedupStringsInPlace(required);

  // Pick the smallest bucket as starting point (speed)
  std::size_t best_size = std::numeric_limits<std::size_t>::max();
  const IdSet* start_bucket = nullptr;
  std::string start_tag;

  for (const std::string& t : required) {
    const std::string tag = NormalizeTag(t);
    if (tag.empty()) return {};
    

    auto it = tag_to_ids_.find(tag);
    if (it == tag_to_ids_.end()) return {};  // required tag not present => no results

    if (it->second.size() < best_size) {
      best_size = it->second.size();
      start_bucket = &it->second;
      start_tag = tag;
    }
  }

  assert(start_bucket != nullptr);

  // Dedup forbidden list too
  std::vector<std::string> forbid = must_not_have;
  DedupStringsInPlace(forbid);

  std::vector<ObjectId> result;
  result.reserve(start_bucket->size());

  // Filter candidates
  for (ObjectId id : *start_bucket) {
    // must have all required
    bool ok = true;
    for (const std::string& t : required) {
      const std::string tag = NormalizeTag(t);
      if (tag == start_tag) continue;
      if (!BucketContainsId(tag, id)) {
        ok = false;
        break;
      }
    }
    if (!ok) continue;

    // must not have any forbidden
    for (const std::string& t : forbid) {
      const std::string tag = NormalizeTag(t);
      if (t.empty()) continue;
      if (BucketContainsId(tag, id)) {
        ok = false;
        break;
      }
    }
    if (!ok) continue;

    result.push_back(id);
  }

  return result;
}
