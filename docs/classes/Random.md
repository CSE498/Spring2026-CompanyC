[Directory](/source/tools/Random.hpp)

# Random  
Documented By: Truong Phan  
Developed By: Benjamin Forbes  

## **0** Introduction  
The `Random` class provides deterministic pseudo-random generation using `std::mt19937`. Supports ints, doubles, and probability checks.

## **1** Structural Elements  

### **1.1** Member Variables  

#### **1.1.1** Private Member Variables  
- mEngine (std::mt19937)  
- mSeeded (bool)  

## **2** Functions  

### **2.1** Public Functions  
- Random()  
- Random(seed)  
- Seed(seed)  
- GetInt(min, max)  
- GetDouble(min, max)  
- P(probability)  

## **3** Dependencies  
- random  
- cstdint  
- assert  

## **4** Known Issues  
None  

## **5** Compiler Flags  
None  