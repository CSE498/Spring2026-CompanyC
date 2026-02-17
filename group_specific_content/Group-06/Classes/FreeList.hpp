/**
 * @file FreeList.hpp
 * @author Joshua Thomas
 * @brief Singly-linked freelist for managing reusable memory slots.
 **/

#pragma once
#include <cstddef>

class FreeList {
public:
  /**
   * @brief Constructs an empty freelist.
   **/
  FreeList() = default;
  FreeList(const FreeList &) = delete;
  FreeList &operator=(const FreeList &) = delete;

  /**
   * @brief Checks whether the freelist is empty.
   * @returns True if there are no free slots available.
   **/
  bool Empty() const noexcept { return mHead == nullptr; }

  /**
   * @brief Returns the number of slots currently in the freelist.
   * @return Number of free slots tracked.
   **/
  std::size_t Size() const noexcept { return mCount; }

  /**
   * @brief Pushes a memory slot back onto the freelist.
   * @param P Pointer to a memory slot.
   **/
  void Push(void *p) noexcept {
    Node *n = static_cast<Node *>(p);
    n->next = mHead;
    mHead = n;
    ++mCount;
  }

  /**
   * @brief Pops one free slot from the freelist.
   * @return Pointer to a free slot, or nullptr if nothing is in the free list.
   **/
  void *Pop() noexcept {
    if (!mHead)
      return nullptr;
    Node *n = mHead;
    mHead = mHead->next;
    --mCount;
    return n;
  }
  /**
   * @brief Clears the freelist WITHOUT freeing any memory.
   **/
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
