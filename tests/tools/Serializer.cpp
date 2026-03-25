// Serializer tests converted to Catch2 style

#include <catch2/catch.hpp>

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <climits>
#include <cfloat>

#include "../../source/tools/Serializer.hpp"

// ================================================================
//  Shared fixture
// ================================================================
static cse498::Serializer s;

// ================================================================
//  Helper: Agent custom type
// ================================================================
struct Agent {
    std::string name;
    double health;
    int level;
};

struct Inventory {
    std::string item;
    std::vector<int> quantities;
};

static void registerAgent() {
    s.RegisterType<Agent>("Agent",
        [](const Agent& a) -> std::string {
            return s.Serialize(a.name) + s.Serialize(a.health) + s.Serialize(a.level);
        },
        [](const std::string& data) -> std::optional<Agent> {
            Agent a;
            size_t pos = 0;
            auto name   = s.DeserializeAt<std::string>(data, pos);
            auto health = s.DeserializeAt<double>(data, pos);
            auto level  = s.DeserializeAt<int>(data, pos);
            if (!name || !health || !level) return std::nullopt;
            a.name = *name; a.health = *health; a.level = *level;
            return a;
        }
    );
}

// ================================================================
//  1. int — basic
// ================================================================
TEST_CASE("int basic round-trips", "[int]") {
    REQUIRE(s.DeserializeInt(s.Serialize(42)).value_or(-1)  == 42);
    REQUIRE(s.DeserializeInt(s.Serialize(-7)).value_or(0)   == -7);
    REQUIRE(s.DeserializeInt(s.Serialize(0)).value_or(-1)   == 0);

    CHECK_FALSE(s.DeserializeInt("d:3.14;").has_value());
    CHECK_FALSE(s.DeserializeInt("xyz").has_value());
}

// ================================================================
//  2. double — basic
// ================================================================
TEST_CASE("double basic round-trips", "[double]") {
    auto r = s.DeserializeDouble(s.Serialize(3.14159265358979323));
    REQUIRE(r.has_value());
    REQUIRE(*r == 3.14159265358979323);

    auto r2 = s.DeserializeDouble(s.Serialize(-0.001));
    REQUIRE(r2.has_value());
    REQUIRE(std::abs(*r2 - (-0.001)) < 1e-15);

    auto r3 = s.DeserializeDouble(s.Serialize(0.0));
    REQUIRE(r3.has_value());
    REQUIRE(*r3 == 0.0);

    CHECK_FALSE(s.DeserializeDouble("i:42;").has_value());
}

// ================================================================
//  3. bool — basic
// ================================================================
TEST_CASE("bool basic round-trips", "[bool]") {
    REQUIRE(s.DeserializeBool(s.Serialize(true)).value_or(false)  == true);
    REQUIRE(s.DeserializeBool(s.Serialize(false)).value_or(true)  == false);
    CHECK_FALSE(s.DeserializeBool("i:1;").has_value());
}

// ================================================================
//  4. char — basic
// ================================================================
TEST_CASE("char basic round-trips", "[char]") {
    REQUIRE(s.DeserializeChar(s.Serialize('A')).value_or('\0') == 'A');
    REQUIRE(s.DeserializeChar(s.Serialize('z')).value_or('\0') == 'z');
    CHECK_FALSE(s.DeserializeChar("s:1:A;").has_value());
}

// ================================================================
//  5. string — basic
// ================================================================
TEST_CASE("string basic round-trips", "[string]") {
    REQUIRE(s.DeserializeString(s.Serialize(std::string("hello"))).value_or("") == "hello");

    auto empty = s.DeserializeString(s.Serialize(std::string("")));
    REQUIRE(empty.has_value());
    REQUIRE(empty->empty());

    std::string tricky = "key:val;foo:bar;";
    REQUIRE(s.DeserializeString(s.Serialize(tricky)).value_or("") == tricky);

    CHECK_FALSE(s.DeserializeString("i:42;").has_value());
}

// ================================================================
//  6. vector — basic
// ================================================================
TEST_CASE("vector basic round-trips", "[vector]") {
    std::vector<int> vi = {1, 2, 3};
    REQUIRE(s.DeserializeVector<int>(s.Serialize(vi)).value_or(std::vector<int>{}) == vi);

    std::vector<int> empty;
    auto re = s.DeserializeVector<int>(s.Serialize(empty));
    REQUIRE(re.has_value());
    REQUIRE(re->empty());

    std::vector<std::string> vs = {"hello", "world", "a;b"};
    REQUIRE(s.DeserializeVector<std::string>(s.Serialize(vs)).value_or(std::vector<std::string>{}) == vs);

    std::vector<double> vd = {1.1, 2.2, 3.3};
    REQUIRE(s.DeserializeVector<double>(s.Serialize(vd)).value_or(std::vector<double>{}) == vd);

    CHECK_FALSE(s.DeserializeVector<int>("i:42;").has_value());
}

// ================================================================
//  7. map — basic
// ================================================================
TEST_CASE("map basic round-trips", "[map]") {
    std::map<std::string, int> msi = {{"alice", 1}, {"bob", 2}};
    REQUIRE(s.DeserializeMap<std::string, int>(s.Serialize(msi)).value_or(decltype(msi){}) == msi);

    std::map<std::string, int> empty;
    auto re = s.DeserializeMap<std::string, int>(s.Serialize(empty));
    REQUIRE(re.has_value());
    REQUIRE(re->empty());

    std::map<int, std::string> mis = {{1, "one"}, {2, "two"}};
    REQUIRE(s.DeserializeMap<int, std::string>(s.Serialize(mis)).value_or(decltype(mis){}) == mis);

    std::map<std::string, double> msd = {{"pi", 3.14}, {"e", 2.71}};
    REQUIRE(s.DeserializeMap<std::string, double>(s.Serialize(msd)).value_or(decltype(msd){}) == msd);

    CHECK_FALSE(s.DeserializeMap<std::string, int>("i:42;").has_value());
}

// ================================================================
//  8. Nested containers — basic
// ================================================================
TEST_CASE("nested containers basic round-trips", "[nested]") {
    std::vector<std::vector<int>> vvi = {{1, 2}, {3, 4, 5}};
    REQUIRE(s.DeserializeVector<std::vector<int>>(s.Serialize(vvi)).value_or(decltype(vvi){}) == vvi);

    std::map<std::string, std::vector<int>> msv;
    msv["odds"]  = {1, 3, 5};
    msv["evens"] = {2, 4, 6};
    REQUIRE(s.DeserializeMap<std::string, std::vector<int>>(s.Serialize(msv)).value_or(decltype(msv){}) == msv);
}

