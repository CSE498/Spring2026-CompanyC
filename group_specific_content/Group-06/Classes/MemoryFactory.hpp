
#pragma once
#include "FreeList.hpp"
#include <cstddef>
#include <memory>
#include <new>
// #include <type_traits>
#include <utility>
#include <vector>

template <typename T, std::size_t BlockCount = 1024> class MemoryFactory {
public:
  explicit MemoryFactory(std::size_t initialBlocks = 1) {
    mBlocks.reserve(initialBlocks);
    for (std::size_t i = 0; i < initialBlocks; ++i) {
      AddBlock();
    }
  }

  ~MemoryFactory() {
    for (void *block : mBlocks) {
      ::operator delete(block, std::align_val_t(alignof(T)));
    }
  }

  MemoryFactory(const MemoryFactory &) = delete;
  MemoryFactory &operator=(const MemoryFactory &) = delete;
  MemoryFactory(MemoryFactory &&) = delete;
  MemoryFactory &operator=(MemoryFactory &&) = delete;

  template <typename... Args> T *Make(Args &&...args) {
    void *mem = AllocateSlot();
    try {
      return ::new (mem) T(std::forward<Args>(args)...);
    } catch (...) {
      mFree.Push(mem);
      throw;
    }
  }
  // Start: Code taken from AI
  struct Deleter {
    MemoryFactory *pool{};
    void operator()(T *p) const noexcept {
      if (p)
        pool->Delete(p);
    }
  };

  using UniquePtr = std::unique_ptr<T, Deleter>;

  template <typename... Args> UniquePtr MakeUnique(Args &&...args) {
    return UniquePtr(Make(std::forward<Args>(args)...), Deleter{this});
  }
  // End: Code taken from AI

  void Delete(T *ptr) noexcept {
    if (!ptr)
      return;
    ptr->~T();
    mFree.Push(ptr);
  }

  void ReserveBlocks(std::size_t blocks) {
    mBlocks.reserve(blocks);
    while (mBlocks.size() < blocks) {
      AddBlock();
    }
  }

private:
  std::vector<void *> mBlocks;
  FreeList mFree;

  static constexpr std::size_t RoundUp(std::size_t n, std::size_t align) {
    return (n + align - 1) &
           ~(align -
             1); // source: alignment formula
                 // https://stackoverflow.com/questions/45213511/formula-for-memory-alignment
  }

  // alignment stuff can be constexpr as the type (and sizeof(type)) should be
  // known at compile time for initalize block allocation
  static constexpr std::size_t SlotAlign = alignof(T);

  static constexpr std::size_t SlotSizeRaw =
      (sizeof(T) > sizeof(void *))
          ? sizeof(T)
          : sizeof(void *); // edge case here:  if the amount of bytes for a
                            // data type is less than a pointer (like a char)
  // then the slotsize should default to void*

  static constexpr std::size_t SlotStride = RoundUp(SlotSizeRaw, SlotAlign);

  void *AllocateSlot() {
    void *slot = mFree.Pop();
    if (!slot) {
      AddBlock();
      slot = mFree.Pop();
    }
    return slot;
  }

  void AddBlock() {
    const std::size_t bytes = SlotStride * BlockCount;
    void *block = ::operator new(bytes, std::align_val_t(alignof(T)));
    mBlocks.push_back(block);
    char *start = static_cast<char *>(block);
    for (std::size_t i = 0; i < BlockCount; ++i) {
      mFree.Push(start + (i * SlotStride));
    }
  }
};
