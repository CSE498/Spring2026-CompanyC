[Directory](/tests/tools/MemofunctionTest.cpp)

# MemofunctionTest.cpp

## **1** Classes

- Memofunction<Key, Func>  
  - FIFO memoization cache with fixed capacity

---

## **2** Test Overview

- Cache initialization
- Cache hit/miss correctness
- FIFO eviction policy
- Capacity enforcement
- Clear functionality
- Template type support
- Function call tracking
- Multi-type correctness (int, string)
- Stress testing behavior

Type:
- Unit tests

---

## **3** Failure Conditions

- Incorrect caching behavior
- Cache hits recompute values
- FIFO eviction broken
- Capacity overflow
- Clear does not reset state
- Function call tracking incorrect
- Iterator/list desync
- Template instantiation failure

---

## **4** Bug Log

- None reported