// ================================================================
//  9. Custom type (Agent) — basic
// ================================================================
TEST_CASE("custom type Agent basic", "[custom]") {
    registerAgent();

    CHECK(s.IsTypeRegistered("Agent"));
    CHECK_FALSE(s.IsTypeRegistered("Unknown"));

    Agent agent{"Steve", 100.0, 7};
    std::string data = s.Serialize<Agent>("Agent", agent);
    REQUIRE(data.substr(0, 13) == "custom:Agent:");

    auto restored = s.Deserialize<Agent>("Agent", data);
    REQUIRE(restored.has_value());
    REQUIRE(restored->name   == "Steve");
    REQUIRE(restored->health == 100.0);
    REQUIRE(restored->level  == 7);

    Agent agent2{"A;special:name", -0.5, 0};
    auto r2 = s.Deserialize<Agent>("Agent", s.Serialize<Agent>("Agent", agent2));
    REQUIRE(r2.has_value());
    REQUIRE(r2->name == "A;special:name");
    REQUIRE(std::abs(r2->health - (-0.5)) < 1e-15);
    REQUIRE(r2->level == 0);

    Agent agent3{"", 0.0, -1};
    auto r3 = s.Deserialize<Agent>("Agent", s.Serialize<Agent>("Agent", agent3));
    REQUIRE(r3.has_value());
    REQUIRE(r3->name.empty());
    REQUIRE(r3->health == 0.0);
    REQUIRE(r3->level  == -1);

    CHECK_FALSE(s.Deserialize<Agent>("Unknown", data).has_value());
    CHECK_FALSE(s.Deserialize<Agent>("Agent", "garbage").has_value());
    CHECK_FALSE(s.Deserialize<Agent>("Agent", "i:42;").has_value());
}

// ================================================================
//  10. int — edge cases
// ================================================================
TEST_CASE("int edge cases", "[int][edge]") {
    SECTION("boundary values") {
        REQUIRE(s.DeserializeInt(s.Serialize(INT_MAX)).value_or(0) == INT_MAX);
        REQUIRE(s.DeserializeInt(s.Serialize(INT_MIN)).value_or(0) == INT_MIN);
        REQUIRE(s.DeserializeInt(s.Serialize(1)).value_or(0)       == 1);
        REQUIRE(s.DeserializeInt(s.Serialize(-1)).value_or(0)      == -1);
    }

    SECTION("malformed inputs") {
        CHECK_FALSE(s.DeserializeInt("").has_value());
        CHECK_FALSE(s.DeserializeInt("i").has_value());
        CHECK_FALSE(s.DeserializeInt("i:").has_value());
        CHECK_FALSE(s.DeserializeInt("i:42").has_value());
        CHECK_FALSE(s.DeserializeInt("i:;").has_value());
        CHECK_FALSE(s.DeserializeInt("i:42abc;").has_value());
        CHECK_FALSE(s.DeserializeInt("i:3.14;").has_value());
        CHECK_FALSE(s.DeserializeInt("b:1;").has_value());
        CHECK_FALSE(s.DeserializeInt("c:A;").has_value());
        CHECK_FALSE(s.DeserializeInt("s:2:42;").has_value());
        CHECK_FALSE(s.DeserializeInt("v:1:i:1;").has_value());
        CHECK_FALSE(s.DeserializeInt("m:0:").has_value());
        CHECK_FALSE(s.DeserializeInt("i:99999999999999999999;").has_value());
    }


}

// ================================================================
//  11. double — edge cases
// ================================================================
TEST_CASE("double edge cases", "[double][edge]") {
    SECTION("extreme values") {
        REQUIRE(s.DeserializeDouble(s.Serialize(1e308)).value_or(0.0)   == 1e308);
        REQUIRE(s.DeserializeDouble(s.Serialize(-1e308)).value_or(0.0)  == -1e308);
        REQUIRE(s.DeserializeDouble(s.Serialize(DBL_MIN)).value_or(0.0) == DBL_MIN);
        REQUIRE(s.DeserializeDouble(s.Serialize(DBL_MAX)).value_or(0.0) == DBL_MAX);

        auto r_sub = s.DeserializeDouble(s.Serialize(1e-308));
        REQUIRE(r_sub.has_value());
        REQUIRE(std::abs(*r_sub - 1e-308) / 1e-308 < 1e-10);
    }

    SECTION("special float values") {
        auto r_neg_zero = s.DeserializeDouble(s.Serialize(-0.0));
        REQUIRE(r_neg_zero.has_value());
        REQUIRE(*r_neg_zero == 0.0);

        auto r_inf = s.DeserializeDouble(s.Serialize(std::numeric_limits<double>::infinity()));
        REQUIRE(r_inf.has_value());
        REQUIRE(std::isinf(*r_inf));
        REQUIRE(*r_inf > 0);

        auto r_ninf = s.DeserializeDouble(s.Serialize(-std::numeric_limits<double>::infinity()));
        REQUIRE(r_ninf.has_value());
        REQUIRE(std::isinf(*r_ninf));
        REQUIRE(*r_ninf < 0);

        auto r_nan = s.DeserializeDouble(s.Serialize(std::numeric_limits<double>::quiet_NaN()));
        REQUIRE(r_nan.has_value());
        REQUIRE(std::isnan(*r_nan));
    }

    SECTION("malformed inputs") {
        CHECK_FALSE(s.DeserializeDouble("").has_value());
        CHECK_FALSE(s.DeserializeDouble("d").has_value());
        CHECK_FALSE(s.DeserializeDouble("d:").has_value());
        CHECK_FALSE(s.DeserializeDouble("d:3.14").has_value());
        CHECK_FALSE(s.DeserializeDouble("d:;").has_value());
        CHECK_FALSE(s.DeserializeDouble("d:abc;").has_value());
        CHECK_FALSE(s.DeserializeDouble("b:1;").has_value());
        CHECK_FALSE(s.DeserializeDouble("c:A;").has_value());
        CHECK_FALSE(s.DeserializeDouble("s:3:3.1;").has_value());
    }
}

// ================================================================
//  12. bool — edge cases
// ================================================================
TEST_CASE("bool edge cases", "[bool][edge]") {
    SECTION("invalid digit values") {
        CHECK_FALSE(s.DeserializeBool("b:2;").has_value());
        CHECK_FALSE(s.DeserializeBool("b:9;").has_value());
        CHECK_FALSE(s.DeserializeBool("b:a;").has_value());
        CHECK_FALSE(s.DeserializeBool("b:T;").has_value());
    }

    SECTION("truncated / malformed") {
        CHECK_FALSE(s.DeserializeBool("").has_value());
        CHECK_FALSE(s.DeserializeBool("b").has_value());
        CHECK_FALSE(s.DeserializeBool("b:").has_value());
        CHECK_FALSE(s.DeserializeBool("b:1").has_value());
        CHECK_FALSE(s.DeserializeBool("b:1x").has_value());
    }

    SECTION("wrong type") {
        CHECK_FALSE(s.DeserializeBool("d:1.0;").has_value());
        CHECK_FALSE(s.DeserializeBool("c:1;").has_value());
        CHECK_FALSE(s.DeserializeBool("s:1:1;").has_value());
    }
}

