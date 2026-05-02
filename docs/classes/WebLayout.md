# WebLayout
Documented By: Abigail MacKersie

Developed By: Abigail MacKersie

## **0** Introduction
This class serves as the initializer for the shell of the DOM upon application startup. 

## **1** Structural Elements

### **1.1** Member Variables

#### **1.1.1** Private Member Variables
- mRoot (std::unique_ptr(Node)) - the root of the DOM tree map
- mUidDOM (std::unordered_map(std::string, Node*)) - all nodes keyed by a unique identifier for O(1) lookup
- mUidCounter (unsigned long long) - counter to make all DOM elements be keyed to a UID that corresponds to their time of entry

### **1.2** Enum Classes

#### **1.2.1** SearchKind
An enum class to query for an element in the DOM with some specified attribute.

- tagName - search by tag name
- id - search by element.id if it exists
- className - search by name of item class attribute
- attributeEquals - key/value pair of an attribute name and its desired value
- attributeExists - key only attribute name

### **1.3** Structs

#### **1.3.1** Query
A struct to form a query for the mUidDOM map recreation to search for a specific element

**1.3.1.1** Member Variables
- kind (SearchKind) - the kind of query to make
- key (std::string) - tagName, id, class, attribute key for the query
- value (std::string) - only for attributeEquals SearchKind type. records attribute value

#### **1.3.2** Node
A struct to record all of the meta information for each element added into the DOM

**1.3.2.1** Member Variables
- uid (std::string) - internal unique ID that only exists inside the C++ code
- tag (std::string) - stores the tagName (e.g. "div")
- id (std::string) - element.id if it exists
- classes (std::vector(std::string)) - classList entries of HTML classes
- element (emscripten::val) - the actual element inside the DOm
- parent (Node*) - a pointer to the parent node without ownership (parent is not deleted if child is deleted)
- children (std::vector(std::unique_ptr(Node))) - a vector of pointers to the child nodes with ownership (children are automatically deleted if parent is deleted)

**1.3.2.2** Functions
- An explicit constructor with the necessary attributes (a generated uid, the element itself, and the parent of the element)

## **2** Functions

### **2.1** Static Functions
- static std:string toLower(std::string s) - private helper function to convert a string into all lowercase to help with element parsing

### **2.2** Private Functions
- void initFromDocument() - initialize the DOM from a present HTML document
- void clearAll() - clears the internal unordered_map of all data
- std::unique_ptr(Node) buildTreeFromDom(const emscripten::val& el, Node* parent) - builds the unordered map from the DOM
- std::string makeUid() - generates a UID w/ the mUidCounter
- void hydrateMetaData(Node* n) - hydrates the meta data of a particular Node
- void eraseSubtreeFromMap(Node* node) - deletes a subtree out of the unordered map
- void requireValidNode(const Node* n) const - throws error if the Node pointer is not initialized (nullptr)

### **2.3** Public Functions
- Constructor (initFromDocument) and Destructor (default)
- Copy Constructor and Copy Operator DELETED
- Move Function DELETED
- Node* getRoot() const - basic access to the root of the unordered map
- Node* createElement(Node* parent, const std::string& tag, const std::string& id, const std::vector(std::string) classes) - creates a new element in the DOM and appends it to a target parent element
- Node* findById(const std::string& id) const - helper function that finds an element in the DOM unordered map by UID
- void removeAllChildren(Node* parent) - helper function to remove all child nodes from an element
- void createShell() - initializes the shell of the DOM and adds the necessary elements

## **3** Dependencies
- The Emscripten library
- algorithm
- cctype
- memory
- string
- stdexcept
- unordered_map
- vector