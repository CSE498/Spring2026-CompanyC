[Directory](/source/tools/Memofunction.hpp)

# MemoFunction  
Documented By: Truong Phan  
Developed By: Jose Antonio Hernandez-Martinez  

## **0** Introduction  
The `MemoFunction` class caches function results using FIFO eviction with fixed capacity.

## **1** Structural Elements  

### **1.1** Member Variables  

#### **1.1.1** Static Member Variables  
- DEFAULT_CAPACITY (std::size_t)  

#### **1.1.2** Private Member Variables  
- cache (std::unordered_map)  
- mfunc (Func)  
- capacity (std::size_t)  
- eviction_order (std::list)  
- hit/miss counters  

## **2** Functions  

### **2.1** Public Functions  
- MemoFunction(func)  
- MemoFunction(func, capacity)  
- operator()(key)  
- clear()  
- set_capacity()  
- size()  
- get_capacity()  
- get_hits()  
- get_misses()  
- hit_rate()  

## **3** Dependencies  
- unordered_map  
- list  
- functional  
- type_traits  

## **4** Known Issues  
None  

## **5** Compiler Flags  
None  