// Used claude, chatgpt, gemini and some more LLMs for making tests.
// Added comments with claude 

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <climits>
#include <cfloat>

#include "../../source/tools/Serializer.hpp"

static int passed = 0;
static int failed = 0;
static cse498::Serializer s;

static void check(const std::string& label, bool ok) {
    if (ok) {
        ++passed;
        std::cout << "  PASS  " << label << "\n";
    } else {
        ++failed;
        std::cout << "  FAIL  " << label << "\n";
    }
}

// ================================================================
//  1. int — basic
// ================================================================
static void testInt() {
    std::cout << "\n--- int ---\n";

    std::string data = s.Serialize(42);
    std::cout << "  Serialize(42) = \"" << data << "\"\n";
    auto r = s.DeserializeInt(data);
    check("round-trip 42", r.has_value() && *r == 42);

    data = s.Serialize(-7);
    r = s.DeserializeInt(data);
    check("round-trip -7", r.has_value() && *r == -7);

    data = s.Serialize(0);
    r = s.DeserializeInt(data);
    check("round-trip 0", r.has_value() && *r == 0);

    check("reject double data", !s.DeserializeInt("d:3.14;").has_value());
    check("reject garbage", !s.DeserializeInt("xyz").has_value());
}

// ================================================================
//  2. double — basic
// ================================================================
static void testDouble() {
    std::cout << "\n--- double ---\n";

    std::string data = s.Serialize(3.14159265358979323);
    std::cout << "  Serialize(pi) = \"" << data << "\"\n";
    auto r = s.DeserializeDouble(data);
    check("round-trip pi", r.has_value() && *r == 3.14159265358979323);

    data = s.Serialize(-0.001);
    r = s.DeserializeDouble(data);
    check("round-trip -0.001", r.has_value() && std::abs(*r - (-0.001)) < 1e-15);

    data = s.Serialize(0.0);
    r = s.DeserializeDouble(data);
    check("round-trip 0.0", r.has_value() && *r == 0.0);

    check("reject int data", !s.DeserializeDouble("i:42;").has_value());
}

// ================================================================
//  3. bool — basic
// ================================================================
static void testBool() {
    std::cout << "\n--- bool ---\n";

    std::string data = s.Serialize(true);
    std::cout << "  Serialize(true) = \"" << data << "\"\n";
    auto r = s.DeserializeBool(data);
    check("round-trip true", r.has_value() && *r == true);

    data = s.Serialize(false);
    r = s.DeserializeBool(data);
    check("round-trip false", r.has_value() && *r == false);

    check("reject int data", !s.DeserializeBool("i:1;").has_value());
}

// ================================================================
//  4. char — basic
// ================================================================
static void testChar() {
    std::cout << "\n--- char ---\n";

    std::string data = s.Serialize('A');
    std::cout << "  Serialize('A') = \"" << data << "\"\n";
    auto r = s.DeserializeChar(data);
    check("round-trip 'A'", r.has_value() && *r == 'A');

    data = s.Serialize('z');
    r = s.DeserializeChar(data);
    check("round-trip 'z'", r.has_value() && *r == 'z');

    check("reject string data", !s.DeserializeChar("s:1:A;").has_value());
}

// ================================================================
//  5. std::string — basic
// ================================================================
static void testString() {
    std::cout << "\n--- string ---\n";

    std::string data = s.Serialize(std::string("hello"));
    std::cout << "  Serialize(\"hello\") = \"" << data << "\"\n";
    auto r = s.DeserializeString(data);
    check("round-trip \"hello\"", r.has_value() && *r == "hello");

    data = s.Serialize(std::string(""));
    r = s.DeserializeString(data);
    check("round-trip empty", r.has_value() && r->empty());

    std::string tricky = "key:val;foo:bar;";
    data = s.Serialize(tricky);
    std::cout << "  Serialize(\"" << tricky << "\") = \"" << data << "\"\n";
    r = s.DeserializeString(data);
    check("round-trip with ;: chars", r.has_value() && *r == tricky);

    check("reject int data", !s.DeserializeString("i:42;").has_value());
}

// ================================================================
//  6. std::vector<T> — basic
// ================================================================
static void testVector() {
    std::cout << "\n--- vector ---\n";

    std::vector<int> vi = {1, 2, 3};
    std::string data = s.Serialize(vi);
    std::cout << "  Serialize({1,2,3}) = \"" << data << "\"\n";
    auto ri = s.DeserializeVector<int>(data);
    check("round-trip vector<int>", ri.has_value() && *ri == vi);

    std::vector<int> empty;
    data = s.Serialize(empty);
    ri = s.DeserializeVector<int>(data);
    check("round-trip empty vector", ri.has_value() && ri->empty());

    std::vector<std::string> vs = {"hello", "world", "a;b"};
    data = s.Serialize(vs);
    std::cout << "  Serialize({\"hello\",\"world\",\"a;b\"}) = \"" << data << "\"\n";
    auto rs = s.DeserializeVector<std::string>(data);
    check("round-trip vector<string>", rs.has_value() && *rs == vs);

    std::vector<double> vd = {1.1, 2.2, 3.3};
    data = s.Serialize(vd);
    auto rd = s.DeserializeVector<double>(data);
    check("round-trip vector<double>", rd.has_value() && *rd == vd);

    check("reject int data", !s.DeserializeVector<int>("i:42;").has_value());
}

// ================================================================
//  7. std::map<K, V> — basic
// ================================================================
static void testMap() {
    std::cout << "\n--- map ---\n";

    std::map<std::string, int> msi = {{"alice", 1}, {"bob", 2}};
    std::string data = s.Serialize(msi);
    std::cout << "  Serialize(map) = \"" << data << "\"\n";
    auto rsi = s.DeserializeMap<std::string, int>(data);
    check("round-trip map<string,int>", rsi.has_value() && *rsi == msi);

    std::map<std::string, int> empty;
    data = s.Serialize(empty);
    rsi = s.DeserializeMap<std::string, int>(data);
    check("round-trip empty map", rsi.has_value() && rsi->empty());

    std::map<int, std::string> mis = {{1, "one"}, {2, "two"}};
    data = s.Serialize(mis);
    auto ris = s.DeserializeMap<int, std::string>(data);
    check("round-trip map<int,string>", ris.has_value() && *ris == mis);

    std::map<std::string, double> msd = {{"pi", 3.14}, {"e", 2.71}};
    data = s.Serialize(msd);
    auto rsd = s.DeserializeMap<std::string, double>(data);
    check("round-trip map<string,double>", rsd.has_value() && *rsd == msd);

    check("reject int data", !s.DeserializeMap<std::string, int>("i:42;").has_value());
}

// ================================================================
//  8. Nested containers — basic
// ================================================================
static void testNested() {
    std::cout << "\n--- nested containers ---\n";

    std::vector<std::vector<int>> vvi = {{1, 2}, {3, 4, 5}};
    std::string data = s.Serialize(vvi);
    std::cout << "  Serialize(vector<vector<int>>) = \"" << data << "\"\n";
    auto r = s.DeserializeVector<std::vector<int>>(data);
    check("round-trip vector<vector<int>>", r.has_value() && *r == vvi);

    std::map<std::string, std::vector<int>> msv;
    msv["odds"] = {1, 3, 5};
    msv["evens"] = {2, 4, 6};
    data = s.Serialize(msv);
    auto r2 = s.DeserializeMap<std::string, std::vector<int>>(data);
    check("round-trip map<string,vector<int>>", r2.has_value() && *r2 == msv);
}

// ================================================================
//  9. Custom type (Agent) — basic
// ================================================================
struct Agent {
    std::string name;
    double health;
    int level;
};

