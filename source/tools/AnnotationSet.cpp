#include "AnnotationSet.hpp"

#include "TagManager.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace cse498 {

namespace {

constexpr TagManager::ObjectId ToTagManagerId(int object_id) noexcept {
  return static_cast<TagManager::ObjectId>(object_id);
}

}

AnnotationSet::AnnotationSet(int obj, TagSet in_tags)
    : object_id_(obj) {
  for (std::string tag : in_tags) {
    tag = NormalizeTag_(std::move(tag));
    if (!tag.empty()) {
      tags_.insert(std::move(tag));
    }
  }
}

void AnnotationSet::AddTag(std::string tag) {
  tag = NormalizeTag_(std::move(tag));
  if (tag.empty()) {
    return;
  }

  const bool inserted = tags_.insert(tag).second;
  if (inserted && tag_manager_ != nullptr) {
    tag_manager_->AddTag(ToTagManagerId(object_id_), tag);
  }
}

void AnnotationSet::AddTags(const std::vector<std::string>& added_tags) {
  for (const auto& tag : added_tags) {
    AddTag(tag);
  }
}

bool AnnotationSet::RemoveTags(const std::vector<std::string>& removed_tags) {
  bool all_removed = true;
  for (const auto& tag : removed_tags) {
    all_removed = RemoveTag(tag) && all_removed;
  }
  return all_removed;
}

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

bool AnnotationSet::FindTag(const std::string& tag) const {
  const std::string normalized_tag = NormalizeTag_(tag);
  return !normalized_tag.empty() && tags_.contains(normalized_tag);
}

bool AnnotationSet::FindAnyTag(const std::vector<std::string>& search_tags) const {
  return std::any_of(search_tags.begin(), search_tags.end(),
                     [this](const auto& tag) { return FindTag(tag); });
}

bool AnnotationSet::FindAllTags(const std::vector<std::string>& search_tags) const {
  return std::all_of(search_tags.begin(), search_tags.end(),
                     [this](const auto& tag) { return FindTag(tag); });
}

void AnnotationSet::DeleteAllTags() {
  if (tag_manager_ != nullptr) {
    for (const auto& tag : tags_) {
      tag_manager_->RemoveTag(ToTagManagerId(object_id_), tag);
    }
  }
  tags_.clear();
}

int AnnotationSet::GetObjId() const noexcept {
  return object_id_;
}

AnnotationSet::TagSet AnnotationSet::GetTags() const {
  return tags_;
}

int AnnotationSet::Size() const noexcept {
  return static_cast<int>(tags_.size());
}

std::string AnnotationSet::NormalizeTag_(std::string tag) {
  tag.erase(std::remove_if(tag.begin(), tag.end(),
                           [](unsigned char ch) { return std::isspace(ch) != 0; }),
            tag.end());
  return tag;
}

void AnnotationSet::AttachTagManager(TagManager& tm) {
  tag_manager_ = &tm;
  tm.RegisterObject(ToTagManagerId(object_id_));
  for (const auto& tag : tags_) {
    tm.AddTag(ToTagManagerId(object_id_), tag);
  }
}

}