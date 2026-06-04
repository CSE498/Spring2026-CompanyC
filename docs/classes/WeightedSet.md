[Directory](/source/tools/WeightedSet.hpp)

# WeightedSet  
Documented By: Truong Phan  
Developed By: George Almeida  

## **0** Introduction  
The `WeightedSet` stores unique elements with weights and supports weighted random sampling.

## **1** Structural Elements  

### **1.1** Member Variables  

#### **1.1.1** Private Member Variables  
- m_map (std::map<T, double>)  
- m_total (double)  

## **2** Functions  

### **2.1** Public Functions  
- insert  
- erase  
- set_weight  
- find  
- size  
- empty  
- clear  
- total_weight  
- sample_by_weight  
- sample_random  
- begin/end  
- for_each  
- swap  
- operator== / !=  

## **3** Dependencies  
- map  
- random  
- concepts  
- type_traits  

## **4** Known Issues  
None  

## **5** Compiler Flags  
None  