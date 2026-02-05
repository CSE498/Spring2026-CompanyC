#pragma once
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <new>
#include <utility>
#include <vector>

template <typename T, std::size_t BlockCount =
                          1024> // T is the type of the pool, and BlockCount is
                                // the amount of that type (default to 1024)
class MemoryFactory {
  static_assert(BlockCount > 0);

public:
  explicit MemoryFactory(std::size_t initialBlocks = 1) {
    mBlocks.reserve(initialBlocks);
    mFree.reserve(initialBlocks * BlockCount);

    for (std::size_t i = 0; i < initialBlocks; i++) {
      AddBlock();
    }
  }

  ~MemoryFactory() {
    for (void *block : mBlocks) {
      ::operator delete(block);
    }
  }

  MemoryFactory(const MemoryFactory &) = delete; // No copying
  MemoryFactory &operator=(const MemoryFactory &&) = delete;
  MemoryFactory(MemoryFactory &&) = delete; // no move for now

  MemoryFactory &operator=(MemoryFactory &&) = delete;
  template <typename... Args> T *Make(Args &&...args) {
    void *mem = AllocateSlot();
    try {
      return ::new (mem) T(std::forward<Args>(args)...);
    } catch (...) {
      FreeSlot(mem);
      throw;
    }
  }

  void Delete(T *ptr) {
    if (!ptr)
      return;
    ptr->~T();
    FreeSlot(ptr);
  }

  void ReserveBlocks(std::size_t blocks) {
    mBlocks.reserve(blocks);
    mFree.reserve(blocks * BlockCount);
    while (mBlocks.size() < blocks) {
      AddBlock();
    }
  }
  // Will implement a more robust freelist implementation
private:
  std::vector<void *> mBlocks; // raw blocks
  std::vector<void *> mFree;   // free slots

  void *AllocateSlot() {
    if (mFree.empty()) {
      AddBlock();
    }
    void *slot = mFree.back();
    mFree.pop_back();
    return slot;
  }

  void AddBlock() {
    const std::size_t bytes = sizeof(T) * BlockCount;
    void *block = ::operator new(bytes);
    mBlocks.push_back(block);

    char *start = static_cast<char *>(block);
    for (std::size_t i = 0; i < BlockCount; ++i) {
      mFree.push_back(start + (i * sizeof(T)));
    }
  }
  void FreeSlot(void *p) { mFree.push_back(p); }
};
