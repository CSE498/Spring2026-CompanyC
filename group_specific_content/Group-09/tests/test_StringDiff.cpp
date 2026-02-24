#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../source/StringDiff.hpp"

using sim::StringDiff;
using sim::DiffError;



// ************************************************************************
// Test Cases for MakeDiff
// ************************************************************************

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




// ************************************************************************
// Test Cases for ApplyDiff
// ************************************************************************

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

//tests to make sure properly fails with wrong base inputed 
TEST_CASE("ApplyDiff: wrong base string returns BaseHashMismatch", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";
    std::string wrong_base = "Hello Borld";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(wrong_base, patch);

    REQUIRE_FALSE(result.has_value());                      //makes sure no output
    REQUIRE(result.error() == DiffError::BaseHashMismatch); //hash mismatch because content differs
}

//tests different base length error is properly handled
TEST_CASE("ApplyDiff: wrong base length returns BaseLengthMismatch", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";
    std::string shorter_base = "Hell rld";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(shorter_base, patch);

    REQUIRE_FALSE(result.has_value());                          //makes sure no output
    REQUIRE(result.error() == DiffError::BaseLengthMismatch);   //length differs from patch expectation
}


//manual invalid patch where prefix + suffix > base_length should return InvalidPatchInvariant
TEST_CASE("ApplyDiff: prefix + suffix > base_length returns InvalidPatchInvariant", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";

    //start from a valid patch so hash and length pass verification
    auto invalid_patch = StringDiff::MakeDiff(base, base);
    invalid_patch.prefix_length = 12;
    invalid_patch.suffix_length = 12;
    invalid_patch.replacement = "c";

    auto result = StringDiff::ApplyDiff(base, invalid_patch);

    REQUIRE_FALSE(result.has_value());                              //should fail
    REQUIRE(result.error() == DiffError::InvalidPatchInvariant);    //prefix + suffix > base_length
}


//test complete ApplyDiff flow with a complete string replacement
TEST_CASE("ApplyDiff: complete flow, complete string replacement", "[StringDiff][ApplyDiff]") {
    std::string base = "abc";
    std::string updated = "xyz";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}


//complete ApplyDiff flow adding string
TEST_CASE("ApplyDiff: complete flow, append string", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello";
    std::string updated = "Hello World";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}

//complete ApplyDiff flow removing string
TEST_CASE("ApplyDiff: complete flow, deleting string", "[StringDiff][ApplyDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto result = StringDiff::ApplyDiff(base, patch);

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}


// ************************************************************************
// Test Cases for EncodeDiff
// ************************************************************************

//makes sure encode diff properly returns a format that has 5 '|' separators
TEST_CASE("EncodeDiff: Verify 5 seperators", "[StringDiff][EncodeDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";
    
    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());
    std::string encoded = encoded_result.value();

    int sep_count = 0;
    for (char c : encoded) {
        if (c == '|') {
            sep_count++;
        }
    }
    REQUIRE(sep_count == 5);
}

//makes sure the actual encoded text has the correct replacement text at the end
TEST_CASE("EncodeDiff: Verify encode ends with replacement text", "[StringDiff][EncodeDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";
    
    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    
    REQUIRE(encoded_result.has_value());
    std::string encoded = encoded_result.value();

    //should end in 'C++ '
    REQUIRE(encoded.size() >= 4);
    //std::cout << "Encoded: " << encoded.substr(encoded.size() - 3) << std::endl;
    REQUIRE(encoded.substr(encoded.size() - 4) == "C++ ");
}

//full encoding full with manual build confirmation
TEST_CASE("EncodeDiff: Manual build comparison", "[StringDiff][EncodeDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";
    
    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());
    
    std::string encoded = encoded_result.value();

    std::ostringstream oss;
    oss << patch.base_hash << '|';
    oss << base.size() << '|';
    oss << 6 << '|';    // prefix length 6 "Hello "
    oss << 5 << '|';    // suffix length 5 "World"
    oss << 4 << '|';    // replacement len 4 "C++ "
    oss << "C++ ";       // replacement is "C++ "

    REQUIRE(encoded == oss.str());
}


// ************************************************************************
// Test Cases for DecodeDiff
// ************************************************************************

//test full decode flow with a valid encoding
TEST_CASE("DecodeDiff: Full flow, encode then decode", "[StringDiff][DecodeDiff]") {
    std::string base = "Hello World";
    std::string updated = "Hello C++ World";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());
    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == patch);
}


//test full decode flow with no replacement text
TEST_CASE("DecodeDiff: Full flow, no replacement", "[StringDiff][DecodeDiff]") {
    std::string base = "Hello C++ World";
    std::string updated = "Hello World";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());
    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == patch);
}


//test full decode flow with no base
TEST_CASE("DecodeDiff: Full flow, empty base", "[StringDiff][DecodeDiff]") {
    std::string base = "";
    std::string updated = "Hello World";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());
    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());
    REQUIRE(decoded.value() == patch);
}


//decode fail test, should fail on empty encoded input 
TEST_CASE("DecodeDiff: empty encoded string returns MalformedEncoding", "[StringDiff][DecodeDiff]") {
    auto decoded = StringDiff::DecodeDiff("");

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::MalformedEncoding);
}

//decode fail test, missing separators
TEST_CASE("DecodeDiff: no separators returns MalformedEncoding", "[StringDiff][DecodeDiff]") {
    std::string encoded = "Hello_World";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::MalformedEncoding);
}

//decode fail test, missing fields & seps
TEST_CASE("DecodeDiff: Not enough separators returns MalformedEncoding", "[StringDiff][DecodeDiff]") {
    std::string encoded = "420|21|9";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::MalformedEncoding);
}

