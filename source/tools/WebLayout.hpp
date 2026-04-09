// WebLayout.hpp
/****************
 * A class to manage the HTML DOM through C++ code.
 * Minimal layout shell support for the CSE 498 web UI.
 * @author Abigail MacKersie
 ****************/

#ifndef CSE498_WEBLAYOUT_HPP
#define CSE498_WEBLAYOUT_HPP

#include <emscripten/val.h>     // For interacting with the HTML DOM
#include <algorithm>            // For transform
#include <cctype>               // For tolower
#include <memory>               // For unique_ptr
#include <string>               // For parsing/storing text
#include <stdexcept>            // Exceptions
#include <unordered_map>        // UID map
#include <vector>               // Tree structure

namespace cse498
{

/****************
 * Parses and stores information within the HTML DOM through
 * a tree-structured Node system.
 * Can be used to create, access, and delete HTML DOM
 * elements through class functions.
 ****************/
class WebLayout
{
public:
    /*****
     * Helper structs to create queries and parse objects
     *****/

    enum class SearchKind
    {
        tagName,
        id,
        className,
        attributeEquals,    // key/value pair
        attributeExists     // key only
    };

    struct Query
    {
        SearchKind kind;
        std::string key;    // tagName, id, class, attribute key
        std::string value;  // attribute value (only for attributeEquals)
    };

    struct Node
    {
        // Identifying features in the tree
        std::string uid;                    // Internal unique ID
        std::string tag;                    // tagName (e.g. "div")
        std::string id;                     // element.id if exists
        std::vector<std::string> classes;   // classList entries

        // DOM
        emscripten::val element;

        // Tree structure
        Node* parent = nullptr; // non-owning back pointer
        std::vector<std::unique_ptr<Node>> children;

        explicit Node(const std::string& uid_,
                      const emscripten::val& el_,
                      Node* parent_ = nullptr)
            : uid(uid_), element(el_), parent(parent_) {}
    };

private:
    // Member variables
    std::unique_ptr<Node> mRoot;                     //! Root element (#document)
    std::unordered_map<std::string, Node*> mUidDOM; //! All nodes keyed by uid
    unsigned long long mUidCounter = 1;              //! UID counter

    // Initialization
    void initFromDocument();
    void clearAll();

    /*
        DOM tree building
    */
    std::unique_ptr<Node> buildTreeFromDom(const emscripten::val& el, Node* parent);

    /*
        Utility
    */
    std::string makeUid() { return "el_" + std::to_string(mUidCounter++); }
    void hydrateMetaData(Node* n);
    void eraseSubtreeFromMap(Node* node);

    void requireValidNode(const Node* n) const
    {
        if (!n) throw std::runtime_error("WebLayout: null Node pointer");
    }

    static std::string toLower(std::string s)
    {
        std::transform(
            s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); }
        );
        return s;
    }

public:
    /*
        Constructor and Destructor
    */
    WebLayout() { initFromDocument(); }
    WebLayout(const WebLayout&) = delete;
    WebLayout(WebLayout&&) = delete;
    WebLayout& operator=(const WebLayout&) = delete;
    ~WebLayout() = default;

    /*
        Basic access
    */
    Node* getRoot() const { return mRoot.get(); }

    /*
        DOM helpers
    */
    Node* createElement(Node* parent,
                        const std::string& tag,
                        const std::string& id = "",
                        const std::vector<std::string>& classes = {});

    Node* findById(const std::string& id) const;
    void removeAllChildren(Node* parent);

    /*
        App shell
    */
    void createShell();
};

} // namespace cse498

#endif