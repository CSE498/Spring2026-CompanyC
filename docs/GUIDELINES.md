# Guidelines and Standards
This document details the naming conventions for the individual classes, modules, APIs, and documentation files inside this code base.

## C++ Classes (HPP and CPP Files)
This section will outline the standards for naming and documentation of CPP files.

### **1** File Names
Both .cpp files and .hpp files should be named with the class name, with each unique word in the name capitalized. (e.g. WebTextbox.cpp, ExampleClass.cpp, Timer.hpp)

### **2** File Location
All C++ class files (both header and code together) may be found under the **tools** directory of the code base. No individual folders should be made to house them.

### **3** Member Variable Names
All member variables, both public and private, should be prefixed by a lowercase 'm' and the first letter of each individual word should be capitalized. (e.g mRoot, mTree, mImageName, mUserCursorImage). 

### **4** Function Names & Struct Names
The first letter of each individual word should be capitalized. (e.g. BuildFromTree, StartWorld, ListAgents, HelperStruct)

### **5** Error Handling
Using std::optional for all error handling.

Exceptions are not allowed due to the inclusion of the Emscripten library.

### **6** In-line Documentation
All class files are documented in-line according to the Doxygen standards. Checks will be run on the code base weekly to ensure that all files conform to these standards. 

## Test Files
All test files should be named after the class or module that they are testing, followed by the word "Test" and the first letter of every word should be capitalized.

C++ tests must use the Catch library for test structure and execution. UI tests must be compiled and run using (TBD).

Test files will be automatically run with every major push into the repository, and new failures will be documented and reported. 

In-line documentation TBD.

## Documentation Files
All documentation files should be named after the file/class/module they are documenting.