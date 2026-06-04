[Directory](/tests/tools/RandomTest.cpp)

# RandomTest.cpp

## **1** Classes

- Random  
  - Deterministic pseudo-random number generator

---

## **2** Test Overview

- Deterministic behavior with same seed
- GetInt range correctness
- GetDouble range correctness
- Probability function correctness (P)
- Copy constructor correctness
- Edge cases (degenerate and large ranges)
- Statistical sanity checks

Type:
- Unit tests

---

## **3** Failure Conditions

- Non-deterministic output with same seed
- Out-of-range integers
- Invalid floating-point values
- Broken probability logic
- Copy state corruption
- Distribution bias or off-by-one errors

---

## **4** Bug Log

- None reported