// ================================================================
//  13. char — edge cases
// ================================================================
TEST_CASE("char edge cases", "[char][edge]") {
    SECTION("special characters") {
        REQUIRE(s.DeserializeChar(s.Serialize(' ')).value_or('\0')  == ' ');
        REQUIRE(s.DeserializeChar(s.Serialize('0')).value_or('\0')  == '0');
        REQUIRE(s.DeserializeChar(s.Serialize('9')).value_or('\0')  == '9');
        REQUIRE(s.DeserializeChar(s.Serialize(';')).value_or('\0')  == ';');
        REQUIRE(s.DeserializeChar(s.Serialize(':')).value_or('\0')  == ':');
        REQUIRE(s.DeserializeChar(s.Serialize('\t')).value_or('\0') == '\t');
        REQUIRE(s.DeserializeChar(s.Serialize('\n')).value_or('\0') == '\n');
    }

    SECTION("malformed inputs") {
        CHECK_FALSE(s.DeserializeChar("").has_value());
        CHECK_FALSE(s.DeserializeChar("c").has_value());
        CHECK_FALSE(s.DeserializeChar("c:").has_value());
        CHECK_FALSE(s.DeserializeChar("c:A").has_value());
    }

    SECTION("wrong type") {
        CHECK_FALSE(s.DeserializeChar("i:65;").has_value());
        CHECK_FALSE(s.DeserializeChar("b:0;").has_value());
    }
}

// ================================================================
//  14. string — edge cases
// ================================================================
TEST_CASE("string edge cases", "[string][edge]") {
    SECTION("special content") {
        REQUIRE(s.DeserializeString(s.Serialize(std::string("\t\n\r"))).value_or("") == "\t\n\r");
        REQUIRE(s.DeserializeString(s.Serialize(std::string("i:42;"))).value_or("") == "i:42;");
        REQUIRE(s.DeserializeString(s.Serialize(std::string("b:1;"))).value_or("") == "b:1;");

        std::string longStr(1000, 'x');
        REQUIRE(s.DeserializeString(s.Serialize(longStr)).value_or("") == longStr);

        std::string delims = ":::;;;:::;;;";
        REQUIRE(s.DeserializeString(s.Serialize(delims)).value_or("") == delims);

        REQUIRE(s.DeserializeString(s.Serialize(std::string("X"))).value_or("") == "X");
    }

    SECTION("malformed inputs") {
        CHECK_FALSE(s.DeserializeString("").has_value());
        CHECK_FALSE(s.DeserializeString("s").has_value());
        CHECK_FALSE(s.DeserializeString("s:").has_value());
        CHECK_FALSE(s.DeserializeString("s:5hello;").has_value());
        CHECK_FALSE(s.DeserializeString("s:100:hi;").has_value());
        CHECK_FALSE(s.DeserializeString("s:1:hello;").has_value());
        CHECK_FALSE(s.DeserializeString("s:5:hello").has_value());
        CHECK_FALSE(s.DeserializeString("s:-1:x;").has_value());
    }

    SECTION("wrong type") {
        CHECK_FALSE(s.DeserializeString("b:0;").has_value());
        CHECK_FALSE(s.DeserializeString("c:h;").has_value());
    }
}

// ================================================================
//  15. vector — edge cases
// ================================================================
TEST_CASE("vector edge cases", "[vector][edge]") {
    SECTION("special element types") {
        std::vector<int> single = {42};
        REQUIRE(s.DeserializeVector<int>(s.Serialize(single)).value_or(decltype(single){}) == single);

        std::vector<char> vc = {'a', 'b', 'c'};
        REQUIRE(s.DeserializeVector<char>(s.Serialize(vc)).value_or(decltype(vc){}) == vc);

        std::vector<int> large;
        for (int i = 0; i < 100; ++i) large.push_back(i);
        REQUIRE(s.DeserializeVector<int>(s.Serialize(large)).value_or(decltype(large){}) == large);
    }

    SECTION("vector<bool>") {
        std::vector<bool> vb = {true, false, true, true, false};
        auto r = s.DeserializeVector<bool>(s.Serialize(vb));
        REQUIRE(r.has_value());
        REQUIRE(r->size() == 5);
        REQUIRE((*r)[0] == true);
        REQUIRE((*r)[1] == false);
        REQUIRE((*r)[2] == true);
        REQUIRE((*r)[3] == true);
        REQUIRE((*r)[4] == false);
    }

    SECTION("malformed inputs") {
        CHECK_FALSE(s.DeserializeVector<int>("v:5:i:1;i:2;").has_value());
        CHECK_FALSE(s.DeserializeVector<int>("v:3").has_value());
        CHECK_FALSE(s.DeserializeVector<int>("v:abc:i:1;").has_value());
        CHECK_FALSE(s.DeserializeVector<int>("").has_value());
        CHECK_FALSE(s.DeserializeVector<int>("v:").has_value());
        CHECK_FALSE(s.DeserializeVector<int>("v:2:s:2:hi;s:2:lo;").has_value());
    }
}

// ================================================================
//  16. map — edge cases
// ================================================================
TEST_CASE("map edge cases", "[map][edge]") {
    SECTION("various key/value types") {
        std::map<std::string, int>  single = {{"only", 1}};
        REQUIRE(s.DeserializeMap<std::string, int>(s.Serialize(single)).value_or(decltype(single){}) == single);

        std::map<int, int> mii = {{1, 10}, {2, 20}, {3, 30}};
        REQUIRE(s.DeserializeMap<int, int>(s.Serialize(mii)).value_or(decltype(mii){}) == mii);

        std::map<std::string, bool> msb = {{"active", true}, {"deleted", false}};
        REQUIRE(s.DeserializeMap<std::string, bool>(s.Serialize(msb)).value_or(decltype(msb){}) == msb);

        std::map<char, std::string> mcs = {{'a', "alpha"}, {'b', "bravo"}};
        REQUIRE(s.DeserializeMap<char, std::string>(s.Serialize(mcs)).value_or(decltype(mcs){}) == mcs);

        std::map<std::string, int> tricky = {{"key;with:delims", 99}};
        REQUIRE(s.DeserializeMap<std::string, int>(s.Serialize(tricky)).value_or(decltype(tricky){}) == tricky);
    }

    SECTION("malformed inputs") {
        CHECK_FALSE(s.DeserializeMap<std::string, int>("m:5:s:1:a;i:1;").has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>("m:2").has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>("").has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>("m:").has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>("m:1:s:3:key;").has_value());
    }
}