static void registerAgent() {
    s.RegisterType<Agent>("Agent",
        [](const Agent& a) -> std::string {
            return s.Serialize(a.name) + s.Serialize(a.health) + s.Serialize(a.level);
        },
        [](const std::string& data) -> std::optional<Agent> {
            Agent a;
            size_t pos = 0;
            auto name = s.DeserializeAt<std::string>(data, pos);
            if (!name) return std::nullopt;
            auto health = s.DeserializeAt<double>(data, pos);
            if (!health) return std::nullopt;
            auto level = s.DeserializeAt<int>(data, pos);
            if (!level) return std::nullopt;
            a.name = *name;
            a.health = *health;
            a.level = *level;
            return a;
        }
    );
}

static void testCustomType() {
    std::cout << "\n--- custom type (Agent) ---\n";

    registerAgent();

    check("IsTypeRegistered(\"Agent\")", s.IsTypeRegistered("Agent"));
    check("!IsTypeRegistered(\"Unknown\")", !s.IsTypeRegistered("Unknown"));

    Agent agent{"Steve", 100.0, 7};
    std::string data = s.Serialize<Agent>("Agent", agent);
    std::cout << "  Serialize(Agent{\"Steve\", 100.0, 7}) = \"" << data << "\"\n";

    check("format starts with custom:Agent:",
          data.substr(0, 13) == "custom:Agent:");

    auto restored = s.Deserialize<Agent>("Agent", data);
    check("round-trip Agent",
          restored.has_value() &&
          restored->name == "Steve" &&
          restored->health == 100.0 &&
          restored->level == 7);

    Agent agent2{"A;special:name", -0.5, 0};
    data = s.Serialize<Agent>("Agent", agent2);
    restored = s.Deserialize<Agent>("Agent", data);
    check("round-trip Agent with special chars",
          restored.has_value() &&
          restored->name == "A;special:name" &&
          std::abs(restored->health - (-0.5)) < 1e-15 &&
          restored->level == 0);

    Agent agent3{"", 0.0, -1};
    data = s.Serialize<Agent>("Agent", agent3);
    restored = s.Deserialize<Agent>("Agent", data);
    check("round-trip Agent with empty name",
          restored.has_value() &&
          restored->name.empty() &&
          restored->health == 0.0 &&
          restored->level == -1);

    check("reject unregistered type_id",
          !s.Deserialize<Agent>("Unknown", data).has_value());
    check("reject garbage data",
          !s.Deserialize<Agent>("Agent", "garbage").has_value());
    check("reject int data as Agent",
          !s.Deserialize<Agent>("Agent", "i:42;").has_value());
}

// ================================================================
//  10. int — edge cases
// ================================================================
static void testIntEdgeCases() {
    std::cout << "\n--- int edge cases ---\n";

    // Boundary values
    std::string data = s.Serialize(INT_MAX);
    auto r = s.DeserializeInt(data);
    check("round-trip INT_MAX", r.has_value() && *r == INT_MAX);

    data = s.Serialize(INT_MIN);
    r = s.DeserializeInt(data);
    check("round-trip INT_MIN", r.has_value() && *r == INT_MIN);

    data = s.Serialize(1);
    r = s.DeserializeInt(data);
    check("round-trip 1", r.has_value() && *r == 1);

    data = s.Serialize(-1);
    r = s.DeserializeInt(data);
    check("round-trip -1", r.has_value() && *r == -1);

    // Malformed inputs
    check("reject empty string",       !s.DeserializeInt("").has_value());
    check("reject just tag",           !s.DeserializeInt("i").has_value());
    check("reject just prefix",        !s.DeserializeInt("i:").has_value());
    check("reject missing semicolon",  !s.DeserializeInt("i:42").has_value());
    check("reject empty value",        !s.DeserializeInt("i:;").has_value());
    check("reject trailing junk",      !s.DeserializeInt("i:42abc;").has_value());
    check("reject float in int",       !s.DeserializeInt("i:3.14;").has_value());
    // Note: std::stoi accepts leading whitespace, so "i: 42;" parses as 42.
    // The serializer never produces spaces, but the parser is lenient here.
    auto rSpace = s.DeserializeInt("i: 42;");
    check("accept leading space (stoi is lenient)", rSpace.has_value() && *rSpace == 42);
    check("reject bool data",          !s.DeserializeInt("b:1;").has_value());
    check("reject char data",          !s.DeserializeInt("c:A;").has_value());
    check("reject string data",        !s.DeserializeInt("s:2:42;").has_value());
    check("reject vector data",        !s.DeserializeInt("v:1:i:1;").has_value());
    check("reject map data",           !s.DeserializeInt("m:0:").has_value());

    // Overflow — value larger than INT_MAX
    check("reject overflow",
          !s.DeserializeInt("i:99999999999999999999;").has_value());
}

// ================================================================
//  11. double — edge cases
// ================================================================
static void testDoubleEdgeCases() {
    std::cout << "\n--- double edge cases ---\n";

    // Extreme values
    std::string data = s.Serialize(1e308);
    auto r = s.DeserializeDouble(data);
    check("round-trip 1e308", r.has_value() && *r == 1e308);

    data = s.Serialize(-1e308);
    r = s.DeserializeDouble(data);
    check("round-trip -1e308", r.has_value() && *r == -1e308);

    // 1e-308 is a subnormal double (below DBL_MIN) — reduced precision
    // means exact equality may fail; use relative tolerance instead.
    data = s.Serialize(1e-308);
    r = s.DeserializeDouble(data);
    check("round-trip 1e-308 (subnormal)",
          r.has_value() && std::abs(*r - 1e-308) / 1e-308 < 1e-10);

    data = s.Serialize(DBL_MIN);
    r = s.DeserializeDouble(data);
    check("round-trip DBL_MIN", r.has_value() && *r == DBL_MIN);

    data = s.Serialize(DBL_MAX);
    r = s.DeserializeDouble(data);
    check("round-trip DBL_MAX", r.has_value() && *r == DBL_MAX);

    // Negative zero
    data = s.Serialize(-0.0);
    r = s.DeserializeDouble(data);
    check("round-trip -0.0", r.has_value() && *r == 0.0);

    // Infinity
    data = s.Serialize(std::numeric_limits<double>::infinity());
    r = s.DeserializeDouble(data);
    check("round-trip +inf", r.has_value() && std::isinf(*r) && *r > 0);

    data = s.Serialize(-std::numeric_limits<double>::infinity());
    r = s.DeserializeDouble(data);
    check("round-trip -inf", r.has_value() && std::isinf(*r) && *r < 0);

    // NaN — NaN != NaN, so check with std::isnan
    data = s.Serialize(std::numeric_limits<double>::quiet_NaN());
    r = s.DeserializeDouble(data);
    check("round-trip NaN", r.has_value() && std::isnan(*r));

    // Malformed inputs
    check("reject empty string",      !s.DeserializeDouble("").has_value());
    check("reject just tag",          !s.DeserializeDouble("d").has_value());
    check("reject just prefix",       !s.DeserializeDouble("d:").has_value());
    check("reject missing semicolon", !s.DeserializeDouble("d:3.14").has_value());
    check("reject empty value",       !s.DeserializeDouble("d:;").has_value());
    check("reject non-numeric",       !s.DeserializeDouble("d:abc;").has_value());
    check("reject bool data",         !s.DeserializeDouble("b:1;").has_value());
    check("reject char data",         !s.DeserializeDouble("c:A;").has_value());
    check("reject string data",       !s.DeserializeDouble("s:3:3.1;").has_value());
}

