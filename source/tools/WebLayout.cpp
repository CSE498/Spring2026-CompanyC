// WebLayout.cpp
//This class uses ChatGPT to help produce functionality, it is then reviewed.
#include "WebLayout.hpp"

namespace cse498
{

void WebLayout::initFromDocument()
{
    clearAll();

    emscripten::val doc = emscripten::val::global("document");
    if (doc.isNull() || doc.isUndefined())
    {
        throw std::runtime_error("WebLayout: document is not available");
    }

    // Root is the document itself
    mRoot = std::make_unique<Node>(makeUid(), doc, nullptr);
    mUidDOM[mRoot->uid] = mRoot.get();
    mRoot->tag = "#document";

    // Build children from documentElement (<html>) down
    emscripten::val html = doc["documentElement"];
    if (!html.isNull() && !html.isUndefined())
    {
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

std::unique_ptr<WebLayout::Node> WebLayout::buildTreeFromDom(const emscripten::val& el, Node* parent)
{
    auto uid = makeUid();
    auto node = std::make_unique<Node>(uid, el, parent);
    mUidDOM[uid] = node.get();

    hydrateMetaData(node.get());

    // Only build element children (ignore text/comment nodes)
    emscripten::val kids = el["children"];
    int len = kids["length"].as<int>();
    node->children.reserve(std::max(0, len));

    for (int i = 0; i < len; ++i)
    {
        emscripten::val childEl = kids[i];
        node->children.emplace_back(buildTreeFromDom(childEl, node.get()));
    }

    return node;
}

void WebLayout::hydrateMetaData(Node* n)
{
    requireValidNode(n);

    // tagName
    if (n->element.hasOwnProperty("tagName"))
    {
        n->tag = toLower(n->element["tagName"].as<std::string>());
    }
    else
    {
        n->tag = "#document";
    }

    // id
    if (n->element.hasOwnProperty("id"))
    {
        n->id = n->element["id"].as<std::string>();
    }
    else
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

        for (int i = 0; i < len; ++i)
        {
            n->classes.push_back(cl[i].as<std::string>());
        }
    }
}

void WebLayout::eraseSubtreeFromMap(Node* node)
{
    if (!node)
    {
        return;
    }

    for (auto& child : node->children)
    {
        eraseSubtreeFromMap(child.get());
    }

    mUidDOM.erase(node->uid);
}

WebLayout::Node* WebLayout::createElement(Node* parent,
                                          const std::string& tag,
                                          const std::string& id,
                                          const std::vector<std::string>& classes)
{
    requireValidNode(parent);

    emscripten::val doc = emscripten::val::global("document");
    if (doc.isNull() || doc.isUndefined())
    {
        throw std::runtime_error("WebLayout: document is not available");
    }

    emscripten::val el = doc.call<emscripten::val>("createElement", emscripten::val(tag));

    if (!id.empty())
    {
        el.set("id", id);
    }

    for (const auto& cls : classes)
    {
        el["classList"].call<void>("add", emscripten::val(cls));
    }

    // Attach to DOM first
    parent->element.call<void>("appendChild", el);

    auto uid = makeUid();
    auto node = std::make_unique<Node>(uid, el, parent);
    Node* raw = node.get();

    hydrateMetaData(raw);
    mUidDOM[uid] = raw;
    parent->children.emplace_back(std::move(node));

    return raw;
}

WebLayout::Node* WebLayout::findById(const std::string& id) const
{
    for (const auto& entry : mUidDOM)
    {
        Node* node = entry.second;
        if (node && node->id == id)
        {
            return node;
        }
    }
    return nullptr;
}

void WebLayout::removeAllChildren(Node* parent)
{
    requireValidNode(parent);

    for (auto& child : parent->children)
    {
        eraseSubtreeFromMap(child.get());
    }

    while (!parent->element["firstChild"].isNull() &&
           !parent->element["firstChild"].isUndefined())
    {
        emscripten::val firstChild = parent->element["firstChild"];
        parent->element.call<emscripten::val>("removeChild", firstChild);
    }

    parent->children.clear();
}

void WebLayout::createShell()
{
    emscripten::val doc = emscripten::val::global("document");
    if (doc.isNull() || doc.isUndefined())
    {
        throw std::runtime_error("WebLayout: document is not available");
    }

    emscripten::val body = doc["body"];
    if (body.isNull() || body.isUndefined())
    {
        throw std::runtime_error("WebLayout: document.body is not available");
    }

    // Clear visible page content
    body.set("innerHTML", std::string());

    // Reset internal tree and rebuild from body outward
    clearAll();

    mRoot = std::make_unique<Node>(makeUid(), doc, nullptr);
    mUidDOM[mRoot->uid] = mRoot.get();
    mRoot->tag = "#document";

    auto bodyNode = std::make_unique<Node>(makeUid(), body, mRoot.get());
    Node* bodyRaw = bodyNode.get();
    hydrateMetaData(bodyRaw);
    mUidDOM[bodyRaw->uid] = bodyRaw;
    mRoot->children.emplace_back(std::move(bodyNode));

    // Root shell
    Node* root = createElement(bodyRaw, "div", "cse498_root");
    Node* topbar = createElement(root, "div", "cse498_topbar");
    Node* main = createElement(root, "div", "cse498_main");

    // Main layout containers
    createElement(main, "div", "cse498_canvas_host");
    Node* sidebar = createElement(main, "div", "cse498_sidebar");

    // Topbar title
    Node* title = createElement(topbar, "span", "cse498_title");
    title->element.set("textContent", std::string("CSE 498"));

    // Mode / tick
    createElement(topbar, "span", "cse498_mode_tick");

    // World label + select
    Node* worldLabel = createElement(topbar, "label");
    worldLabel->element.set("htmlFor", std::string("cse498_world_select"));
    worldLabel->element.set("textContent", std::string("World:"));

    Node* worldSelect = createElement(topbar, "select", "cse498_world_select");
    // Dropdown options for the world select UI
    Node* stubWorldOption = createElement(worldSelect, "option");
    stubWorldOption->element.set("value", std::string("stub"));
    stubWorldOption->element.set("textContent", std::string("Stub World"));

    Node* mazeWorldOption = createElement(worldSelect, "option");
    mazeWorldOption->element.set("value", std::string("maze"));
    mazeWorldOption->element.set("textContent", std::string("Maze World"));

    Node* interactionWorldOption = createElement(worldSelect, "option");
    interactionWorldOption->element.set("value", std::string("interaction"));
    interactionWorldOption->element.set("textContent", std::string("Interaction World"));

    // Topbar buttons
    Node* startBtn = createElement(topbar, "button", "cse498btn-1");
    startBtn->element.set("textContent", std::string("Start"));

    Node* resetBtn = createElement(topbar, "button", "cse498btn-2");
    resetBtn->element.set("textContent", std::string("Reset"));

    Node* saveBtn = createElement(topbar, "button", "cse498btn-3");
    saveBtn->element.set("textContent", std::string("Save"));

    Node* loadBtn = createElement(topbar, "button", "cse498btn-4");
    loadBtn->element.set("textContent", std::string("Load"));

    
    // Sidebar containers
    createElement(sidebar, "div", "cse498_actions");
    createElement(sidebar, "div", "cse498_hud_host");

    Node* analysisLabel = createElement(sidebar, "div", "cse498_analysis_label");
    analysisLabel->element.set("innerHTML", std::string("<strong>Analyzed Data</strong>"));

    createElement(sidebar, "div", "cse498_analysis_host");
    createElement(sidebar, "div", "cse498_log_host");
}

} // namespace cse498
