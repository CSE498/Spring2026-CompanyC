[Directory](/source/tools/EventQueue.hpp)
# EventQueue  
Documented By: Truong Phan  
Developed By: Truong Phan  

## **0** Introduction  
The `EventQueue` class implements a priority queue using a min-heap. Events are ordered by priority, then by insertion order for ties.

## **1** Structural Elements  

### **1.1** Member Variables  

#### **1.1.1** Private Member Variables  
- mHeap (std::vector<Event<T>>) - Heap storage  
- mInsertionIndex (std::size_t) - Tiebreaker index  

### **1.2** Structs  

#### **1.2.1** Comparator  
Defines heap ordering.

- operator() - priority + FIFO tie-break  

## **2** Functions  

### **2.1** Public Functions  
- EventQueue()  
- ~EventQueue()  
- Push(Event<T>)  
- Push(data, priority)  
- Pop()  
- Top()  
- PopTop()  
- Size()  
- Empty()  
- Clear()  

## **3** Dependencies  
- vector  
- algorithm  
- optional  
- utility  

## **4** Known Issues  
None  

## **5** Compiler Flags  
None  