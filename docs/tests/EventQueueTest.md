[Directory](/tests/tools/EventQueueTest.cpp)

# EventQueueTest.cpp

## **1** Classes

- EventQueue<T>  
- Event<T>  

---

## **2** Test Overview

- Event construction and getters
- Push / Pop / Top / PopTop behavior
- Size / Empty / Clear correctness
- Priority ordering correctness
- FIFO tiebreaking behavior
- Negative priority handling
- Deep copy correctness
- Stress testing (1000+ events)
- Template type support

Type:
- Unit tests + stress tests

---

## **3** Failure Conditions

- Incorrect priority ordering
- Broken FIFO tiebreaking
- Heap corruption
- Wrong pop/top behavior
- Incorrect size/empty state
- Shallow copy bugs
- Template instantiation failures
- Stress test instability

---

## **4** Bug Log

- None reported