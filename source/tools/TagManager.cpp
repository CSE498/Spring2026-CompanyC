/*
  Author: Shashank Papani

  File: TagManager.cpp

  CLass TagManager: a two way mapping between objects and string tags

  Design:
    id_to_tags_: ObjectId -> set of tags for that object
    tag_to_ids_: tag -> set of ObjectIds that have that tag
*/

#include "TagManager.hpp"

#include <algorithm>
#include <cassert>

namespace cse498 {

namespace {

// Shared empty tag set that only gets returned in the safe fallback path of GetTags.
const TagManager::TagSet kEmptyTagSet{};

}  // namespace

/*
  Constructor: just uses default construction
  Both maps begin empty
*/
TagManager::TagManager() = default;

/*
  NormalizeTag(tag):
    Takes out all whitespace

    Returns:
      "" if tag is empty or only whitespace
      otherwise, cleaned tag string
*/
std::string TagManager::NormalizeTag(std::string_view tag) {
  std::string out{tag};
  out.erase(std::remove_if(out.begin(), out.end(),
                           [](unsigned char ch) {
                             return std::isspace(ch) != 0;
                           }),
            out.end());
  return out;
}

/*
  IsValidTag(tag): A tag is valid if it is not empty after normalization.
*/
bool TagManager::IsValidTag(std::string_view tag) {
  const std::string normalized = NormalizeTag(tag);
  return normalized.size() >= kMinValidTagLength;
}

/*
  RegisterObject(id) class:
    Makes sure the object exists in id_to_tags_ with an empty tag set.

    Also checks the internal invariant after inserting.
*/
void TagManager::RegisterObject(ObjectId id) {
  id_to_tags_.try_emplace(id, TagSet{});
  assert(CheckInvariant());
}

/*
  UnregisterObject(id) Class:
    If an id is registered:
      removes id from every tag bucket it is in
      deletes any tag buckets from tag_to_ids_ that become empty
      removes id from id_to_tags_

    Also checks the internal invariant after removing.
*/
void TagManager::UnregisterObject(ObjectId id) {
  auto it = id_to_tags_.find(id);
  if (it == id_to_tags_.end()) return;

  // Remove id from every tag bucket it was in
  for (const std::string& tag : it->second) {
    auto bucket_it = tag_to_ids_.find(tag);
    assert(bucket_it != tag_to_ids_.end());
    bucket_it->second.erase(id);
    if (bucket_it->second.empty()) {
      tag_to_ids_.erase(bucket_it);
    }
  }

  id_to_tags_.erase(it);
  assert(CheckInvariant());
}

/*
  IsRegistered(id) class:
    Returns true if id is in id_to_tags_
*/
bool TagManager::IsRegistered(ObjectId id) const noexcept {
  return id_to_tags_.find(id) != id_to_tags_.end();
}

/*
  AddTag(id, tag) class:
    inserts normalized tag into id_to_tags_[id]
    if it gets inserted, it also inserts the id into tag_to_ids_[tag]
    returns true if tag was added, false if it was already there

    Preconditions:
      id must be registered
      tag must be valid after normalization

    Debug:
      checks the class invariant before and after changing stuff
*/
bool TagManager::AddTag(ObjectId id, std::string_view tag_view) {
  assert(CheckInvariant());
  assert(IsRegistered(id) && "AddTag requires a registered object id.");

  if (!IsRegistered(id)) return false;

  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return false;

  TagSet& tags = id_to_tags_.at(id);
  auto [it, inserted] = tags.insert(tag);
  if (!inserted) return false;

  tag_to_ids_[tag].insert(id);
  assert(CheckInvariant());
  return true;
}

/*
  RemoveTag(id, tag) class:
    removes normalized tag from id_to_tags_[id]

    if removal happened:
      removes id from tag_to_ids_[tag]
      deletes the tag bucket if it becomes empty

    returns true if something got removed, false if tag was not there

    Preconditions:
      id must be registered
      tag must be valid after normalization

    Debug:
      checks the class invariant before and after changing stuff
*/
bool TagManager::RemoveTag(ObjectId id, std::string_view tag_view) {
  assert(CheckInvariant());
  assert(IsRegistered(id) && "RemoveTag requires a registered object id.");

  if (!IsRegistered(id)) return false;

  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return false;

  TagSet& tags = id_to_tags_.at(id);
  auto erased = tags.erase(tag);
  if (erased == 0) return false;

  auto bucket_it = tag_to_ids_.find(tag);
  assert(bucket_it != tag_to_ids_.end());
  bucket_it->second.erase(id);
  if (bucket_it->second.empty()) tag_to_ids_.erase(bucket_it);

  assert(CheckInvariant());
  return true;
}

/*
  HasTag(id, tag) class:
    Returns true iff the normalized tag is in the object's tag set

    Preconditions:
      id must be registered
      tag must be valid after normalization
*/
bool TagManager::HasTag(ObjectId id, std::string_view tag_view) const {
  assert(IsRegistered(id) && "HasTag requires a registered object id.");

  if (!IsRegistered(id)) return false;

  const std::string tag = NormalizeTag(tag_view);
  if (tag.empty()) return false;

  const TagSet& tags = id_to_tags_.at(id);
  return tags.find(tag) != tags.end();
}

/*
  GetTags(id):
    Returns the set of tags for the object

    Preconditions:
      id must be registered

    Notes:
      a debug assert is used for programmer mistakes.
      a shared empty set is only returned as a safe fallback.
*/
const TagManager::TagSet& TagManager::GetTags(ObjectId id) const {
  assert(IsRegistered(id) && "GetTags requires a registered object id.");

  const TagSet* tags = TryGetTags(id);
  if (tags == nullptr) return kEmptyTagSet;
  return *tags;
}

/*
  TryGetTags(id):
    Returns a pointer to the tag set for an object.

    Returns:
      nullptr if the object is not registered
      otherwise a pointer to the stored tag set
*/
const TagManager::TagSet* TagManager::TryGetTags(ObjectId id) const noexcept {
  auto it = id_to_tags_.find(id);
  if (it == id_to_tags_.end()) return nullptr;
  return &it->second;
}

/*
  Count(tag):
    Returns how many registered objects are in the bucket for that tag
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
  ObjectCount():
    Returns the number of objects currently registered.
*/
std::size_t TagManager::ObjectCount() const noexcept {
  return id_to_tags_.size();
}

/*
  BucketContainsId(tag, id):
    Helper function used by query operations
    Returns true if tag_to_ids_ has an entry for 'tag' and that entry contains 'id'
*/
bool TagManager::BucketContainsId(const std::string& tag, ObjectId id) const {
  auto it = tag_to_ids_.find(tag);
  if (it == tag_to_ids_.end()) return false;
  return it->second.find(id) != it->second.end();
}

/*
  DedupStringsInPlace(v):
    Sorts and removes duplicates from a vector of strings
    Used to keep query behavior stable when duplicate tags get passed in
*/
void TagManager::DedupStringsInPlace(std::vector<std::string>& v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

/*
  SortIdsInPlace(ids):
    Sorts query results so the output order stays stable.

    Purpose:
      This makes tests easier to write and keeps API behavior more predictable
      across different runs.
*/
void TagManager::SortIdsInPlace(IdList& ids) {
  std::sort(ids.begin(), ids.end());
}

/*
  CheckInvariant():
    Checks that both internal maps match each other.

    It checks:
      every tag stored for an id points back to that same id
      every id stored in a tag bucket points back to that same tag

    Returns:
      true if the structure is internally consistent
      false otherwise
*/
bool TagManager::CheckInvariant() const {
  for (const auto& [id, tags] : id_to_tags_) {
    for (const auto& tag : tags) {
      auto bucket_it = tag_to_ids_.find(tag);
      if (bucket_it == tag_to_ids_.end()) return false;
      if (bucket_it->second.find(id) == bucket_it->second.end()) return false;
    }
  }

  for (const auto& [tag, ids] : tag_to_ids_) {
    if (tag.empty() || ids.empty()) return false;
    for (ObjectId id : ids) {
      auto object_it = id_to_tags_.find(id);
      if (object_it == id_to_tags_.end()) return false;
      if (object_it->second.find(tag) == object_it->second.end()) return false;
    }
  }

  return true;
}

/*
  FindAny(tags):
    OR query:
      Returns all object ids that contain at least one of the given tags

    Returns:
      empty vector if the input list is empty or no tags match
      otherwise, ids

    Notes:
      input tags are normalized, empty tags are ignored, duplicates are removed,
      and output ids are sorted so results stay stable.
*/
TagManager::IdList TagManager::FindAny(const std::vector<std::string>& tags) const {
  std::vector<std::string> normalized_tags = NormalizeTags(tags);
  if (normalized_tags.empty()) return {};

  IdSet result_set;
  for (const auto& tag : normalized_tags) {
    auto it = tag_to_ids_.find(tag);
    if (it == tag_to_ids_.end()) continue;
    result_set.insert(it->second.begin(), it->second.end());
  }

  IdList result(result_set.begin(), result_set.end());
  SortIdsInPlace(result);
  return result;
}

/*
  FindAll(must_have):
    Returns ids that have all required tags.
*/
TagManager::IdList TagManager::FindAll(const std::vector<std::string>& must_have) const {
  return FindAllExcept(must_have, {});
}

/*
  FindAllExcept(must_have, must_not_have):
    Returns ids that have all required tags and none of the forbidden tags.

    Cases:
      If must_have is empty:
        it returns all registered object ids except the ones with forbidden tags
    Otherwise:
      chooses the smallest required tag bucket as the candidate set,
      then checks the rest of the required tags and forbidden tags for each candidate

    Notes:
      query inputs are normalized first, duplicate tags are removed, and the
      final result is sorted for stable behavior.
*/
TagManager::IdList TagManager::FindAllExcept(
    const std::vector<std::string>& must_have,
    const std::vector<std::string>& must_not_have) const {
  std::vector<std::string> required = NormalizeTags(must_have);
  std::vector<std::string> forbidden = NormalizeTags(must_not_have);

  // If must_have is empty, return all registered objects.
  if (required.empty()) {
    IdList all_ids;
    all_ids.reserve(id_to_tags_.size());
    for (const auto& [id, _] : id_to_tags_) all_ids.push_back(id);

    all_ids.erase(std::remove_if(all_ids.begin(), all_ids.end(),
                                 [this, &forbidden](ObjectId id) {
                                   return std::any_of(
                                       forbidden.begin(), forbidden.end(),
                                       [this, id](const std::string& tag) {
                                         return BucketContainsId(tag, id);
                                       });
                                 }),
                  all_ids.end());

    SortIdsInPlace(all_ids);
    return all_ids;
  }

  // Pick the smallest required tag bucket as the starting point (for speed).
  const IdSet* start_bucket = nullptr;
  std::string start_tag;

  for (const std::string& tag : required) {
    auto it = tag_to_ids_.find(tag);
    if (it == tag_to_ids_.end()) return {};

    if (start_bucket == nullptr || it->second.size() < start_bucket->size()) {
      start_bucket = &it->second;
      start_tag = tag;
    }
  }

  assert(start_bucket != nullptr);

  IdList result;
  result.reserve(start_bucket->size());

  // Filter the candidates from the smallest bucket.
  for (ObjectId id : *start_bucket) {
    bool has_all_required = std::all_of(
        required.begin(), required.end(),
        [this, id, &start_tag](const std::string& tag) {
          return tag == start_tag || BucketContainsId(tag, id);
        });
    if (!has_all_required) continue;

    bool has_forbidden_tag = std::any_of(
        forbidden.begin(), forbidden.end(),
        [this, id](const std::string& tag) { return BucketContainsId(tag, id); });
    if (has_forbidden_tag) continue;

    result.push_back(id);
  }

  SortIdsInPlace(result);
  return result;
}

}  // namespace cse498
