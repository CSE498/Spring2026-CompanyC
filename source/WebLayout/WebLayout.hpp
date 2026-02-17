/****************
 * A class to manage the HTML DOM through C++ code.
 * @author Abigail MacKersie
 ****************/

#ifndef WEBLAYOUT
#define WEBLAYOUT

#include <emscripten/val.h>     // For interacting with the HTML DOM
#include <algorithm>            // For sorting & searching
#include <unordered_map>        // DOM data structure
#include <vector>               // For the Node tree structure
#include <memory>               // For object parenthood and lifetimes
#include <string>               // For parsing the HTML DOM and storing text
#include <stdexcept>            // Exceptions
using std::vector, std::unique_ptr, std::string;

/****************
 * Will parse and store the information within the HTML DOM through
 * a Tree-structured Node system
 * Can be used to edit, access, and delete HTML DOM
 * elements through the class functions
 ****************/
class WebLayout 
{
    public:
    /*****
     * Helper structs to create queries and parse objects
     *****/

    // The kind of query to find an object in the HTML DOM
    enum class SearchKind 
    {
        tagName,
        id,
        className,
        attributeEquals,    // key value pair
        attributeExists     // key only
    };

    // A structure to compose a query for an HTML element in the DOM
    struct Query
    {
        SearchKind kind;
        string key;         // tagName, id, class, attribute key
        string value;       // attr value (only for attributeEquals)
    };

    struct Node
    {
        // Identifying features in the tree
        string uid;       // Internal Unique ID
        string tag;       // tagName (e.g. "div")
        string id;        // element.id if exists
        vector<string> classes;

        // DOM
        emscripten::val element;

        // Tree Structure
        Node* parent = nullptr;
        vector<unique_ptr<Node>> children;

        // Constructor
        explicit Node(const string& uid_, const emscripten::val& el_, Node* parent_)
            : uid(uid_), element(el_), parent(parent_) {}
    };

    private:
    // Member Variables
    unique_ptr<Node> mRoot;                     //! Root element (usually <html>)
    std::unordered_map<string, Node*> mUidDOM;  //! Easily traversable map of all elements keyed by uid
    unsigned long long mUidCounter;             //! UID counter

    // Initialization
    void initFromDocument();
    void clearAll();

    public:

    /*
        Constructor and Destructor
    */

    // Constructor
    WebLayout() { initFromDocument(); }
    // Destructor
    ~WebLayout() = default;


    /*
        DOM tree building
    */

    // Build from document
    unique_ptr<Node> buildTreeFromDom(const emscripten::val& el, Node* parent);


    /*
        Utility
    */

    // Set up all Node field metadata
    void hydrateMetaData(Node* n);

    // Ensures that Node pointer is not Null
    void requireValidNode(const Node* n) const { if (!n) throw std::runtime_error("WebLayout: null Node pointer"); }

    // Create unique ID for the element
    string makeUid() { return "el_" + std::to_string(mUidCounter++); }
    
    // Make a full string lowercase
    static string toLower(string s) 
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

};

#endif