#pragma once
#include <cstddef>

class FreeList {
public:
  FreeList() = default;
  FreeList(const FreeList &) = delete;
  FreeList &operator=(const FreeList &) = delete;

  bool Empty() const noexcept { return mHead == nullptr; }
  std::size_t Size() const noexcept { return mCount; }

  void Push(void *p) noexcept {
    Node *n = static_cast<Node *>(p);
    n->next = mHead;
    mHead = n;
    ++mCount;
  }

  void *Pop() noexcept {
    if (!mHead)
      return nullptr;
    Node *n = mHead;
    mHead = mHead->next;
    --mCount;
    return n;
  }

  void Clear() noexcept {
    mHead = nullptr;
    mCount = 0;
  }

private:
  struct Node {
    Node *next;
  };
  Node *mHead = nullptr;
  std::size_t mCount = 0;
};