// ================================================================
//  17. nested containers — edge cases
// ================================================================
TEST_CASE("nested containers edge cases", "[nested][edge]") {
    SECTION("3-level deep vector") {
        std::vector<std::vector<std::vector<int>>> deep = {{{1, 2}, {3}}, {{4, 5, 6}}};
        REQUIRE(s.DeserializeVector<std::vector<std::vector<int>>>(s.Serialize(deep)).value_or(decltype(deep){}) == deep);
    }

    SECTION("map of maps") {
        std::map<std::string, std::map<std::string, int>> mom;
        mom["team1"] = {{"alice", 10}, {"bob", 20}};
        mom["team2"] = {{"carol", 30}};
        REQUIRE(s.DeserializeMap<std::string, std::map<std::string, int>>(s.Serialize(mom)).value_or(decltype(mom){}) == mom);
    }

    SECTION("vector of maps") {
        std::vector<std::map<std::string, int>> vom;
        vom.push_back({{"x", 1}, {"y", 2}});
        vom.push_back({{"z", 3}});
        REQUIRE(s.DeserializeVector<std::map<std::string, int>>(s.Serialize(vom)).value_or(decltype(vom){}) == vom);
    }

    SECTION("map of vectors with delimiters") {
        std::map<int, std::vector<std::string>> mivs;
        mivs[1] = {"hello", "world"};
        mivs[2] = {"foo;bar", "baz:qux"};
        REQUIRE(s.DeserializeMap<int, std::vector<std::string>>(s.Serialize(mivs)).value_or(decltype(mivs){}) == mivs);
    }

    SECTION("empty nested containers") {
        std::vector<std::vector<int>> emptyNested = {{}, {}, {}};
        REQUIRE(s.DeserializeVector<std::vector<int>>(s.Serialize(emptyNested)).value_or(decltype(emptyNested){}) == emptyNested);

        std::map<std::string, std::vector<int>> emptyVals;
        emptyVals["a"] = {};
        emptyVals["b"] = {1};
        REQUIRE(s.DeserializeMap<std::string, std::vector<int>>(s.Serialize(emptyVals)).value_or(decltype(emptyVals){}) == emptyVals);
    }
}

// ================================================================
//  18. custom type — edge cases
// ================================================================
TEST_CASE("custom type edge cases", "[custom][edge]") {
    SECTION("re-registration overwrites previous entry") {
        s.RegisterType<Agent>("AgentOverwrite",
            [](const Agent&) -> std::string { return ""; },
            [](const std::string&) -> std::optional<Agent> { return std::nullopt; }
        );
        s.RegisterType<Agent>("AgentOverwrite",
            [](const Agent& a) -> std::string {
                return s.Serialize(a.name) + s.Serialize(a.health) + s.Serialize(a.level);
            },
            [](const std::string& data) -> std::optional<Agent> {
                Agent a; size_t pos = 0;
                auto name   = s.DeserializeAt<std::string>(data, pos);
                auto health = s.DeserializeAt<double>(data, pos);
                auto level  = s.DeserializeAt<int>(data, pos);
                if (!name || !health || !level) return std::nullopt;
                a.name = *name; a.health = *health; a.level = *level;
                return a;
            }
        );
        Agent testAgent{"Overwrite", 50.0, 3};
        auto r = s.Deserialize<Agent>("AgentOverwrite", s.Serialize<Agent>("AgentOverwrite", testAgent));
        REQUIRE(r.has_value());
        REQUIRE(r->name == "Overwrite");
    }

    SECTION("tampered content length") {
        CHECK_FALSE(s.Deserialize<Agent>("Agent", "custom:Agent:5:s:5:Steve;d:100;i:7;;").has_value());
        CHECK_FALSE(s.Deserialize<Agent>("Agent", "custom:Agent:9999:s:5:Steve;d:100;i:7;;").has_value());
    }

    SECTION("missing trailing semicolon") {
        std::string valid = s.Serialize<Agent>("Agent", Agent{"Bob", 50.0, 2});
        CHECK_FALSE(s.Deserialize<Agent>("Agent", valid.substr(0, valid.size() - 1)).has_value());
    }

    SECTION("empty inner content") {
        CHECK_FALSE(s.Deserialize<Agent>("Agent", "custom:Agent:0:;").has_value());
    }

    SECTION("type_id mismatch") {
        std::string agentData = s.Serialize<Agent>("Agent", Agent{"Mix", 1.0, 1});
        CHECK_FALSE(s.Deserialize<Agent>("AgentOverwrite", agentData).has_value());
    }

    SECTION("corrupted and non-numeric length") {
        CHECK_FALSE(s.Deserialize<Agent>("Agent", "custom:Agent:7:garbage;").has_value());
        CHECK_FALSE(s.Deserialize<Agent>("Agent", "custom:Agent:abc:stuff;").has_value());
    }

    SECTION("separate serializer instances have independent registries") {
        cse498::Serializer s2;
        std::string agentData = s.Serialize<Agent>("Agent", Agent{"Iso", 1.0, 1});
        CHECK_FALSE(s2.IsTypeRegistered("Agent"));
        CHECK_FALSE(s2.Deserialize<Agent>("Agent", agentData).has_value());
    }

    SECTION("custom type with container fields") {
        s.RegisterType<Inventory>("Inventory",
            [](const Inventory& inv) -> std::string {
                return s.Serialize(inv.item) + s.Serialize(inv.quantities);
            },
            [](const std::string& data) -> std::optional<Inventory> {
                Inventory inv; size_t pos = 0;
                auto item = s.DeserializeAt<std::string>(data, pos);
                auto qty  = s.DeserializeAt<std::vector<int>>(data, pos);
                if (!item || !qty) return std::nullopt;
                inv.item = *item; inv.quantities = *qty;
                return inv;
            }
        );

        Inventory inv{"Sword", {1, 5, 10}};
        auto r = s.Deserialize<Inventory>("Inventory", s.Serialize<Inventory>("Inventory", inv));
        REQUIRE(r.has_value());
        REQUIRE(r->item == "Sword");
        REQUIRE(r->quantities == std::vector<int>({1, 5, 10}));

        Inventory emptyInv{"Nothing", {}};
        auto r2 = s.Deserialize<Inventory>("Inventory", s.Serialize<Inventory>("Inventory", emptyInv));
        REQUIRE(r2.has_value());
        REQUIRE(r2->item == "Nothing");
        REQUIRE(r2->quantities.empty());
    }

    SECTION("multiple custom types coexist") {
        CHECK(s.IsTypeRegistered("Agent"));
        CHECK(s.IsTypeRegistered("Inventory"));

        Agent a1{"Multi", 99.0, 5};
        Inventory i1{"Shield", {2, 3}};
        auto ar = s.Deserialize<Agent>("Agent", s.Serialize<Agent>("Agent", a1));
        auto ir = s.Deserialize<Inventory>("Inventory", s.Serialize<Inventory>("Inventory", i1));

        REQUIRE(ar.has_value());
        REQUIRE(ar->name == "Multi");
        REQUIRE(ir.has_value());
        REQUIRE(ir->item == "Shield");
        REQUIRE(ir->quantities == std::vector<int>({2, 3}));

        // Cross-deserialize should fail
        CHECK_FALSE(s.Deserialize<Agent>("Agent", s.Serialize<Inventory>("Inventory", i1)).has_value());
    }
}

