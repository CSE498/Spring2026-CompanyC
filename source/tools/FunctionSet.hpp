#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace cse498 {

template <typename Signature>
class FunctionSet;

template <typename R, typename... Args>
class FunctionSet<R(Args...)> {
public:
  using FunctionType = std::function<R(Args...)>;

  FunctionSet() = default;

  template <typename F>
    requires std::constructible_from<FunctionType, F>
  void Add(F&& fn) {
    functions_.emplace_back(std::forward<F>(fn));
  }

  void Clear() noexcept {
    functions_.clear();
  }

  void CallAll(Args... args) const {
    for (const auto& fn : functions_) {
      fn(args...);
    }
  }

  [[nodiscard]] bool Empty() const noexcept {
    return functions_.empty();
  }

  [[nodiscard]] std::size_t Size() const noexcept {
    return functions_.size();
  }

private:
  std::vector<FunctionType> functions_;
};

}