// ================================================================
//  12. bool — edge cases
// ================================================================
static void testBoolEdgeCases() {
    std::cout << "\n--- bool edge cases ---\n";

    // Invalid digit values
    check("reject b:2;",              !s.DeserializeBool("b:2;").has_value());
    check("reject b:9;",              !s.DeserializeBool("b:9;").has_value());
    check("reject b:a;",              !s.DeserializeBool("b:a;").has_value());
    check("reject b:T;",              !s.DeserializeBool("b:T;").has_value());

    // Truncated / malformed
    check("reject empty string",      !s.DeserializeBool("").has_value());
    check("reject just tag",          !s.DeserializeBool("b").has_value());
    check("reject just prefix",       !s.DeserializeBool("b:").has_value());
    check("reject no semicolon",      !s.DeserializeBool("b:1").has_value());
    check("reject b:1x (bad semi)",   !s.DeserializeBool("b:1x").has_value());

    // Wrong type
    check("reject double data",       !s.DeserializeBool("d:1.0;").has_value());
    check("reject char data",         !s.DeserializeBool("c:1;").has_value());
    check("reject string data",       !s.DeserializeBool("s:1:1;").has_value());
}

// ================================================================
//  13. char — edge cases
// ================================================================
static void testCharEdgeCases() {
    std::cout << "\n--- char edge cases ---\n";

    // Special character values
    std::string data = s.Serialize(' ');
    auto r = s.DeserializeChar(data);
    check("round-trip space", r.has_value() && *r == ' ');

    data = s.Serialize('0');
    r = s.DeserializeChar(data);
    check("round-trip digit '0'", r.has_value() && *r == '0');

    data = s.Serialize('9');
    r = s.DeserializeChar(data);
    check("round-trip digit '9'", r.has_value() && *r == '9');

    // Delimiter characters as values
    data = s.Serialize(';');
    r = s.DeserializeChar(data);
    check("round-trip semicolon", r.has_value() && *r == ';');

    data = s.Serialize(':');
    r = s.DeserializeChar(data);
    check("round-trip colon", r.has_value() && *r == ':');

    data = s.Serialize('\t');
    r = s.DeserializeChar(data);
    check("round-trip tab", r.has_value() && *r == '\t');

    data = s.Serialize('\n');
    r = s.DeserializeChar(data);
    check("round-trip newline", r.has_value() && *r == '\n');

    // Truncated / malformed
    check("reject empty string",      !s.DeserializeChar("").has_value());
    check("reject just tag",          !s.DeserializeChar("c").has_value());
    check("reject just prefix",       !s.DeserializeChar("c:").has_value());
    check("reject missing semicolon", !s.DeserializeChar("c:A").has_value());

    // Wrong type
    check("reject int data",          !s.DeserializeChar("i:65;").has_value());
    check("reject bool data",         !s.DeserializeChar("b:0;").has_value());
}

// ================================================================
//  14. string — edge cases
// ================================================================
static void testStringEdgeCases() {
    std::cout << "\n--- string edge cases ---\n";

    // Whitespace content
    std::string data = s.Serialize(std::string("\t\n\r"));
    auto r = s.DeserializeString(data);
    check("round-trip whitespace chars", r.has_value() && *r == "\t\n\r");

    // String that looks like other serialized types
    data = s.Serialize(std::string("i:42;"));
    r = s.DeserializeString(data);
    check("round-trip string containing i:42;", r.has_value() && *r == "i:42;");

    data = s.Serialize(std::string("b:1;"));
    r = s.DeserializeString(data);
    check("round-trip string containing b:1;", r.has_value() && *r == "b:1;");

    // Long string
    std::string longStr(1000, 'x');
    data = s.Serialize(longStr);
    r = s.DeserializeString(data);
    check("round-trip 1000-char string", r.has_value() && *r == longStr);

    // String of only delimiters
    std::string delims = ":::;;;:::;;;";
    data = s.Serialize(delims);
    r = s.DeserializeString(data);
    check("round-trip delimiter-only string", r.has_value() && *r == delims);

    // Single character string (not char type)
    data = s.Serialize(std::string("X"));
    r = s.DeserializeString(data);
    check("round-trip single-char string", r.has_value() && *r == "X");

    // Malformed inputs
    check("reject empty string",              !s.DeserializeString("").has_value());
    check("reject just tag",                  !s.DeserializeString("s").has_value());
    check("reject just prefix",              !s.DeserializeString("s:").has_value());
    check("reject missing length colon",     !s.DeserializeString("s:5hello;").has_value());
    check("reject length too large",         !s.DeserializeString("s:100:hi;").has_value());
    check("reject length too small",         !s.DeserializeString("s:1:hello;").has_value());
    check("reject missing trailing semi",    !s.DeserializeString("s:5:hello").has_value());
    check("reject negative-looking length",  !s.DeserializeString("s:-1:x;").has_value());

    // Wrong type
    check("reject bool data",                !s.DeserializeString("b:0;").has_value());
    check("reject char data",                !s.DeserializeString("c:h;").has_value());
}

// ================================================================
//  15. vector — edge cases
// ================================================================
static void testVectorEdgeCases() {
    std::cout << "\n--- vector edge cases ---\n";

    // Single element
    std::vector<int> single = {42};
    std::string data = s.Serialize(single);
    auto ri = s.DeserializeVector<int>(data);
    check("round-trip single-element vector", ri.has_value() && *ri == single);

    // Vector of bools
    std::vector<bool> vb = {true, false, true, true, false};
    // Serialize manually since vector<bool> is special in C++
    std::string boolData = "v:" + std::to_string(vb.size()) + ":";
    for (bool b : vb) boolData += s.Serialize(b);
    auto rb = s.DeserializeVector<bool>(boolData);
    check("round-trip vector<bool>",
          rb.has_value() && rb->size() == 5 &&
          (*rb)[0] == true && (*rb)[1] == false && (*rb)[2] == true &&
          (*rb)[3] == true && (*rb)[4] == false);

    // Vector of chars
    std::vector<char> vc = {'a', 'b', 'c'};
    data = s.Serialize(vc);
    auto rc = s.DeserializeVector<char>(data);
    check("round-trip vector<char>", rc.has_value() && *rc == vc);

    // Malformed: count exceeds actual elements
    check("reject count too high",
          !s.DeserializeVector<int>("v:5:i:1;i:2;").has_value());

    // Malformed: no colon after count
    check("reject missing count colon",
          !s.DeserializeVector<int>("v:3").has_value());

    // Malformed: non-numeric count
    check("reject non-numeric count",
          !s.DeserializeVector<int>("v:abc:i:1;").has_value());

    // Malformed: empty string
    check("reject empty string",
          !s.DeserializeVector<int>("").has_value());

    // Malformed: just prefix
    check("reject just prefix",
          !s.DeserializeVector<int>("v:").has_value());

    // Malformed: element type mismatch (claims int, contains string elements)
    check("reject type mismatch in elements",
          !s.DeserializeVector<int>("v:2:s:2:hi;s:2:lo;").has_value());

    // Large vector
    std::vector<int> large;
    for (int i = 0; i < 100; ++i) large.push_back(i);
    data = s.Serialize(large);
    ri = s.DeserializeVector<int>(data);
    check("round-trip 100-element vector", ri.has_value() && *ri == large);
}