// ================================================================
//  19. DeserializeAt — positional parsing edge cases
// ================================================================
TEST_CASE("DeserializeAt positional parsing", "[deserializeAt]") {
    SECTION("sequential mixed types") {
        std::string combined = s.Serialize(42) + s.Serialize(std::string("hi")) +
                               s.Serialize(true) + s.Serialize('Z') + s.Serialize(3.14);
        size_t pos = 0;

        auto ri = s.DeserializeAt<int>(combined, pos);
        REQUIRE(ri.has_value());
        REQUIRE(*ri == 42);

        auto rs = s.DeserializeAt<std::string>(combined, pos);
        REQUIRE(rs.has_value());
        REQUIRE(*rs == "hi");

        auto rb = s.DeserializeAt<bool>(combined, pos);
        REQUIRE(rb.has_value());
        REQUIRE(*rb == true);

        auto rc = s.DeserializeAt<char>(combined, pos);
        REQUIRE(rc.has_value());
        REQUIRE(*rc == 'Z');

        auto rd = s.DeserializeAt<double>(combined, pos);
        REQUIRE(rd.has_value());

        REQUIRE(pos == combined.size());

        CHECK_FALSE(s.DeserializeAt<int>(combined, pos).has_value());
    }

    SECTION("vector then continue") {
        std::string vecStr = s.Serialize(std::vector<int>{10, 20}) + s.Serialize(99);
        size_t pos = 0;

        auto rv = s.DeserializeAt<std::vector<int>>(vecStr, pos);
        REQUIRE(rv.has_value());
        REQUIRE(*rv == std::vector<int>({10, 20}));

        auto rAfter = s.DeserializeAt<int>(vecStr, pos);
        REQUIRE(rAfter.has_value());
        REQUIRE(*rAfter == 99);
    }

    SECTION("map then continue") {
        std::map<int, int> m = {{1, 2}};
        std::string mapStr = s.Serialize(m) + s.Serialize(std::string("end"));
        size_t pos = 0;

        auto rm = s.DeserializeAt<std::map<int, int>>(mapStr, pos);
        REQUIRE(rm.has_value());
        REQUIRE(*rm == m);

        auto rEnd = s.DeserializeAt<std::string>(mapStr, pos);
        REQUIRE(rEnd.has_value());
        REQUIRE(*rEnd == "end");
    }

    SECTION("wrong type at position") {
        std::string intData = s.Serialize(42);
        size_t pos = 0;
        CHECK_FALSE(s.DeserializeAt<std::string>(intData, pos).has_value());
    }
}

// ================================================================
//  20. Cross-type rejection — systematic
// ================================================================
TEST_CASE("cross-type rejection", "[cross-type]") {
    std::string intData  = s.Serialize(42);
    std::string dblData  = s.Serialize(3.14);
    std::string boolData = s.Serialize(true);
    std::string charData = s.Serialize('A');
    std::string strData  = s.Serialize(std::string("hi"));
    std::string vecData  = s.Serialize(std::vector<int>{1});
    std::string mapData  = s.Serialize(std::map<std::string, int>{{"k", 1}});

    SECTION("int rejects all others") {
        CHECK_FALSE(s.DeserializeInt(dblData).has_value());
        CHECK_FALSE(s.DeserializeInt(boolData).has_value());
        CHECK_FALSE(s.DeserializeInt(charData).has_value());
        CHECK_FALSE(s.DeserializeInt(strData).has_value());
        CHECK_FALSE(s.DeserializeInt(vecData).has_value());
        CHECK_FALSE(s.DeserializeInt(mapData).has_value());
    }

    SECTION("double rejects all others") {
        CHECK_FALSE(s.DeserializeDouble(intData).has_value());
        CHECK_FALSE(s.DeserializeDouble(boolData).has_value());
        CHECK_FALSE(s.DeserializeDouble(charData).has_value());
        CHECK_FALSE(s.DeserializeDouble(strData).has_value());
        CHECK_FALSE(s.DeserializeDouble(vecData).has_value());
        CHECK_FALSE(s.DeserializeDouble(mapData).has_value());
    }

    SECTION("bool rejects all others") {
        CHECK_FALSE(s.DeserializeBool(intData).has_value());
        CHECK_FALSE(s.DeserializeBool(dblData).has_value());
        CHECK_FALSE(s.DeserializeBool(charData).has_value());
        CHECK_FALSE(s.DeserializeBool(strData).has_value());
        CHECK_FALSE(s.DeserializeBool(vecData).has_value());
        CHECK_FALSE(s.DeserializeBool(mapData).has_value());
    }

    SECTION("char rejects all others") {
        CHECK_FALSE(s.DeserializeChar(intData).has_value());
        CHECK_FALSE(s.DeserializeChar(dblData).has_value());
        CHECK_FALSE(s.DeserializeChar(boolData).has_value());
        CHECK_FALSE(s.DeserializeChar(strData).has_value());
        CHECK_FALSE(s.DeserializeChar(vecData).has_value());
        CHECK_FALSE(s.DeserializeChar(mapData).has_value());
    }

    SECTION("string rejects all others") {
        CHECK_FALSE(s.DeserializeString(intData).has_value());
        CHECK_FALSE(s.DeserializeString(dblData).has_value());
        CHECK_FALSE(s.DeserializeString(boolData).has_value());
        CHECK_FALSE(s.DeserializeString(charData).has_value());
        CHECK_FALSE(s.DeserializeString(vecData).has_value());
        CHECK_FALSE(s.DeserializeString(mapData).has_value());
    }

    SECTION("vector rejects all non-vector") {
        CHECK_FALSE(s.DeserializeVector<int>(intData).has_value());
        CHECK_FALSE(s.DeserializeVector<int>(dblData).has_value());
        CHECK_FALSE(s.DeserializeVector<int>(boolData).has_value());
        CHECK_FALSE(s.DeserializeVector<int>(charData).has_value());
        CHECK_FALSE(s.DeserializeVector<int>(strData).has_value());
        CHECK_FALSE(s.DeserializeVector<int>(mapData).has_value());
    }

    SECTION("map rejects all non-map") {
        CHECK_FALSE(s.DeserializeMap<std::string, int>(intData).has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>(dblData).has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>(boolData).has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>(charData).has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>(strData).has_value());
        CHECK_FALSE(s.DeserializeMap<std::string, int>(vecData).has_value());
    }
}

