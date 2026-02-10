#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../source/StringDiff.hpp"

using sim::StringDiff;



// ************************
// Test Cases for MakeDiff
// ************************

//makes sure that diffing a string against itself produces no replacement and the full string is treated as prefix
TEST_CASE("MakeDiff: identical strings produce empty replacement", "[StringDiff][MakeDiff]") {
    std::string s = "Hello World";
    auto patch = StringDiff::MakeDiff(s, s);

    REQUIRE(patch.prefix_length == s.length()); //makes sure entire string is considered prefix
    REQUIRE(patch.suffix_length == 0);          //makes sure no suffix
    REQUIRE(patch.replacement.empty());         //makes sure replacement is blank
    REQUIRE(patch.base_length == s.length());   //makes sure the base is just the entire string
}

//makes sure that diffing two empty strings makes empty patch
TEST_CASE("MakeDiff: both strings empty", "[StringDiff][MakeDiff]") {
    auto patch = StringDiff::MakeDiff("", "");

    REQUIRE(patch.prefix_length == 0);  //makes sure prefix has length of 0
    REQUIRE(patch.suffix_length == 0);  //makes sure suffix has length 0
    REQUIRE(patch.replacement.empty()); //makes sure replacement empty
    REQUIRE(patch.base_length == 0);    //no base to have a length of
}

//makes sure going from nothing to a string stores entire new string as replacement
TEST_CASE("MakeDiff: empty to new string", "[StringDiff][MakeDiff]") {
    std::string s = "Hello World";
    auto patch = StringDiff::MakeDiff("", s);
    
    REQUIRE(patch.prefix_length == 0);  //prefix should be empty 
    REQUIRE(patch.suffix_length == 0);  //suffix should be empty
    REQUIRE(patch.replacement == s);    //the replacement should be the entire string
    REQUIRE(patch.base_length == 0);    //should have no base
}

//makes sure going from a string to nothing makes no replacement & no suffix/prefix
TEST_CASE("MakeDiff: string to empty", "[StringDiff][MakeDiff]") {
    std::string s = "Hello World";
    auto patch = StringDiff::MakeDiff(s, "");
    
    REQUIRE(patch.prefix_length == 0);          //prefix should be empty 
    REQUIRE(patch.suffix_length == 0);          //suffix should be empty
    REQUIRE(patch.replacement.empty());         //should have no replacement
    REQUIRE(patch.base_length == s.length());   //base should be the string
}




// ************************
// Test Cases for ApplyDiff
// ************************

//tests making a diff then applying it gives the same updated string
TEST_CASE("ApplyDiff: Reconstructs updated string", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());            //makes sure something even gets returned
    REQUIRE(result.value() == updated);     //makes sure the resulted patch actually returns updated
}

//confirms applying a patch with no changes gives the same string and actually does no change
TEST_CASE("ApplyDiff: No change patch makes no change", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";
    auto patch = StringDiff::MakeDiff(base, base);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());    //makes sure something gets returned
    REQUIRE(result.value() == base);   //makes sure returned the same string
}

//tests applying diff with empty base has result of new non empty string
TEST_CASE("ApplyDiff: Complete diff apply with empty base", "[StringDiff][ApplyDiff]") {
    std::string base = "";
    std::string updated = "Hello World";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());            //makes sure something gets returned
    REQUIRE(result.value() == updated);     //makes sure result from empty is entire updated
}

//tests applying diff with empty updated string and has result of empty
TEST_CASE("ApplyDiff: Complete diff apply with empty updated", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";
    std::string updated = "";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());            //makes sure something gets returned
    REQUIRE(result.value() == updated);     //makes sure result is the new empty updated
}

//full test of both empty, should have value just empty
TEST_CASE("ApplyDiff: Complete diff apply with both empty", "[StringDiff][ApplyDiff]") {
    std::string base = "";
    std::string updated = "";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());        //should still have value
    REQUIRE(result.value().empty());    //empty result
}
