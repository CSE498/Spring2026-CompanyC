/**
 * @file MemoryFactory.hpp
 * @author Joshua Thomas
 * @brief Fixed size-pool allocator for type T with a freelist for fast reuse
 *
 * @details
 *  - Make(...) returns a raw T* that must be returned via Delete(ptr).
 *  - MakeUnique(...) returns a std::unique_ptr with a custom deleter
 thatautomatically returns the object to the pool when it is destroyed or reset
 (this method is generally preferred).
 *
 * Usage notes (IMPORTANT):
 *  - The pool must outlive any UniquePtr created from it (deleter stores a pool
 pointer).
 *  - Delete(ptr) destroys the object and returns its slot to the freelist; it
 does not release the underlying block memory until the MemoryFactory is
 destroyed.
 **/

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
  /**
   * @brief Constructs Memoryfactory and pre-allocates an initial number of
   *blocks.
   * @param initialBlocks Number of blocks to allocate immediately (defaults to
   *1).
   **/
  explicit MemoryFactory(std::size_t initialBlocks = 1) {
    mBlocks.reserve(initialBlocks);
    for (std::size_t i = 0; i < initialBlocks; ++i) {
      AddBlock();
    }
  }

  /**
   * @brief destroys the factory and releases all allocated blocks.
   * @details This releases raw block memory. It assumes all live objects have
   *already been destroyed/returned.
   **/
  ~MemoryFactory() {
    for (void *block : mBlocks) {
      ::operator delete(block, std::align_val_t(alignof(T)));
    }
  }

  MemoryFactory(const MemoryFactory &) = delete;
  MemoryFactory &operator=(const MemoryFactory &) = delete;
  MemoryFactory(MemoryFactory &&) = delete;
  MemoryFactory &operator=(MemoryFactory &&) = delete;

  // Start: Code taken from AI
  /**
   * @brief custom deleter uses by UniquePtr to return objects to this pool.
   **/
  struct Deleter {
    MemoryFactory *pool{};
    void operator()(T *p) const noexcept {
      if (p)
        pool->Delete(p);
    }
  };
  /**
   * @brief Owning smart pointer that returns objects to the pool on
   *desctruction.
   **/
  using UniquePtr = std::unique_ptr<T, Deleter>;

  /**
   * @brief Allocates and constructs a T returning an owning UniquePtr
   * @param Args constructor argument types for T (template pack)
   * @return UniquePtr managing the object. Destruction returns a slot to the
   *pool.
   **/
  template <typename... Args> UniquePtr MakeUnique(Args &&...args) {
    return UniquePtr(Make(std::forward<Args>(args)...), Deleter{this});
  }
  // End: Code taken from AI

  /**
   * @brief Allocates one slot from the pool and constructs a T.
   * @param Args constructor argument types for T (template pack)
   * @return Pointer to the newly constructed object.
   **/
  template <typename... Args> T *Make(Args &&...args) {
    void *mem = AllocateSlot();
    try {
      return ::new (mem) T(std::forward<Args>(args)...);
    } catch (...) {
      mFree.Push(mem);
      throw;
    }
  }

  /**
   * @brief Destroys an object and returns its slot to the freelist.
   * @param ptr Pointer previously returned by Make (or managed by UniquePtr)
   **/
  void Delete(T *ptr) noexcept {
    if (!ptr)
      return;
    ptr->~T();
    mFree.Push(ptr);
  }

  /**
   * @brief Ensures at least a given number of blocs are allocated
   * @param blocks minimum # of blocks to have allocated.
   **/
  void ReserveBlocks(std::size_t blocks) {
    mBlocks.reserve(blocks);
    while (mBlocks.size() < blocks) {
      AddBlock();
    }
  }

private:
  std::vector<void *> mBlocks;
  FreeList mFree;

  /**
   * @brief Rounds n up to the next multiple of align.
   * @param n Value to round up.
   * @param align Aligment multiple
   * @return n rounded up to the next multiple of align
   **/
  static constexpr std::size_t RoundUp(std::size_t n, std::size_t align) {
    return (n + align - 1) &
           ~(align -
             1); // source: alignment formula
                 // https://stackoverflow.com/questions/45213511/formula-for-memory-alignment
  }

  // alignment stuff can be constexpr as the type (and sizeof(type)) should be
  // known at compile time for initalize block allocation
  static constexpr std::size_t SlotAlign = alignof(T);

  /**
   *@brief Raw slot size before alignment rounding
   **/
  static constexpr std::size_t SlotSizeRaw =
      (sizeof(T) > sizeof(void *))
          ? sizeof(T)
          : sizeof(void *); // edge case here:  if the amount of bytes for a
                            // data type is less than a pointer (like a char)
  // then the slotsize should default to void*

  /**
   * @brief Byte stride between slot starts within a block.
   **/
  static constexpr std::size_t SlotStride = RoundUp(SlotSizeRaw, SlotAlign);

  /**
   * @brief Pops a free slot from the freelist and grows pool if its empty
   * @return Pointer to a free slot.
   **/
  void *AllocateSlot() {
    void *slot = mFree.Pop();
    if (!slot) {
      AddBlock();
      slot = mFree.Pop();
    }
    return slot;
  }

  /**
   * @brief Allocates a new block and pushes its slots onto the freelist.
   **/
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