// ================================================================
//  21. strtod quirks (now locale-safe via strtod_l)
// ================================================================
TEST_CASE("strtod parser quirks", "[double][quirks]") {
    CHECK(s.DeserializeDouble("d:0x1.0p0;").value_or(0.0) == 1.0);

    auto rSpace = s.DeserializeDouble("d: 3.14;");
    REQUIRE(rSpace.has_value());
    REQUIRE(std::abs(*rSpace - 3.14) < 1e-12);

    REQUIRE(s.DeserializeDouble("d:+2.5;").value_or(0.0) == 2.5);

    auto rInf = s.DeserializeDouble("d:inf;");
    REQUIRE(rInf.has_value());
    REQUIRE(std::isinf(*rInf));
    REQUIRE(*rInf > 0);

    auto rNan = s.DeserializeDouble("d:nan;");
    REQUIRE(rNan.has_value());
    REQUIRE(std::isnan(*rNan));

    CHECK_FALSE(s.DeserializeDouble("d:;").has_value());
    CHECK_FALSE(s.DeserializeDouble("d:hello;").has_value());
}

// ================================================================
//  22. stoi quirks
// ================================================================
TEST_CASE("stoi parser quirks", "[int][quirks]") {
    REQUIRE(s.DeserializeInt("i:007;").value_or(-1) == 7);
    REQUIRE(s.DeserializeInt("i:-0;").value_or(-1)  == 0);

    CHECK_FALSE(s.DeserializeInt("i:0xFF;").has_value());
    CHECK_FALSE(s.DeserializeInt("i:;").has_value());
    CHECK_FALSE(s.DeserializeInt("i: ;").has_value());
}

// ================================================================
//  23. trailing data ignored by public API
// ================================================================
TEST_CASE("trailing data ignored by public Deserialize functions", "[trailing]") {
    REQUIRE(s.DeserializeInt("i:42;JUNK").value_or(-1)                   == 42);
    REQUIRE(std::abs(s.DeserializeDouble("d:3.14;JUNK").value_or(0.0) - 3.14) < 1e-12);
    REQUIRE(s.DeserializeBool("b:1;JUNK").value_or(false)                == true);
    REQUIRE(s.DeserializeChar("c:A;JUNK").value_or('\0')                 == 'A');
    REQUIRE(s.DeserializeString("s:2:hi;JUNK").value_or("")              == "hi");

    auto rv = s.DeserializeVector<int>("v:1:i:1;JUNK");
    REQUIRE(rv.has_value());
    REQUIRE(rv->size() == 1);
    REQUIRE((*rv)[0] == 1);

    auto rm = s.DeserializeMap<std::string, int>("m:1:s:1:k;i:1;JUNK");
    REQUIRE(rm.has_value());
    REQUIRE(rm->size() == 1);

    Agent a{"Test", 1.0, 1};
    std::string agentData = s.Serialize<Agent>("Agent", a) + "JUNK";
    auto ra = s.Deserialize<Agent>("Agent", agentData);
    REQUIRE(ra.has_value());
    REQUIRE(ra->name == "Test");
}

// ================================================================
//  24. vector<bool> direct serialization
// ================================================================
TEST_CASE("vector<bool> direct serialization", "[vector][bool]") {
    std::vector<bool> vb = {true, false, true, true, false};
    auto r = s.DeserializeVector<bool>(s.Serialize(vb));
    REQUIRE(r.has_value());
    REQUIRE(r->size() == 5);
    REQUIRE((*r)[0] == true);
    REQUIRE((*r)[1] == false);
    REQUIRE((*r)[2] == true);
    REQUIRE((*r)[3] == true);
    REQUIRE((*r)[4] == false);

    std::vector<bool> empty;
    auto re = s.DeserializeVector<bool>(s.Serialize(empty));
    REQUIRE(re.has_value());
    REQUIRE(re->empty());

    std::vector<bool> single = {true};
    auto rs = s.DeserializeVector<bool>(s.Serialize(single));
    REQUIRE(rs.has_value());
    REQUIRE(rs->size() == 1);
    REQUIRE((*rs)[0] == true);

    std::vector<bool> allFalse = {false, false, false};
    auto raf = s.DeserializeVector<bool>(s.Serialize(allFalse));
    REQUIRE(raf.has_value());
    REQUIRE(raf->size() == 3);
    REQUIRE((*raf)[0] == false);
    REQUIRE((*raf)[1] == false);
    REQUIRE((*raf)[2] == false);
}

// ================================================================
//  25. const char* overload routes to string
// ================================================================
TEST_CASE("const char* overload serializes as string", "[string][const-char]") {
    std::string data = s.Serialize("hello");
    REQUIRE(data.substr(0, 2) == "s:");
    REQUIRE(s.DeserializeString(data).value_or("") == "hello");

    auto empty = s.DeserializeString(s.Serialize(""));
    REQUIRE(empty.has_value());
    REQUIRE(empty->empty());

    REQUIRE(s.DeserializeString(s.Serialize("a:b;c")).value_or("") == "a:b;c");
    REQUIRE(s.Serialize("true").substr(0, 2) == "s:");
}

// ================================================================
//  26. serialize → deserialize → serialize idempotency
// ================================================================
TEST_CASE("serialize/deserialize idempotency", "[idempotent]") {
    auto s1 = s.Serialize(42);
    REQUIRE(s.Serialize(*s.DeserializeInt(s1)) == s1);

    auto s2 = s.Serialize(3.14159265358979323);
    REQUIRE(s.Serialize(*s.DeserializeDouble(s2)) == s2);

    auto s3 = s.Serialize(true);
    REQUIRE(s.Serialize(*s.DeserializeBool(s3)) == s3);

    auto s4 = s.Serialize('Z');
    REQUIRE(s.Serialize(*s.DeserializeChar(s4)) == s4);

    auto s5 = s.Serialize(std::string("hello;world:!"));
    REQUIRE(s.Serialize(*s.DeserializeString(s5)) == s5);

    auto s6 = s.Serialize(std::vector<int>{1, 2, 3});
    REQUIRE(s.Serialize(*s.DeserializeVector<int>(s6)) == s6);

    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    auto s7 = s.Serialize(m);
    REQUIRE(s.Serialize(*s.DeserializeMap<std::string, int>(s7)) == s7);

    std::vector<std::vector<int>> nested = {{1, 2}, {3}};
    auto s8 = s.Serialize(nested);
    REQUIRE(s.Serialize(*s.DeserializeVector<std::vector<int>>(s8)) == s8);
}

