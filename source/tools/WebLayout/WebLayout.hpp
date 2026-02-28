/****************
 * A class to manage the HTML DOM through C++ code.
 * @author Abigail MacKersie
 ****************/

#ifndef CSE498_WEBLAYOUT_HPP
#define CSE498_WEBLAYOUT_HPP

#include <emscripten/val.h>     // For interacting with the HTML DOM
#include <algorithm>            // For sorting and searching
#include <cctype>               // For tolower
#include <memory>               // For object parenthood and lifetimes
#include <string>               // For parsing the HTML DOM and storing text
#include <stdexcept>            // Exceptions
#include <unordered_map>        // DOM data structure
#include <vector>               // For the Node tree structure

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
        std::string key;         // tagName, id, class, attribute key
        std::string value;       // attr value (only for attributeEquals)
    };

    struct Node
    {
        // Identifying features in the tree
        std::string uid;       // Internal Unique ID
        std::string tag;       // tagName (e.g. "div")
        std::string id;        // element.id if exists
        std::vector<std::string> classes;

        // DOM
        emscripten::val element;

        // Tree Structure
        std::unique_ptr<Node> parent;
        std::vector<std::unique_ptr<Node>> children;

        // Constructor
        explicit Node(const std::string& uid_, const emscripten::val& el_, std::unique_ptr<Node> parent_)
            : uid(uid_), element(el_), parent(parent_) {}
    };

    private:
    // Member Variables
    std::unique_ptr<Node> mRoot;                     //! Root element (usually <html>)
    std::unordered_map<std::string, Node*> mUidDOM;  //! Easily traversable map of all elements keyed by uid
    unsigned long long mUidCounter = 1;              //! UID counter

    // Initialization
    void initFromDocument();
    void clearAll();

    /*
        DOM tree building
    */

    // Build from document
    std::unique_ptr<Node> buildTreeFromDom(const emscripten::val& el, std::unique_ptr<Node> parent);

    /*
        Utility
    */

    // Create unique ID for the element
    std::string makeUid() { return "el_" + std::to_string(mUidCounter++); }

    // Set up all Node field metadata
    void hydrateMetaData(std::unique_ptr<Node> n);

    // Ensures that Node pointer is not Null
    void requireValidNode(const std::unique_ptr<Node> n) const { if (!n) throw std::runtime_error("WebLayout: null Node pointer"); }
    
    // Make a full string lowercase
    static std::string toLower(std::string s) 
    {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

    public:

    /*
        Constructor and Destructor
    */

    // Constructor
    WebLayout() { initFromDocument(); }
    // Copy Constructor
    WebLayout(const WebLayout&) = delete;
    // Move Constructor
    WebLayout(WebLayout&&) = delete;
    WebLayout& operator=(const WebLayout&) = delete;
    // Destructor
    ~WebLayout() = default;

};

#endif
