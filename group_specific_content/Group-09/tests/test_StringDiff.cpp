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