// ================================================================
//  27. double precision extremes
// ================================================================
TEST_CASE("double precision extremes", "[double][precision]") {
    double denorm = std::numeric_limits<double>::denorm_min();
    REQUIRE(s.DeserializeDouble(s.Serialize(denorm)).value_or(0.0) == denorm);

    double eps = std::numeric_limits<double>::epsilon();
    REQUIRE(s.DeserializeDouble(s.Serialize(eps)).value_or(0.0) == eps);

    REQUIRE(s.DeserializeDouble(s.Serialize(0.1)).value_or(0.0) == 0.1);

    double a = 1.0, b = std::nextafter(1.0, 2.0);
    REQUIRE(s.Serialize(a) != s.Serialize(b));
    REQUIRE(s.DeserializeDouble(s.Serialize(a)).value_or(0.0) == a);
    REQUIRE(s.DeserializeDouble(s.Serialize(b)).value_or(0.0) == b);

    REQUIRE(s.DeserializeDouble(s.Serialize(std::numeric_limits<double>::max())).value_or(0.0)
            == std::numeric_limits<double>::max());
}

// ================================================================
//  28. deeply nested containers (5 levels)
// ================================================================
TEST_CASE("deeply nested containers", "[nested][deep]") {
    SECTION("5-level vector") {
        std::vector<std::vector<std::vector<std::vector<std::vector<int>>>>> deep =
            {{{{{1, 2}, {3}}}, {{{4}}}}, {{{{5, 6, 7}}}}};
        auto r = s.DeserializeVector<std::vector<std::vector<std::vector<std::vector<int>>>>>(s.Serialize(deep));
        REQUIRE(r.has_value());
        REQUIRE(*r == deep);
    }

    SECTION("mixed map/vector nesting") {
        std::map<std::string, std::vector<std::map<std::string, std::vector<int>>>> mixed;
        mixed["group1"] = {{{"nums", {1, 2, 3}}, {"more", {4, 5}}}};
        mixed["group2"] = {{{"single", {99}}}};
        auto r = s.DeserializeMap<std::string, std::vector<std::map<std::string, std::vector<int>>>>(s.Serialize(mixed));
        REQUIRE(r.has_value());
        REQUIRE(*r == mixed);
    }

    SECTION("empty at every level") {
        std::vector<std::vector<std::vector<int>>> emptyDeep = {{{}}, {}, {{}, {}}};
        auto r = s.DeserializeVector<std::vector<std::vector<int>>>(s.Serialize(emptyDeep));
        REQUIRE(r.has_value());
        REQUIRE(*r == emptyDeep);
    }

    SECTION("single element at 4 levels") {
        std::vector<std::vector<std::vector<std::vector<int>>>> single4 = {{{{42}}}};
        auto r = s.DeserializeVector<std::vector<std::vector<std::vector<int>>>>(s.Serialize(single4));
        REQUIRE(r.has_value());
        REQUIRE(*r == single4);
    }
}

// ================================================================
//  29. custom type_id special characters
// ================================================================
TEST_CASE("custom type_id special characters", "[custom][type_id]") {
    s.RegisterType<int>("my:type",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> { return s.DeserializeInt(data); }
    );
    REQUIRE(s.Deserialize<int>("my:type", s.Serialize<int>("my:type", 42)).value_or(-1) == 42);

    s.RegisterType<int>("semi;type",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> { return s.DeserializeInt(data); }
    );
    REQUIRE(s.Deserialize<int>("semi;type", s.Serialize<int>("semi;type", 7)).value_or(-1) == 7);

    s.RegisterType<int>("",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> { return s.DeserializeInt(data); }
    );
    REQUIRE(s.Deserialize<int>("", s.Serialize<int>("", 99)).value_or(-1) == 99);

    s.RegisterType<int>("custom",
        [](const int& v) -> std::string { return s.Serialize(v); },
        [](const std::string& data) -> std::optional<int> { return s.DeserializeInt(data); }
    );
    REQUIRE(s.Deserialize<int>("custom", s.Serialize<int>("custom", 77)).value_or(-1) == 77);

    CHECK(s.IsTypeRegistered("my:type"));
    CHECK(s.IsTypeRegistered("semi;type"));
    CHECK(s.IsTypeRegistered(""));
    CHECK(s.IsTypeRegistered("custom"));
}

// ================================================================
//  30. strings with embedded null bytes
// ================================================================
TEST_CASE("strings with embedded null bytes", "[string][null]") {
    std::string withNull("hel\0lo", 6);
    REQUIRE(withNull.size() == 6);
    auto r = s.DeserializeString(s.Serialize(withNull));
    REQUIRE(r.has_value());
    REQUIRE(r->size() == 6);
    REQUIRE(*r == withNull);

    std::string justNull(1, '\0');
    auto r2 = s.DeserializeString(s.Serialize(justNull));
    REQUIRE(r2.has_value());
    REQUIRE(r2->size() == 1);
    REQUIRE((*r2)[0] == '\0');

    std::string multiNull(5, '\0');
    auto r3 = s.DeserializeString(s.Serialize(multiNull));
    REQUIRE(r3.has_value());
    REQUIRE(r3->size() == 5);
    REQUIRE(*r3 == multiNull);
}

// ================================================================
//  31. map key ordering preservation
// ================================================================
TEST_CASE("map key ordering preserved through round-trip", "[map][ordering]") {
    std::map<std::string, int> m = {{"cherry", 3}, {"apple", 1}, {"banana", 2}};
    auto r = s.DeserializeMap<std::string, int>(s.Serialize(m));
    REQUIRE(r.has_value());
    REQUIRE(*r == m);

    auto it = r->begin();
    REQUIRE(it->first == "apple");
    ++it;
    REQUIRE(it->first == "banana");
}

// ================================================================
//  32. char special values
// ================================================================
TEST_CASE("char special values", "[char][special]") {
    REQUIRE(s.DeserializeChar(s.Serialize('\0')).value_or('X')              == '\0');
    REQUIRE(s.DeserializeChar(s.Serialize(static_cast<char>(0xFF))).value_or('\0')
            == static_cast<char>(0xFF));
    REQUIRE(s.DeserializeChar(s.Serialize('\x7F')).value_or('\0')           == '\x7F');
    REQUIRE(s.DeserializeChar(s.Serialize('\x07')).value_or('\0')           == '\x07');
    REQUIRE(s.DeserializeChar(s.Serialize('\x01')).value_or('\0')           == '\x01');
}

// ================================================================
//  33. stoul negative wrapping — bug-revealing test
// ================================================================
TEST_CASE("stoul negative wrapping does not crash", "[vector][map][bug]") {
    SECTION("vector negative count") {
        REQUIRE_NOTHROW([&]{ CHECK_FALSE(s.DeserializeVector<int>("v:-1:i:1;").has_value()); }());
    }

    SECTION("map negative count") {
        REQUIRE_NOTHROW([&]{ CHECK_FALSE(s.DeserializeMap<std::string, int>("m:-1:s:1:a;i:1;").has_value()); }());
    }

    SECTION("vector huge count") {
        REQUIRE_NOTHROW([&]{ CHECK_FALSE(s.DeserializeVector<int>("v:999999999999999999:i:1;").has_value()); }());
    }

    SECTION("map huge count") {
        REQUIRE_NOTHROW([&]{ CHECK_FALSE(s.DeserializeMap<int, int>("m:999999999999999999:i:1;i:2;").has_value()); }());
    }

    SECTION("string huge length") {
        REQUIRE_NOTHROW([&]{ CHECK_FALSE(s.DeserializeString("s:18446744073709551615:x;").has_value()); }());
    }

    SECTION("string negative length") {
        REQUIRE_NOTHROW([&]{ CHECK_FALSE(s.DeserializeString("s:-5:hello;").has_value()); }());
    }
}