// ================================================================
//  16. map — edge cases
// ================================================================
static void testMapEdgeCases() {
    std::cout << "\n--- map edge cases ---\n";

    // Single entry
    std::map<std::string, int> single = {{"only", 1}};
    std::string data = s.Serialize(single);
    auto r = s.DeserializeMap<std::string, int>(data);
    check("round-trip single-entry map", r.has_value() && *r == single);

    // Map<int, int>
    std::map<int, int> mii = {{1, 10}, {2, 20}, {3, 30}};
    data = s.Serialize(mii);
    auto rii = s.DeserializeMap<int, int>(data);
    check("round-trip map<int,int>", rii.has_value() && *rii == mii);

    // Map<string, bool>
    std::map<std::string, bool> msb = {{"active", true}, {"deleted", false}};
    data = s.Serialize(msb);
    auto rsb = s.DeserializeMap<std::string, bool>(data);
    check("round-trip map<string,bool>", rsb.has_value() && *rsb == msb);

    // Map<char, string>
    std::map<char, std::string> mcs = {{'a', "alpha"}, {'b', "bravo"}};
    data = s.Serialize(mcs);
    auto rcs = s.DeserializeMap<char, std::string>(data);
    check("round-trip map<char,string>", rcs.has_value() && *rcs == mcs);

    // Map with delimiter chars in keys
    std::map<std::string, int> tricky = {{"key;with:delims", 99}};
    data = s.Serialize(tricky);
    auto rt = s.DeserializeMap<std::string, int>(data);
    check("round-trip map with delimiters in key", rt.has_value() && *rt == tricky);

    // Malformed: count too high
    check("reject count too high",
          !s.DeserializeMap<std::string, int>("m:5:s:1:a;i:1;").has_value());

    // Malformed: missing count colon
    check("reject missing count colon",
          !s.DeserializeMap<std::string, int>("m:2").has_value());

    // Malformed: empty string
    check("reject empty string",
          !s.DeserializeMap<std::string, int>("").has_value());

    // Malformed: just prefix
    check("reject just prefix",
          !s.DeserializeMap<std::string, int>("m:").has_value());

    // Malformed: value missing (key present but no value)
    check("reject missing value",
          !s.DeserializeMap<std::string, int>("m:1:s:3:key;").has_value());
}

// ================================================================
//  17. nested containers — edge cases
// ================================================================
static void testNestedEdgeCases() {
    std::cout << "\n--- nested edge cases ---\n";

    // 3 levels deep: vector<vector<vector<int>>>
    std::vector<std::vector<std::vector<int>>> deep = {
        {{1, 2}, {3}},
        {{4, 5, 6}}
    };
    std::string data = s.Serialize(deep);
    auto r = s.DeserializeVector<std::vector<std::vector<int>>>(data);
    check("round-trip 3-level nested vector", r.has_value() && *r == deep);

    // Map of maps
    std::map<std::string, std::map<std::string, int>> mom;
    mom["team1"] = {{"alice", 10}, {"bob", 20}};
    mom["team2"] = {{"carol", 30}};
    data = s.Serialize(mom);
    auto r2 = s.DeserializeMap<std::string, std::map<std::string, int>>(data);
    check("round-trip map<string,map<string,int>>", r2.has_value() && *r2 == mom);

    // Vector of maps
    std::vector<std::map<std::string, int>> vom;
    vom.push_back({{"x", 1}, {"y", 2}});
    vom.push_back({{"z", 3}});
    data = s.Serialize(vom);
    auto r3 = s.DeserializeVector<std::map<std::string, int>>(data);
    check("round-trip vector<map<string,int>>", r3.has_value() && *r3 == vom);

    // Map of vectors of strings
    std::map<int, std::vector<std::string>> mivs;
    mivs[1] = {"hello", "world"};
    mivs[2] = {"foo;bar", "baz:qux"};
    data = s.Serialize(mivs);
    auto r4 = s.DeserializeMap<int, std::vector<std::string>>(data);
    check("round-trip map<int,vector<string>> with delims", r4.has_value() && *r4 == mivs);

    // Empty nested containers
    std::vector<std::vector<int>> emptyNested = {{}, {}, {}};
    data = s.Serialize(emptyNested);
    auto r5 = s.DeserializeVector<std::vector<int>>(data);
    check("round-trip vector of empty vectors", r5.has_value() && *r5 == emptyNested);

    // Map with empty vector values
    std::map<std::string, std::vector<int>> emptyVals;
    emptyVals["a"] = {};
    emptyVals["b"] = {1};
    data = s.Serialize(emptyVals);
    auto r6 = s.DeserializeMap<std::string, std::vector<int>>(data);
    check("round-trip map with empty vector values", r6.has_value() && *r6 == emptyVals);
}

// ================================================================
//  18. custom type — edge cases
// ================================================================
struct Inventory {
    std::string item;
    std::vector<int> quantities;
};

