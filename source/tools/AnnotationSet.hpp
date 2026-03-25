#pragma once

#include <set>
#include <string>
#include <vector>

namespace cse498 {

class TagManager;

class AnnotationSet {
public:
  using TagSet = std::set<std::string>;

  AnnotationSet() = delete;
  AnnotationSet(int obj, TagSet tags = {});

  void AddTag(std::string tag);
  void AddTags(const std::vector<std::string>& added_tags);

  bool RemoveTags(const std::vector<std::string>& removed_tags);
  bool RemoveTag(const std::string& tag);

  [[nodiscard]] bool FindTag(const std::string& tag) const;
  [[nodiscard]] bool FindAnyTag(const std::vector<std::string>& search_tags) const;
  [[nodiscard]] bool FindAllTags(const std::vector<std::string>& search_tags) const;

  void DeleteAllTags();

  [[nodiscard]] int GetObjId() const noexcept;
  [[nodiscard]] int Size() const noexcept;
  [[nodiscard]] TagSet GetTags() const;

  void AttachTagManager(TagManager& tm);

private:
  [[nodiscard]] static std::string NormalizeTag_(std::string tag);

  int object_id_;
  TagSet tags_;
  TagManager* tag_manager_ = nullptr;
};

}
