[Directory](/source/tools/DataFileManager.hpp)

# DataFileManager  
Documented By: Truong Phan  
Developed By: John Masterman  

## **0** Introduction  
The `DataFileManager` class provides a lightweight CSV logging system. It allows users to register named columns backed by callable functions and periodically write computed rows to a file. It is primarily used for runtime data capture in simulations, debugging, or telemetry-style logging.

## **1** Structural Elements  

### **1.1** Member Variables  

#### **1.1.1** Static Member Variables  
None  

#### **1.1.2** Private Member Variables  
- mFile (std::ofstream) - Output CSV file stream  
- mColumns (std::vector<Column>) - Registered columns  
- mHeaderWritten (bool) - Tracks whether header has been written  

#### **1.1.3** Public Member Variables  
None  

### **1.2** Enum Classes  
None  

### **1.3** Structs  

#### **1.3.1** Column  
Represents a CSV column.

**Member Variables**
- mName (std::string) - Column name  
- mFun (std::function<std::string()>) - Value generator  

## **2** Functions  

### **2.1** Static Functions  
None  

### **2.2** Private Functions  
- WriteHeaderIfNeeded() - Writes header once  

### **2.3** Public Functions  
- DataFileManager()  
- Open(filename)  
- Close()  
- AddColumn(name, func)  
- Update()  
- Flush()  

## **3** Dependencies  
- fstream  
- string  
- vector  
- functional  

## **4** Known Issues  
None  

## **5** Compiler Flags  
None  