static void testCustomTypeEdgeCases() {
    std::cout << "\n--- custom type edge cases ---\n";

    // --- Re-registration overwrites previous entry ---
    // Register a dummy that always fails, then re-register properly
    s.RegisterType<Agent>("AgentOverwrite",
        [](const Agent&) -> std::string { return ""; },
        [](const std::string&) -> std::optional<Agent> { return std::nullopt; }
    );
    // Overwrite with a working implementation
    s.RegisterType<Agent>("AgentOverwrite",
        [](const Agent& a) -> std::string {
            return s.Serialize(a.name) + s.Serialize(a.health) + s.Serialize(a.level);
        },
        [](const std::string& data) -> std::optional<Agent> {
            Agent a;
            size_t pos = 0;
            auto name = s.DeserializeAt<std::string>(data, pos);
            auto health = s.DeserializeAt<double>(data, pos);
            auto level = s.DeserializeAt<int>(data, pos);
            if (!name || !health || !level) return std::nullopt;
            a.name = *name; a.health = *health; a.level = *level;
            return a;
        }
    );
    Agent testAgent{"Overwrite", 50.0, 3};
    auto overData = s.Serialize<Agent>("AgentOverwrite", testAgent);
    auto overRes = s.Deserialize<Agent>("AgentOverwrite", overData);
    check("re-registration overwrites previous",
          overRes.has_value() && overRes->name == "Overwrite");

    // --- Serialize with unregistered type_id returns "" ---
    Agent dummy{"X", 0.0, 0};
    check("serialize unregistered returns empty",
          s.Serialize<Agent>("NoSuchType", dummy).empty());

    // --- Tampered content length (too short) ---
    std::string good = s.Serialize<Agent>("Agent", Agent{"A", 1.0, 1});
    // Find the length field and reduce it
    std::string tampered = "custom:Agent:5:s:5:Steve;d:100;i:7;;";
    check("reject tampered length (too short)",
          !s.Deserialize<Agent>("Agent", tampered).has_value());

    // --- Tampered content length (too long) ---
    std::string tampered2 = "custom:Agent:9999:s:5:Steve;d:100;i:7;;";
    check("reject tampered length (too long)",
          !s.Deserialize<Agent>("Agent", tampered2).has_value());

    // --- Missing trailing semicolon ---
    // Take valid data and strip the last character
    std::string valid = s.Serialize<Agent>("Agent", Agent{"Bob", 50.0, 2});
    std::string noSemi = valid.substr(0, valid.size() - 1);
    check("reject missing trailing semicolon",
          !s.Deserialize<Agent>("Agent", noSemi).has_value());

    // --- Empty inner content ---
    check("reject empty inner content",
          !s.Deserialize<Agent>("Agent", "custom:Agent:0:;").has_value());

    // --- Wrong type_id in prefix vs argument ---
    std::string agentData = s.Serialize<Agent>("Agent", Agent{"Mix", 1.0, 1});
    check("reject type_id mismatch (Agent data to AgentOverwrite)",
          !s.Deserialize<Agent>("AgentOverwrite", agentData).has_value());

    // --- Prefix present but content is corrupted ---
    check("reject corrupted inner content",
          !s.Deserialize<Agent>("Agent", "custom:Agent:7:garbage;").has_value());

    // --- Non-numeric content length ---
    check("reject non-numeric length",
          !s.Deserialize<Agent>("Agent", "custom:Agent:abc:stuff;").has_value());

    // --- Separate Serializer instances have independent registries ---
    cse498::Serializer s2;
    check("separate instance: Agent not registered",
          !s2.IsTypeRegistered("Agent"));
    check("separate instance: deserialize fails",
          !s2.Deserialize<Agent>("Agent", agentData).has_value());

    // --- Custom type with container fields ---
    s.RegisterType<Inventory>("Inventory",
        [](const Inventory& inv) -> std::string {
            return s.Serialize(inv.item) + s.Serialize(inv.quantities);
        },
        [](const std::string& data) -> std::optional<Inventory> {
            Inventory inv;
            size_t pos = 0;
            auto item = s.DeserializeAt<std::string>(data, pos);
            if (!item) return std::nullopt;
            auto qty = s.DeserializeAt<std::vector<int>>(data, pos);
            if (!qty) return std::nullopt;
            inv.item = *item;
            inv.quantities = *qty;
            return inv;
        }
    );

    Inventory inv{"Sword", {1, 5, 10}};
    auto invData = s.Serialize<Inventory>("Inventory", inv);
    auto invRes = s.Deserialize<Inventory>("Inventory", invData);
    check("custom type with vector field",
          invRes.has_value() &&
          invRes->item == "Sword" &&
          invRes->quantities == std::vector<int>({1, 5, 10}));

    // Empty vector in custom type
    Inventory emptyInv{"Nothing", {}};
    invData = s.Serialize<Inventory>("Inventory", emptyInv);
    invRes = s.Deserialize<Inventory>("Inventory", invData);
    check("custom type with empty vector field",
          invRes.has_value() &&
          invRes->item == "Nothing" &&
          invRes->quantities.empty());

    // --- Multiple custom types coexist ---
    check("Agent still registered after Inventory", s.IsTypeRegistered("Agent"));
    check("Inventory registered", s.IsTypeRegistered("Inventory"));
    // Use both in sequence
    Agent a1{"Multi", 99.0, 5};
    Inventory i1{"Shield", {2, 3}};
    auto ad = s.Serialize<Agent>("Agent", a1);
    auto id = s.Serialize<Inventory>("Inventory", i1);
    auto ar = s.Deserialize<Agent>("Agent", ad);
    auto ir = s.Deserialize<Inventory>("Inventory", id);
    check("multiple types: Agent round-trip",
          ar.has_value() && ar->name == "Multi");
    check("multiple types: Inventory round-trip",
          ir.has_value() && ir->item == "Shield" &&
          ir->quantities == std::vector<int>({2, 3}));

    // --- Cross-deserialize: Inventory data to Agent ---
    check("reject Inventory data as Agent",
          !s.Deserialize<Agent>("Agent", id).has_value());
}

// ================================================================
//  19. DeserializeAt — positional parsing edge cases
// ================================================================
static void testDeserializeAtEdgeCases() {
    std::cout << "\n--- DeserializeAt edge cases ---\n";

    // Sequential parsing of mixed types in a single string
    std::string combined = s.Serialize(42) + s.Serialize(std::string("hi")) +
                           s.Serialize(true) + s.Serialize('Z') + s.Serialize(3.14);
    size_t pos = 0;

    auto ri = s.DeserializeAt<int>(combined, pos);
    check("positional: int from combined", ri.has_value() && *ri == 42);

    auto rs = s.DeserializeAt<std::string>(combined, pos);
    check("positional: string from combined", rs.has_value() && *rs == "hi");

    auto rb = s.DeserializeAt<bool>(combined, pos);
    check("positional: bool from combined", rb.has_value() && *rb == true);

    auto rc = s.DeserializeAt<char>(combined, pos);
    check("positional: char from combined", rc.has_value() && *rc == 'Z');

    auto rd = s.DeserializeAt<double>(combined, pos);
    check("positional: double from combined", rd.has_value());

    // pos should now be at the end
    check("positional: pos at end", pos == combined.size());

    // Reading past end returns nullopt
    auto pastEnd = s.DeserializeAt<int>(combined, pos);
    check("positional: past-end returns nullopt", !pastEnd.has_value());

    // Positional with vector
    std::string vecStr = s.Serialize(std::vector<int>{10, 20}) + s.Serialize(99);
    pos = 0;
    auto rv = s.DeserializeAt<std::vector<int>>(vecStr, pos);
    check("positional: vector then continue",
          rv.has_value() && *rv == std::vector<int>({10, 20}));
    auto rAfter = s.DeserializeAt<int>(vecStr, pos);
    check("positional: int after vector", rAfter.has_value() && *rAfter == 99);

    // Positional with map
    std::map<int, int> m = {{1, 2}};
    std::string mapStr = s.Serialize(m) + s.Serialize(std::string("end"));
    pos = 0;
    auto rm = s.DeserializeAt<std::map<int, int>>(mapStr, pos);
    check("positional: map then continue", rm.has_value() && *rm == m);
    auto rEnd = s.DeserializeAt<std::string>(mapStr, pos);
    check("positional: string after map", rEnd.has_value() && *rEnd == "end");

    // Wrong type at position
    std::string intData = s.Serialize(42);
    pos = 0;
    check("positional: wrong type at pos",
          !s.DeserializeAt<std::string>(intData, pos).has_value());
}

// ================================================================
//  20. Cross-type rejection — systematic
// ================================================================
static void testCrossTypeRejection() {
    std::cout << "\n--- cross-type rejection ---\n";

    std::string intData    = s.Serialize(42);
    std::string dblData    = s.Serialize(3.14);
    std::string boolData   = s.Serialize(true);
    std::string charData   = s.Serialize('A');
    std::string strData    = s.Serialize(std::string("hi"));
    std::string vecData    = s.Serialize(std::vector<int>{1});
    std::string mapData    = s.Serialize(std::map<std::string, int>{{"k", 1}});

    // int rejects all others
    check("int rejects double",  !s.DeserializeInt(dblData).has_value());
    check("int rejects bool",    !s.DeserializeInt(boolData).has_value());
    check("int rejects char",    !s.DeserializeInt(charData).has_value());
    check("int rejects string",  !s.DeserializeInt(strData).has_value());
    check("int rejects vector",  !s.DeserializeInt(vecData).has_value());
    check("int rejects map",     !s.DeserializeInt(mapData).has_value());

    // double rejects all others
    check("double rejects int",    !s.DeserializeDouble(intData).has_value());
    check("double rejects bool",   !s.DeserializeDouble(boolData).has_value());
    check("double rejects char",   !s.DeserializeDouble(charData).has_value());
    check("double rejects string", !s.DeserializeDouble(strData).has_value());
    check("double rejects vector", !s.DeserializeDouble(vecData).has_value());
    check("double rejects map",    !s.DeserializeDouble(mapData).has_value());

    // bool rejects all others
    check("bool rejects int",    !s.DeserializeBool(intData).has_value());
    check("bool rejects double", !s.DeserializeBool(dblData).has_value());
    check("bool rejects char",   !s.DeserializeBool(charData).has_value());
    check("bool rejects string", !s.DeserializeBool(strData).has_value());
    check("bool rejects vector", !s.DeserializeBool(vecData).has_value());
    check("bool rejects map",    !s.DeserializeBool(mapData).has_value());

    // char rejects all others
    check("char rejects int",    !s.DeserializeChar(intData).has_value());
    check("char rejects double", !s.DeserializeChar(dblData).has_value());
    check("char rejects bool",   !s.DeserializeChar(boolData).has_value());
    check("char rejects string", !s.DeserializeChar(strData).has_value());
    check("char rejects vector", !s.DeserializeChar(vecData).has_value());
    check("char rejects map",    !s.DeserializeChar(mapData).has_value());

    // string rejects all others
    check("string rejects int",    !s.DeserializeString(intData).has_value());
    check("string rejects double", !s.DeserializeString(dblData).has_value());
    check("string rejects bool",   !s.DeserializeString(boolData).has_value());
    check("string rejects char",   !s.DeserializeString(charData).has_value());
    check("string rejects vector", !s.DeserializeString(vecData).has_value());
    check("string rejects map",    !s.DeserializeString(mapData).has_value());

    // vector rejects all non-vector
    check("vector rejects int",    !s.DeserializeVector<int>(intData).has_value());
    check("vector rejects double", !s.DeserializeVector<int>(dblData).has_value());
    check("vector rejects bool",   !s.DeserializeVector<int>(boolData).has_value());
    check("vector rejects char",   !s.DeserializeVector<int>(charData).has_value());
    check("vector rejects string", !s.DeserializeVector<int>(strData).has_value());
    check("vector rejects map",    !s.DeserializeVector<int>(mapData).has_value());

    // map rejects all non-map
    check("map rejects int",    !s.DeserializeMap<std::string,int>(intData).has_value());
    check("map rejects double", !s.DeserializeMap<std::string,int>(dblData).has_value());
    check("map rejects bool",   !s.DeserializeMap<std::string,int>(boolData).has_value());
    check("map rejects char",   !s.DeserializeMap<std::string,int>(charData).has_value());
    check("map rejects string", !s.DeserializeMap<std::string,int>(strData).has_value());
    check("map rejects vector", !s.DeserializeMap<std::string,int>(vecData).has_value());
}

