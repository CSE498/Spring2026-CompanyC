[Directory](/source/tools/WeightedSet.hpp)

# WeightedSet Tests

## **1** Classes

- WeightedSet<T>  
  - Key-value container with weighted sampling support

---

## **2** Test Overview

- Insert / erase / set_weight operations
- Total weight tracking correctness
- Weighted sampling correctness (cumulative + random)
- Floating-point precision handling
- Edge cases (empty set, zero weights)
- Iteration correctness (for_each)
- Equality / inequality operators

Type:
- Unit tests

---

## **3** Failure Conditions

- Incorrect weight tracking
- Broken sampling distribution
- Floating-point precision errors
- Invalid iterator behavior
- Duplicate insertion issues
- Equality operator failure

---

## **4** Bug Log

- None reported