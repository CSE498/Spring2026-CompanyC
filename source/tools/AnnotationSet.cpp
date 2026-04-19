#include "AnnotationSet.hpp"

#include "TagManager.hpp"

#include <algorithm>
#include <cctype>
#include <utility>
#include <span>

namespace cse498 {

namespace {
/// @brief This helper function is used to convert ints in the code to a tagmanager object id
/// @param object_id int representing object's id
/// @return the converted object id
constexpr TagManager::ObjectId ToTagManagerId(int object_id) noexcept {
  return static_cast<TagManager::ObjectId>(object_id);
}

}

/// @brief Constructor for Annotation Set which must have object Id. Initial tags
///default to a blank list
/// @param obj object's id
/// @param in_tags tags taken in
AnnotationSet::AnnotationSet(int obj, const TagSet &in_tags)
    : object_id_(obj) {
  for (std::string tag : in_tags) {
    tag = NormalizeTag_(std::move(tag));
    if (!tag.empty()) {
      tags_.insert(std::move(tag));
    }
  }
}

/// @brief adds a tag to the stored tags
/// @param tag tag to be added
bool AnnotationSet::AddTag(std::string tag) {
  tag = NormalizeTag_(std::move(tag));
  if (tag.empty()) {
    return false;
  }

  const bool inserted = tags_.insert(tag).second;
  if (inserted && tag_manager_ != nullptr) {
    tag_manager_->AddTag(ToTagManagerId(object_id_), tag);
    return true;
  }
  return false;
}


/// @brief Removes a tag
/// @param tag tag to be removed
/// @return bool saying whether removal happens
bool AnnotationSet::RemoveTag(const std::string& tag) {
  const std::string normalized_tag = NormalizeTag_(tag);
  if (normalized_tag.empty()) {
    return false;
  }

  if (tags_.erase(normalized_tag) == 0) {
    return false;
  }

  if (tag_manager_ != nullptr) {
    tag_manager_->RemoveTag(ToTagManagerId(object_id_), normalized_tag);
  }
  return true;
}

/// @brief Finds a tag in tags
/// @param tag tag to be found
/// @return bool representing if the tag is found or not
bool AnnotationSet::FindTag(const std::string& tag) const {
  const std::string normalized_tag = NormalizeTag_(tag);
  return !normalized_tag.empty() && tags_.contains(normalized_tag);
}
/// @brief Checks if any of the tags in the given list are stored in the set
/// @param search_tags list of tags needing to be checked
/// @return bool representing if any are found
bool AnnotationSet::FindAnyTag(std::span<const std::string> search_tags) const {
  return std::ranges::any_of(search_tags,
    [this](const auto& tag) { return FindTag(tag); });
}


/// @brief Checks if all tags in the given list are stored in the set
/// @param search_tags list of tags that need to be matched
/// @return a bool representing if all are in the set or not
bool AnnotationSet::FindAllTags(const std::vector<std::string> search_tags) const {
  return std::ranges::all_of(search_tags.begin(), search_tags.end(),
    [this](const auto& tag) { return FindTag(tag); });
}

/// @brief clears all tags in tag manager and in annotation set
void AnnotationSet::DeleteAllTags() {
  if (tag_manager_ != nullptr) {
    for (const auto& tag : tags_) {
      tag_manager_->RemoveTag(ToTagManagerId(object_id_), tag);
    }
  }
  tags_.clear();
}

/// @brief gets the object id
/// @return object id
int AnnotationSet::GetObjId() const noexcept {
  return object_id_;
}

/// @brief gets tags
/// @return tags
AnnotationSet::TagSet AnnotationSet::GetTags() const {
  return tags_;
}

/// @brief gets size
/// @return how many tags are stored in the set
int AnnotationSet::Size() const noexcept {
  return static_cast<int>(tags_.size());
}

/// @brief helper function to normalize tags
/// @param tag tag given
/// @return the normalized tag
std::string AnnotationSet::NormalizeTag_(std::string tag) {
  std::erase_if(tag, [](unsigned char ch) { return std::isspace(ch) != 0; });
  return tag;
}

/// @brief attaches a tag manager to annotation set
/// @param tm tagmanager needing to be attached
void AnnotationSet::AttachTagManager(TagManager& tm) {
  tag_manager_ = &tm;
  tm.RegisterObject(ToTagManagerId(object_id_));
  for (const auto& tag : tags_) {
    tm.AddTag(ToTagManagerId(object_id_), tag);
  }
}

}