// ================================================================
//  21. strtod quirks — documents parser leniency
// ================================================================
static void testStrtodQuirks() {
    std::cout << "\n--- strtod quirks ---\n";

    // strtod accepts hex floats like 0x1.0p0 = 1.0
    auto r = s.DeserializeDouble("d:0x1.0p0;");
    check("strtod accepts hex float 0x1.0p0", r.has_value() && *r == 1.0);

    // strtod accepts leading whitespace
    r = s.DeserializeDouble("d: 3.14;");
    check("strtod accepts leading space", r.has_value() && std::abs(*r - 3.14) < 1e-12);

    // strtod accepts leading '+'
    r = s.DeserializeDouble("d:+2.5;");
    check("strtod accepts leading +", r.has_value() && *r == 2.5);

    // strtod accepts "inf"
    r = s.DeserializeDouble("d:inf;");
    check("strtod accepts inf", r.has_value() && std::isinf(*r) && *r > 0);

    // strtod accepts "nan"
    r = s.DeserializeDouble("d:nan;");
    check("strtod accepts nan", r.has_value() && std::isnan(*r));

    // strtod rejects empty value
    r = s.DeserializeDouble("d:;");
    check("strtod rejects empty value", !r.has_value());

    // strtod rejects pure text
    r = s.DeserializeDouble("d:hello;");
    check("strtod rejects pure text", !r.has_value());
}

// ================================================================
//  22. stoi quirks — documents parser leniency
// ================================================================
static void testStoiQuirks() {
    std::cout << "\n--- stoi quirks ---\n";

    // stoi accepts leading zeros
    auto r = s.DeserializeInt("i:007;");
    check("stoi accepts leading zeros", r.has_value() && *r == 7);

    // stoi accepts leading +
    r = s.DeserializeInt("i:+42;");
    check("stoi accepts leading +", r.has_value() && *r == 42);

    // stoi accepts negative zero
    r = s.DeserializeInt("i:-0;");
    check("stoi accepts -0", r.has_value() && *r == 0);

    // stoi rejects hex (0x prefix: stoi parses "0" then stops at "x")
    r = s.DeserializeInt("i:0xFF;");
    check("stoi rejects hex 0xFF", !r.has_value());

    // stoi rejects empty
    r = s.DeserializeInt("i:;");
    check("stoi rejects empty value", !r.has_value());

    // stoi rejects whitespace-only
    r = s.DeserializeInt("i: ;");
    check("stoi rejects whitespace-only", !r.has_value());
}

// ================================================================
//  23. trailing data ignored — documents that public API ignores trailing data
// ================================================================
static void testTrailingDataIgnored() {
    std::cout << "\n--- trailing data ignored ---\n";

    // Each public Deserialize* function parses one element and ignores the rest
    auto ri = s.DeserializeInt("i:42;JUNK");
    check("DeserializeInt ignores trailing data", ri.has_value() && *ri == 42);

    auto rd = s.DeserializeDouble("d:3.14;JUNK");
    check("DeserializeDouble ignores trailing data", rd.has_value() && std::abs(*rd - 3.14) < 1e-12);

    auto rb = s.DeserializeBool("b:1;JUNK");
    check("DeserializeBool ignores trailing data", rb.has_value() && *rb == true);

    auto rc = s.DeserializeChar("c:A;JUNK");
    check("DeserializeChar ignores trailing data", rc.has_value() && *rc == 'A');

    auto rs = s.DeserializeString("s:2:hi;JUNK");
    check("DeserializeString ignores trailing data", rs.has_value() && *rs == "hi");

    auto rv = s.DeserializeVector<int>("v:1:i:1;JUNK");
    check("DeserializeVector ignores trailing data", rv.has_value() && rv->size() == 1 && (*rv)[0] == 1);

    auto rm = s.DeserializeMap<std::string, int>("m:1:s:1:k;i:1;JUNK");
    check("DeserializeMap ignores trailing data", rm.has_value() && rm->size() == 1);

    // Also for custom types
    Agent a{"Test", 1.0, 1};
    std::string agentData = s.Serialize<Agent>("Agent", a);
    // Append junk after the trailing semicolon
    std::string withJunk = agentData + "JUNK";
    auto ra = s.Deserialize<Agent>("Agent", withJunk);
    check("Deserialize<Agent> ignores trailing data", ra.has_value() && ra->name == "Test");
}

// ================================================================
//  24. vector<bool> direct serialization
// ================================================================
static void testVectorBoolDirect() {
    std::cout << "\n--- vector<bool> direct ---\n";

    std::vector<bool> vb = {true, false, true, true, false};
    std::string data = s.Serialize(vb);
    auto r = s.DeserializeVector<bool>(data);
    check("direct vector<bool> round-trip",
          r.has_value() && r->size() == 5 &&
          (*r)[0] == true && (*r)[1] == false && (*r)[2] == true &&
          (*r)[3] == true && (*r)[4] == false);

    std::vector<bool> empty;
    data = s.Serialize(empty);
    r = s.DeserializeVector<bool>(data);
    check("direct empty vector<bool>", r.has_value() && r->empty());

    std::vector<bool> single = {true};
    data = s.Serialize(single);
    r = s.DeserializeVector<bool>(data);
    check("direct single vector<bool>", r.has_value() && r->size() == 1 && (*r)[0] == true);

    std::vector<bool> allFalse = {false, false, false};
    data = s.Serialize(allFalse);
    r = s.DeserializeVector<bool>(data);
    check("direct all-false vector<bool>", r.has_value() && r->size() == 3 &&
          (*r)[0] == false && (*r)[1] == false && (*r)[2] == false);
}

