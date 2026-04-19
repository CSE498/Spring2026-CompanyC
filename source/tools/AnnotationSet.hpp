#pragma once

#include <set>
#include <string>
#include <vector>
#include <span>
namespace cse498 {

class TagManager;

class AnnotationSet {
public:
  using TagSet = std::set<std::string>;

  AnnotationSet() = delete;
  explicit AnnotationSet(int obj, const TagSet& tags = {});

  bool AddTag(std::string);
  template <typename ...T>
  void AddTags(T&&... tags){
    (AddTag(std::string(std::forward<T>(tags))), ...);
  };

  
  bool RemoveTag(const std::string& tag);
  template <typename ...T>
  [[nodiscard]] bool RemoveTags(T&&... tags){
    return (RemoveTag(std::string(std::forward<T>(tags))) && ...);
  };

  [[nodiscard]] bool FindTag(const std::string& tag) const;
  [[nodiscard]] bool FindAnyTag(std::span<const std::string> search_tags) const;
  [[nodiscard]] bool FindAllTags(const std::vector<std::string> search_tags) const;

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