//decode fail test, bad hash 
TEST_CASE("DecodeDiff: Bad hash input returns InvalidNumberField", "[StringDiff][DecodeDiff]") {
    std::string encoded = "abadhash|11|6|5|4|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//decode fail test, bad base_length 
TEST_CASE("DecodeDiff: Bad base_length input returns InvalidNumberField", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|abc|6|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//decode fail test, bad prefix length
TEST_CASE("DecodeDiff: Bad prefix_length input returns InvalidNumberField", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|11|xyz|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//decode fail test, bad suffix length
TEST_CASE("DecodeDiff: Bad suffix_length input returns InvalidNumberField", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|11|6|xyz|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//decode fail test, bad rep_length 
TEST_CASE("DecodeDiff: Bad rep_length input returns InvalidNumberField", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|11|6|5|xyz|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//decode fail test, random trailing stuff after replacement
TEST_CASE("DecodeDiff: Trailing garbage returns ReplacementLengthMismatch", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|11|6|5|3|C++EXTRA";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::ReplacementLengthMismatch);
}

//decode fail test, prefix + suffix > base_length
TEST_CASE("DecodeDiff: prefix + suffix > base_length returns InvalidPatchInvariant", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|5|4|4|1|x";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidPatchInvariant);
}

//overflow test
TEST_CASE("DecodeDiff: prefix + suffix uint64 overflow returns InvalidPatchInvariant", "[StringDiff][DecodeDiff]") {
    std::string encoded = "123|18446744073709551615|18446744073709551615|1|0|";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidPatchInvariant);
}


// ************************************************************************
// tests to make sure proper uint64_t parsing
// ************************************************************************

//trailing letters should be rejected - ex: "123abc" as hash
TEST_CASE("DecodeDiff: hash with suffix junk returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123abc|11|6|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//trailing letters in base_length field
TEST_CASE("DecodeDiff: base_length with suffix junk returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123|11x|6|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//trailing letters in prefix_length field
TEST_CASE("DecodeDiff: prefix_length with suffix junk returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123|11|6z|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//trailing letters in suffix_length field
TEST_CASE("DecodeDiff: suffix_length with suffix junk returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123|11|6|5q|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//trailing letters in rep_length field
TEST_CASE("DecodeDiff: rep_length with suffix junk returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123|11|6|5|3r|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//empty between separators should be rejected
TEST_CASE("DecodeDiff: empty hash token returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "|11|6|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//empty base_length between separators should be rejected
TEST_CASE("DecodeDiff: empty base_length token returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123||6|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}

//negative value should be rejected
TEST_CASE("DecodeDiff: negative number returns InvalidNumberField", "[StringDiff][DecodeDiff][Strict]") {
    std::string encoded = "123|11|-1|5|3|C++";
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE_FALSE(decoded.has_value());
    REQUIRE(decoded.error() == DiffError::InvalidNumberField);
}


// ************************************************************************
// COMPLETE FLOW TESTS
// MakeDiff -> EncodeDiff -> DecodeDiff -> ApplyDiff
// ************************************************************************

TEST_CASE("Complete: make, encode, decode, apply reconstructs string", "[StringDiff][Complete]") {
    std::string base = "The quick brown fox";
    std::string updated = "The quick red fox";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());

    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());

    auto result = StringDiff::ApplyDiff(base, decoded.value());

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}


TEST_CASE("Complete: empty to non-empty", "[StringDiff][Complete]") {
    std::string base = "";
    std::string updated = "The quick red fox";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());
    
    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());

    auto result = StringDiff::ApplyDiff(base, decoded.value());

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}

TEST_CASE("Complete: non-empty to empty", "[StringDiff][Complete]") {
    std::string base = "Hello world";
    std::string updated = "";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());

    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());

    auto result = StringDiff::ApplyDiff(base, decoded.value());

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}

TEST_CASE("Complete: long string with small difference", "[StringDiff][Complete]") {
    std::string base(1000, 'A');
    std::string updated = base;
    updated[500] = 'B';

    auto patch = StringDiff::MakeDiff(base, updated);

    //patch should be small because was only a small change
    REQUIRE(patch.replacement.size() == 1);
    REQUIRE(patch.replacement == "B");
    REQUIRE(patch.prefix_length == 500);
    REQUIRE(patch.suffix_length == 499);

    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());

    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());

    auto result = StringDiff::ApplyDiff(base, decoded.value());

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}

TEST_CASE("Complete: string has special chars", "[StringDiff][Complete]") {
    std::string base = "line1\nline2\ttab";
    std::string updated = "line1\nmodified\ttab";

    auto patch = StringDiff::MakeDiff(base, updated);
    auto encoded_result = StringDiff::EncodeDiff(patch);
    REQUIRE(encoded_result.has_value());

    std::string encoded = encoded_result.value();
    auto decoded = StringDiff::DecodeDiff(encoded);

    REQUIRE(decoded.has_value());

    auto result = StringDiff::ApplyDiff(base, decoded.value());

    REQUIRE(result.has_value());
    REQUIRE(result.value() == updated);
}





// equality operator tests
TEST_CASE("Diff equality: identical diffs are equal", "[StringDiff][Diff]") {
    auto d1 = StringDiff::MakeDiff("Hello World", "Hello C++ World");
    auto d2 = StringDiff::MakeDiff("Hello World", "Hello C++ World");

    REQUIRE(d1 == d2);
}

TEST_CASE("Diff equality: different diffs are not equal", "[StringDiff][Diff]") {
    auto d1 = StringDiff::MakeDiff("Hello World", "Hello C++ World");
    auto d2 = StringDiff::MakeDiff("Hello World", "Hello Java World");

    REQUIRE_FALSE(d1 == d2);
}