// ================================================================
//  25. const char* overload routes to string
// ================================================================
static void testConstCharOverload() {
    std::cout << "\n--- const char* overload ---\n";

    std::string data = s.Serialize("hello");
    check("Serialize(\"hello\") starts with s:", data.substr(0, 2) == "s:");

    auto r = s.DeserializeString(data);
    check("const char* round-trips as string", r.has_value() && *r == "hello");

    // Empty const char*
    data = s.Serialize("");
    r = s.DeserializeString(data);
    check("Serialize(\"\") round-trips", r.has_value() && r->empty());

    // const char* with delimiters
    data = s.Serialize("a:b;c");
    r = s.DeserializeString(data);
    check("const char* with delimiters", r.has_value() && *r == "a:b;c");

    // Ensure it does NOT serialize as bool
    data = s.Serialize("true");
    check("Serialize(\"true\") is not bool", data.substr(0, 2) == "s:");
}

// ================================================================
//  26. serialize->deserialize->serialize idempotency
// ================================================================
static void testSerializeDeserializeIdempotency() {
    std::cout << "\n--- idempotency ---\n";

    // int
    std::string s1 = s.Serialize(42);
    auto r1 = s.DeserializeInt(s1);
    std::string s1b = s.Serialize(*r1);
    check("idempotent int", s1 == s1b);

    // double
    std::string s2 = s.Serialize(3.14159265358979323);
    auto r2 = s.DeserializeDouble(s2);
    std::string s2b = s.Serialize(*r2);
    check("idempotent double", s2 == s2b);

    // bool
    std::string s3 = s.Serialize(true);
    auto r3 = s.DeserializeBool(s3);
    std::string s3b = s.Serialize(*r3);
    check("idempotent bool", s3 == s3b);

    // char
    std::string s4 = s.Serialize('Z');
    auto r4 = s.DeserializeChar(s4);
    std::string s4b = s.Serialize(*r4);
    check("idempotent char", s4 == s4b);

    // string
    std::string s5 = s.Serialize(std::string("hello;world:!"));
    auto r5 = s.DeserializeString(s5);
    std::string s5b = s.Serialize(*r5);
    check("idempotent string", s5 == s5b);

    // vector
    std::string s6 = s.Serialize(std::vector<int>{1, 2, 3});
    auto r6 = s.DeserializeVector<int>(s6);
    std::string s6b = s.Serialize(*r6);
    check("idempotent vector<int>", s6 == s6b);

    // map
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    std::string s7 = s.Serialize(m);
    auto r7 = s.DeserializeMap<std::string, int>(s7);
    std::string s7b = s.Serialize(*r7);
    check("idempotent map<string,int>", s7 == s7b);

    // nested
    std::vector<std::vector<int>> nested = {{1, 2}, {3}};
    std::string s8 = s.Serialize(nested);
    auto r8 = s.DeserializeVector<std::vector<int>>(s8);
    std::string s8b = s.Serialize(*r8);
    check("idempotent nested vector", s8 == s8b);
}

// ================================================================
//  27. double precision extremes
// ================================================================
static void testDoublePrecisionExtremes() {
    std::cout << "\n--- double precision extremes ---\n";

    // denorm_min: smallest positive subnormal
    double denorm = std::numeric_limits<double>::denorm_min();
    std::string data = s.Serialize(denorm);
    auto r = s.DeserializeDouble(data);
    check("round-trip denorm_min", r.has_value() && *r == denorm);

    // epsilon
    double eps = std::numeric_limits<double>::epsilon();
    data = s.Serialize(eps);
    r = s.DeserializeDouble(data);
    check("round-trip epsilon", r.has_value() && *r == eps);

    // 0.1 (classic non-exact binary float)
    data = s.Serialize(0.1);
    r = s.DeserializeDouble(data);
    check("round-trip 0.1", r.has_value() && *r == 0.1);

    // Two values 1 ULP apart
    double a = 1.0;
    double b = std::nextafter(1.0, 2.0);  // 1.0 + 1 ULP
    std::string sa = s.Serialize(a);
    std::string sb = s.Serialize(b);
    check("1-ULP-apart values serialize differently", sa != sb);
    auto ra = s.DeserializeDouble(sa);
    auto rb = s.DeserializeDouble(sb);
    check("1-ULP-apart: first round-trips", ra.has_value() && *ra == a);
    check("1-ULP-apart: second round-trips", rb.has_value() && *rb == b);

    // max / lowest
    data = s.Serialize(std::numeric_limits<double>::max());
    r = s.DeserializeDouble(data);
    check("round-trip double max", r.has_value() && *r == std::numeric_limits<double>::max());
}

// ================================================================
//  28. deeply nested containers (5 levels)
// ================================================================
static void testDeeplyNestedContainers() {
    std::cout << "\n--- deeply nested containers ---\n";

    // vector<vector<vector<vector<vector<int>>>>> — 5 levels
    std::vector<std::vector<std::vector<std::vector<std::vector<int>>>>> deep =
        {{{{{1, 2}, {3}}}, {{{4}}}},
         {{{{5, 6, 7}}}}};
    std::string data = s.Serialize(deep);
    auto r = s.DeserializeVector<std::vector<std::vector<std::vector<std::vector<int>>>>>(data);
    check("round-trip 5-level deep vector", r.has_value() && *r == deep);

    // map<string, vector<map<string, vector<int>>>> — mixed nesting
    std::map<std::string, std::vector<std::map<std::string, std::vector<int>>>> mixed;
    mixed["group1"] = {{{"nums", {1, 2, 3}}, {"more", {4, 5}}}};
    mixed["group2"] = {{{"single", {99}}}};
    data = s.Serialize(mixed);
    auto r2 = s.DeserializeMap<std::string, std::vector<std::map<std::string, std::vector<int>>>>(data);
    check("round-trip mixed map/vector nesting", r2.has_value() && *r2 == mixed);

    // Empty at every level
    std::vector<std::vector<std::vector<int>>> emptyDeep = {{{}}, {}, {{}, {}}};
    data = s.Serialize(emptyDeep);
    auto r3 = s.DeserializeVector<std::vector<std::vector<int>>>(data);
    check("round-trip empty at every level", r3.has_value() && *r3 == emptyDeep);

    // Single element at maximum depth
    std::vector<std::vector<std::vector<std::vector<int>>>> single4 = {{{{42}}}};
    data = s.Serialize(single4);
    auto r4 = s.DeserializeVector<std::vector<std::vector<std::vector<int>>>>(data);
    check("round-trip single elem at 4 levels", r4.has_value() && *r4 == single4);
}

// ================================================================
//  29. custom type_id special characters
// ================================================================
static void testCustomTypeIdSpecialChars() {
    std::cout << "\n--- custom type_id special chars ---\n";

    // Register with colons in type_id
    s.RegisterType<int>("my:type",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> {
            return s.DeserializeInt(data);
        }
    );
    std::string data = s.Serialize<int>("my:type", 42);
    // The format is custom:my:type:<len>:<content>; but find/parse may break
    // because the prefix parser uses the type_id verbatim
    auto r = s.Deserialize<int>("my:type", data);
    check("type_id with colon: round-trip", r.has_value() && *r == 42);

    // Register with semicolons in type_id
    s.RegisterType<int>("semi;type",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> {
            return s.DeserializeInt(data);
        }
    );
    data = s.Serialize<int>("semi;type", 7);
    r = s.Deserialize<int>("semi;type", data);
    check("type_id with semicolon: round-trip", r.has_value() && *r == 7);

    // Empty type_id
    s.RegisterType<int>("",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> {
            return s.DeserializeInt(data);
        }
    );
    data = s.Serialize<int>("", 99);
    r = s.Deserialize<int>("", data);
    check("empty type_id: round-trip", r.has_value() && *r == 99);

    // type_id "custom" (recursive-looking prefix)
    s.RegisterType<int>("custom",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> {
            return s.DeserializeInt(data);
        }
    );
    data = s.Serialize<int>("custom", 77);
    r = s.Deserialize<int>("custom", data);
    check("type_id 'custom': round-trip", r.has_value() && *r == 77);

    // Check registration
    check("IsTypeRegistered('my:type')", s.IsTypeRegistered("my:type"));
    check("IsTypeRegistered('semi;type')", s.IsTypeRegistered("semi;type"));
    check("IsTypeRegistered('')", s.IsTypeRegistered(""));
    check("IsTypeRegistered('custom')", s.IsTypeRegistered("custom"));
}