// ================================================================
//  34. double round-trip precision (locale-safe)
// ================================================================
TEST_CASE("double round-trip precision locale-safe", "[double][precision][locale]") {
    double values[] = {3.14159265358979323, 1.0/3.0, 1e-15, -2.718281828459045, 0.1};
    for (double v : values) {
        auto r = s.DeserializeDouble(s.Serialize(v));
        REQUIRE(r.has_value());
        REQUIRE(*r == v);
    }
}

// ================================================================
//  35. float round-trip
// ================================================================
TEST_CASE("float round-trip", "[float]") {
    SECTION("basic values") {
        float values[] = {3.14f, -1.0f, 0.0f, 1e10f, 1e-10f};
        for (float v : values) {
            auto r = s.DeserializeFloat(s.Serialize(v));
            REQUIRE(r.has_value());
            REQUIRE(*r == v);
        }
    }

    SECTION("via DeserializeAt") {
        std::string data = s.Serialize(3.14f);
        size_t pos = 0;
        auto r = s.DeserializeAt<float>(data, pos);
        REQUIRE(r.has_value());
        REQUIRE(*r == 3.14f);
    }
}

// ================================================================
//  36. integer type round-trips (long, unsigned int, long long,
//      unsigned long long, size_t)
// ================================================================
TEST_CASE("extended integer round-trips", "[int64]") {
    SECTION("long long") {
        long long values[] = {0LL, 1LL, -1LL, 42LL, -42LL};
        for (long long v : values) {
            auto r = s.DeserializeLongLong(s.Serialize(v));
            REQUIRE(r.has_value());
            REQUIRE(*r == v);
        }
    }

    SECTION("unsigned long long") {
        unsigned long long values[] = {0ULL, 1ULL, 42ULL, 1000000ULL};
        for (unsigned long long v : values) {
            auto r = s.DeserializeUnsignedLongLong(s.Serialize(v));
            REQUIRE(r.has_value());
            REQUIRE(*r == v);
        }
    }

    SECTION("long via DeserializeAt") {
        long val = 12345L;
        std::string data = s.Serialize(val);
        size_t pos = 0;
        auto r = s.DeserializeAt<long>(data, pos);
        REQUIRE(r.has_value());
        REQUIRE(*r == val);
    }

    SECTION("unsigned int via DeserializeAt") {
        unsigned int val = 12345u;
        std::string data = s.Serialize(val);
        size_t pos = 0;
        auto r = s.DeserializeAt<unsigned int>(data, pos);
        REQUIRE(r.has_value());
        REQUIRE(*r == val);
    }

    SECTION("size_t via DeserializeAt") {
        size_t val = 99999;
        std::string data = s.Serialize(val);
        size_t pos = 0;
        auto r = s.DeserializeAt<size_t>(data, pos);
        REQUIRE(r.has_value());
        REQUIRE(*r == val);
    }
}

// ================================================================
//  37. 64-bit boundary values
// ================================================================
TEST_CASE("64-bit boundary values", "[int64][boundary]") {
    SECTION("values beyond INT_MAX") {
        long long big = static_cast<long long>(INT_MAX) + 1;
        auto r = s.DeserializeLongLong(s.Serialize(big));
        REQUIRE(r.has_value());
        REQUIRE(*r == big);

        long long bigger = 1LL << 40;
        auto r2 = s.DeserializeLongLong(s.Serialize(bigger));
        REQUIRE(r2.has_value());
        REQUIRE(*r2 == bigger);
    }

    SECTION("LLONG_MAX and LLONG_MIN") {
        auto rmax = s.DeserializeLongLong(s.Serialize(LLONG_MAX));
        REQUIRE(rmax.has_value());
        REQUIRE(*rmax == LLONG_MAX);

        auto rmin = s.DeserializeLongLong(s.Serialize(LLONG_MIN));
        REQUIRE(rmin.has_value());
        REQUIRE(*rmin == LLONG_MIN);
    }

    SECTION("ULLONG_MAX") {
        auto r = s.DeserializeUnsignedLongLong(s.Serialize(ULLONG_MAX));
        REQUIRE(r.has_value());
        REQUIRE(*r == ULLONG_MAX);
    }
}

// ================================================================
//  38. negative 64-bit values
// ================================================================
TEST_CASE("negative 64-bit values", "[int64][negative]") {
    long long negvals[] = {-1LL, -1000000000000LL, LLONG_MIN};
    for (long long v : negvals) {
        auto r = s.DeserializeLongLong(s.Serialize(v));
        REQUIRE(r.has_value());
        REQUIRE(*r == v);
    }

    // unsigned long long rejects negative (from_chars for unsigned fails)
    std::string neg_data = s.Serialize(-1LL);
    CHECK_FALSE(s.DeserializeUnsignedLongLong(neg_data).has_value());
}

// ================================================================
//  39. DeserializeAt chain after failed parse
// ================================================================
TEST_CASE("DeserializeAt chain after failed parse", "[deserializeAt][pos]") {
    SECTION("tag mismatch does not corrupt pos") {
        std::string data = s.Serialize(3.14) + s.Serialize(42);
        size_t pos = 0;

        // Try wrong type: tag 'd' doesn't match 'i'
        auto bad = s.DeserializeAt<int>(data, pos);
        CHECK_FALSE(bad.has_value());
        REQUIRE(pos == 0);  // pos unchanged on tag mismatch

        // Correct parse chain still works
        auto rd = s.DeserializeAt<double>(data, pos);
        REQUIRE(rd.has_value());
        REQUIRE(*rd == 3.14);

        auto ri = s.DeserializeAt<int>(data, pos);
        REQUIRE(ri.has_value());
        REQUIRE(*ri == 42);
    }

    SECTION("value parse failure does not advance pos past semicolon") {
        // An int value that overflows: tag matches, value parse fails
        std::string data = "i:99999999999999;i:42;";
        size_t pos = 0;

        auto bad = s.DeserializeAt<int>(data, pos);
        CHECK_FALSE(bad.has_value());
        // pos should NOT have been advanced past the semicolon (to 17)
        // it may be at TAG_PREFIX_LEN (2) but not at semi+1 (17)
        REQUIRE(pos != 17);
    }
}