#include "FeatureVector.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>

FeatureSchema::FeatureSchema(std::vector<std::string> names)
    : names_(std::move(names)) {
  index_by_name_.reserve(names_.size());
  for (std::size_t i = 0; i < names_.size(); ++i) {
    if (names_[i].empty()) {
      throw std::invalid_argument("FeatureSchema names must not be empty");
    }

    const auto [_, inserted] = index_by_name_.emplace(names_[i], i);
    if (!inserted) {
      throw std::invalid_argument("FeatureSchema names must be unique");
    }
  }
}

const std::string& FeatureSchema::Name(std::size_t i) const {
  return names_.at(i);
}

bool FeatureSchema::TryIndexOf(std::string_view name, std::size_t& out) const {
  const auto it = index_by_name_.find(name);
  if (it == index_by_name_.end()) {
    return false;
  }

  out = it->second;
  return true;
}

bool FeatureSchema::operator==(const FeatureSchema& other) const noexcept {
  return names_ == other.names_;
}

FeatureVector::FeatureVector(std::shared_ptr<const FeatureSchema> schema)
    : schema_(std::move(schema)) {
  if (!schema_) {
    throw std::invalid_argument("FeatureVector schema must not be null");
  }
  values_.assign(schema_->Size(), 0.0);
}

void FeatureVector::Clear() noexcept {
  std::fill(values_.begin(), values_.end(), 0.0);
}

double FeatureVector::Get(std::size_t i) const {
  return values_.at(i);
}

void FeatureVector::Set(std::size_t i, double v) {
  values_.at(i) = v;
}

bool FeatureVector::TryGet(std::string_view name, double& out) const {
  std::size_t i = 0;
  if (!schema_->TryIndexOf(name, i)) {
    return false;
  }

  out = values_[i];
  return true;
}

bool FeatureVector::TrySet(std::string_view name, double v) {
  std::size_t i = 0;
  if (!schema_->TryIndexOf(name, i)) {
    return false;
  }

  values_[i] = v;
  return true;
}

double FeatureVector::Dot(const FeatureVector& other) const {
  RequireSameSchema_(other);
  return std::inner_product(values_.begin(), values_.end(), other.values_.begin(), 0.0);
}

void FeatureVector::RequireSameSchema_(const FeatureVector& other) const {
  if (schema_.get() == other.schema_.get()) {
    return;
  }
  if (*schema_ == *other.schema_) {
    return;
  }

  throw std::invalid_argument("FeatureVector schema mismatch");
}