// ================================================================
//  30. strings with embedded null bytes
// ================================================================
static void testStringWithEmbeddedNulls() {
    std::cout << "\n--- strings with embedded nulls ---\n";

    // String with a null byte in the middle
    std::string withNull("hel\0lo", 6);
    check("withNull.size() == 6", withNull.size() == 6);

    std::string data = s.Serialize(withNull);
    auto r = s.DeserializeString(data);
    check("round-trip string with embedded null",
          r.has_value() && r->size() == 6 && *r == withNull);

    // String that is just a null byte
    std::string justNull(1, '\0');
    data = s.Serialize(justNull);
    r = s.DeserializeString(data);
    check("round-trip single null byte string",
          r.has_value() && r->size() == 1 && (*r)[0] == '\0');

    // Multiple null bytes
    std::string multiNull(5, '\0');
    data = s.Serialize(multiNull);
    r = s.DeserializeString(data);
    check("round-trip 5 null bytes",
          r.has_value() && r->size() == 5 && *r == multiNull);
}

// ================================================================
//  31. map key ordering preservation
// ================================================================
static void testMapKeyOrderingPreservation() {
    std::cout << "\n--- map key ordering ---\n";

    // std::map keeps keys sorted; verify this is preserved through round-trip
    std::map<std::string, int> m = {{"cherry", 3}, {"apple", 1}, {"banana", 2}};
    std::string data = s.Serialize(m);
    auto r = s.DeserializeMap<std::string, int>(data);
    check("map keys preserved through round-trip", r.has_value() && *r == m);

    // Verify key order explicitly
    if (r.has_value()) {
        auto it = r->begin();
        check("first key is 'apple'", it->first == "apple");
        ++it;
        check("second key is 'banana'", it->first == "banana");
    }
}

// ================================================================
//  32. char special values
// ================================================================
static void testCharSpecialValues() {
    std::cout << "\n--- char special values ---\n";

    // Null char '\0' — this is tricky because it's the C string terminator
    char nullChar = '\0';
    std::string data = s.Serialize(nullChar);
    auto r = s.DeserializeChar(data);
    check("round-trip null char", r.has_value() && *r == '\0');

    // High-bit char 0xFF
    char highBit = static_cast<char>(0xFF);
    data = s.Serialize(highBit);
    r = s.DeserializeChar(data);
    check("round-trip 0xFF char", r.has_value() && *r == highBit);

    // DEL character (0x7F)
    data = s.Serialize('\x7F');
    r = s.DeserializeChar(data);
    check("round-trip DEL char (0x7F)", r.has_value() && *r == '\x7F');

    // Bell character (0x07)
    data = s.Serialize('\x07');
    r = s.DeserializeChar(data);
    check("round-trip BEL char (0x07)", r.has_value() && *r == '\x07');

    // Char value 0x01 (SOH)
    data = s.Serialize('\x01');
    r = s.DeserializeChar(data);
    check("round-trip SOH char (0x01)", r.has_value() && *r == '\x01');
}

// ================================================================
//  33. stoul negative wrapping — bug-revealing test
// ================================================================
static void testStoulNegativeWrapping() {
    std::cout << "\n--- stoul negative wrapping (bug test) ---\n";

    // "v:-1:i:1;" — stoul("-1") wraps to ULONG_MAX on most platforms,
    // causing reserve(ULONG_MAX) which throws std::bad_alloc.
    // After fix: should return nullopt instead of crashing.
    try {
        auto r = s.DeserializeVector<int>("v:-1:i:1;");
        check("v:-1: returns nullopt (no crash)", !r.has_value());
    } catch (const std::bad_alloc&) {
        check("v:-1: should not throw bad_alloc", false);
    } catch (...) {
        check("v:-1: should not throw", false);
    }

    // Same for map
    try {
        auto r = s.DeserializeMap<std::string, int>("m:-1:s:1:a;i:1;");
        check("m:-1: returns nullopt (no crash)", !r.has_value());
    } catch (const std::bad_alloc&) {
        check("m:-1: should not throw bad_alloc", false);
    } catch (...) {
        check("m:-1: should not throw", false);
    }

    // Huge count that would exhaust memory
    try {
        auto r = s.DeserializeVector<int>("v:999999999999999999:i:1;");
        check("v:huge_count: returns nullopt (no crash)", !r.has_value());
    } catch (const std::bad_alloc&) {
        check("v:huge_count: should not throw bad_alloc", false);
    } catch (...) {
        check("v:huge_count: should not throw", false);
    }

    // Huge count for map
    try {
        auto r = s.DeserializeMap<int, int>("m:999999999999999999:i:1;i:2;");
        check("m:huge_count: returns nullopt (no crash)", !r.has_value());
    } catch (const std::bad_alloc&) {
        check("m:huge_count: should not throw bad_alloc", false);
    } catch (...) {
        check("m:huge_count: should not throw", false);
    }

    // String with huge length
    try {
        auto r = s.DeserializeString("s:18446744073709551615:x;");
        check("s:huge_len: returns nullopt (no crash)", !r.has_value());
    } catch (const std::bad_alloc&) {
        check("s:huge_len: should not throw bad_alloc", false);
    } catch (...) {
        check("s:huge_len: should not throw", false);
    }

    // Negative-looking string length
    try {
        auto r = s.DeserializeString("s:-5:hello;");
        check("s:-5: returns nullopt (no crash)", !r.has_value());
    } catch (...) {
        check("s:-5: should not throw", false);
    }
}

// ================================================================
int main() {
    std::cout << "=== Serializer Round-Trip Tests ===\n";

    // Basic tests (1–9)
    testInt();
    testDouble();
    testBool();
    testChar();
    testString();
    testVector();
    testMap();
    testNested();
    testCustomType();

    // Edge case tests (10–20)
    testIntEdgeCases();
    testDoubleEdgeCases();
    testBoolEdgeCases();
    testCharEdgeCases();
    testStringEdgeCases();
    testVectorEdgeCases();
    testMapEdgeCases();
    testNestedEdgeCases();
    testCustomTypeEdgeCases();
    testDeserializeAtEdgeCases();
    testCrossTypeRejection();

    // Additional edge case tests (21–33)
    testStrtodQuirks();
    testStoiQuirks();
    testTrailingDataIgnored();
    testVectorBoolDirect();
    testConstCharOverload();
    testSerializeDeserializeIdempotency();
    testDoublePrecisionExtremes();
    testDeeplyNestedContainers();
    testCustomTypeIdSpecialChars();
    testStringWithEmbeddedNulls();
    testMapKeyOrderingPreservation();
    testCharSpecialValues();
    testStoulNegativeWrapping();

    std::cout << "\n=== Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return (failed == 0) ? 0 : 1;
}
