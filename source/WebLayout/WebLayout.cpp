#include "WebLayout.hpp"

void WebLayout::initFromDocument()
{
    clearAll();

    emscripten::val doc = emscripten::val::global("document");
    if (doc.isNull() || doc.isUndefined()) 
    {
        throw std::runtime_error("WebLayout: document is not available");
    }

    // Root is "document" itself (not an element)
    mRoot = std::make_unique<Node>(makeUid(), doc, nullptr);
    mUidDOM[mRoot->uid] = mRoot.get();
    mRoot->tag = "#document";

    // Build children from documentElement (<html>) down
    emscripten::val html = doc["documentElement"];
    if (!html.isNull() && !html.isUndefined()) {
        auto htmlNode = buildTreeFromDom(html, mRoot.get());
        mRoot->children.emplace_back(std::move(htmlNode));
    }
}

void WebLayout::clearAll() 
{
    mUidDOM.clear();
    mRoot.reset();
    mUidCounter = 1;
}

unique_ptr<WebLayout::Node> WebLayout::buildTreeFromDom(const emscripten::val& el, Node* parent)
{
    auto uid = makeUid();
    auto node = std::make_unique<Node>(uid, el, parent);
    mUidDOM[uid] = node.get();

    hydrateMetaData(node.get());

    // Only build element children (ignore text/comment nodes)
    emscripten::val kids = el["children"];
    int len = kids["length"].as<int>();
    node->children.reserve(std::max(0, len));

    for (int i = 0; i < len; i++) 
    {
        emscripten::val childEl = kids[i];
        node->children.emplace_back(buildTreeFromDom(childEl, node.get()));
    }

    return node;
}

void WebLayout::hydrateMetaData(Node* n)
{
    // tagName
    if (n->element.hasOwnProperty("tagName")) 
    {
        n->tag = toLower(n->element["tagName"].as<string>());
    } else 
    {
        n->tag = "#document";
    }

    // id
    if (n->element.hasOwnProperty("id")) 
    {
        n->id = n->element["id"].as<string>();
    } else 
    {
        n->id.clear();
    }

    // class list
    n->classes.clear();
    if (n->element.hasOwnProperty("classList")) 
    {
        emscripten::val cl = n->element["classList"];
        int len = cl["length"].as<int>();
        n->classes.reserve(std::max(0, len));
        for (int i = 0; i < len; i++) 
        {
            n->classes.push_back(cl[i].as<string>());
        }
    }
}