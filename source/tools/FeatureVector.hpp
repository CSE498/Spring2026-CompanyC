#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace cse498 {

class FeatureSchema final {
public:
  explicit FeatureSchema(std::vector<std::string> names);

  std::size_t Size() const noexcept { return names_.size(); }
  const std::string& Name(std::size_t i) const;

  [[nodiscard]] std::optional<std::size_t> IndexOf(std::string_view name) const;
  [[nodiscard]] bool operator==(const FeatureSchema& other) const noexcept;

private:
  struct StringHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view value) const noexcept {
      return std::hash<std::string_view>{}(value);
    }

    std::size_t operator()(const std::string& value) const noexcept {
      return (*this)(std::string_view{value});
    }

  };

  std::vector<std::string> names_;
  std::unordered_map<std::string, std::size_t, StringHash, std::equal_to<>> index_by_name_;
};

class FeatureVector final {
public:
  explicit FeatureVector(std::shared_ptr<const FeatureSchema> schema);

  const FeatureSchema& Schema() const noexcept { return *schema_; }

  void Clear() noexcept;
  std::size_t Size() const noexcept { return values_.size(); }

  double Get(std::size_t i) const;
  [[nodiscard]] std::optional<double> GetByName(std::string_view name) const;
  void Set(std::size_t i, double v);

  [[nodiscard]] bool TrySet(std::string_view name, double v);

  double Dot(const FeatureVector& other) const;

private:
  void RequireSameSchema_(const FeatureVector& other) const;

  std::shared_ptr<const FeatureSchema> schema_;
  std::vector<double> values_;
};

}
