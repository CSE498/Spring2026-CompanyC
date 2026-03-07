#include "catch2/catch.hpp"

#include "../../source/tools/AnnotationSet.hpp"
#include <algorithm>

using cse498::AnnotationSet;

TEST_CASE("BaseFunctionality-AnnotationSet","[AnnotationSet]"){
    int objId=1;
    AnnotationSet annotationSet(objId);
    SECTION("New set is empty"){
        std::set<std::string> vals=annotationSet.GetTags();
        REQUIRE(vals.empty());
        REQUIRE(annotationSet.Size()==0);
    }

    annotationSet.AddTag("Dog");

    SECTION("New set stores proper info"){
        REQUIRE(annotationSet.GetObjId()==1);
        std::set<std::string> rightVal={"Dog"};
        REQUIRE(annotationSet.GetTags()==rightVal);
    }
    SECTION("New set deletes info"){
        annotationSet.DeleteAllTags();
        REQUIRE(annotationSet.Size()==0);
        std::set<std::string> vals=annotationSet.GetTags();
        REQUIRE(vals.empty());
    }
    AnnotationSet annotationSet1(objId, {"cat"});
    SECTION("New set has initialized tags"){
        std::set<std::string> vals=annotationSet1.GetTags();
        std::set<std::string> rightVal={"cat"};
        REQUIRE(vals==rightVal);
        REQUIRE(annotationSet1.Size()==1);
    }
}

TEST_CASE("AddingTags-AnnotationSet","[AnnotationSet]"){
    int objId=2;
    AnnotationSet annotationSet(objId);
    SECTION("Adding one tag"){
        annotationSet.AddTag("happy");
        REQUIRE(annotationSet.Size()==1);
    }
    SECTION("Adding multiple tags at once"){
        annotationSet.AddTags({"cat","dog","raccoon"});
        REQUIRE(annotationSet.Size()==3);
    }
    SECTION("No duplicate tags in set when added"){
        annotationSet.AddTags({"cat","cat","raccoon"});
        REQUIRE(annotationSet.Size()==2);
    }
    SECTION("No duplicate tags in set when added to set with the tag"){
        annotationSet.AddTags({"cat","raccoon"});
        REQUIRE(annotationSet.Size()==2);
        annotationSet.AddTag("cat");
        REQUIRE(annotationSet.Size()==2);
    }
    SECTION("Cant add empty tags"){
        annotationSet.AddTag("");
        REQUIRE(annotationSet.Size()==0);
        annotationSet.AddTag("  ");
        REQUIRE(annotationSet.Size()==0);
    }
    SECTION("Adding tags with white space"){
        annotationSet.AddTag(" Superman");
        annotationSet.AddTag("Superman ");
        annotationSet.AddTag("Super man");
        REQUIRE(annotationSet.Size()==1);
    }
}

TEST_CASE("RemovingTags-AnnotationSet","[AnnotationSet]"){
    int objId=2;
    std::set<std::string> tags ={"ghost","spectre","boogieman"};
    AnnotationSet annotationSet(objId, tags);
    SECTION("Deleting one tag"){
        annotationSet.RemoveTag("ghost");
        std::set<std::string> vals={"spectre","boogieman"};
        REQUIRE(annotationSet.GetTags()==vals);
        REQUIRE(annotationSet.Size()==2);
    }
    SECTION("Deleting multiple tags"){
        annotationSet.RemoveTags({"ghost", "boogieman"});
        std::set<std::string> vals={"spectre"};
        REQUIRE(annotationSet.GetTags()==vals);
        REQUIRE(annotationSet.Size()==1);
    }
    SECTION("Deleting all tags"){
        annotationSet.DeleteAllTags();
        REQUIRE(annotationSet.GetTags().empty());
    }
    SECTION("Deleting whitespace tags"){
        annotationSet.DeleteAllTags();
        annotationSet.AddTag("Batman");
        annotationSet.RemoveTag("Bat man");
        REQUIRE(annotationSet.GetTags().empty()==true);
        annotationSet.AddTag("Batman");
        annotationSet.RemoveTag(" Batman");
        REQUIRE(annotationSet.GetTags().empty()==true);
        annotationSet.AddTag("Batman");
        annotationSet.RemoveTag("Batman ");
        REQUIRE(annotationSet.GetTags().empty()==true);
    }
}
TEST_CASE("TagCheck-AnnotationSet","[AnnotationSet]"){
    int objId=3;
    std::set<std::string> tags={"meow","ruff","blub"};
    AnnotationSet annotationSet(objId,tags);
    SECTION("Finding tags base case"){
        std::vector<std::string> findTags={"meow","ruff"};
        REQUIRE(annotationSet.FindAllTags(findTags)==true);
        findTags={"roar"};
        REQUIRE(annotationSet.FindAllTags(findTags)==false);
        findTags={"ssssssss","meow"};
        REQUIRE(annotationSet.FindAnyTag(findTags)==true);
        findTags={"roar","ssssssss"};
        REQUIRE(annotationSet.FindAnyTag(findTags)==false);
        std::string findTag="meow";
        REQUIRE(annotationSet.FindTag(findTag)==true);
        findTag="sssssss";
        REQUIRE(annotationSet.FindTag(findTag)==false);
    }
    SECTION("Partial text match"){
        std::vector<std::string> findTags={"meo","ru","r","blu"};
        REQUIRE(annotationSet.FindAllTags(findTags)==false);
        findTags={"meo","ru","r","blu"};
        REQUIRE(annotationSet.FindAnyTag(findTags)==false);
        std::string findTag="meo";
        REQUIRE(annotationSet.FindTag(findTag)==false);
    }
    SECTION("Finding tag with whitespace in search term"){
        std::vector<std::string> findTags={"meow ","ru ff","ssssssss"};
        REQUIRE(annotationSet.FindAnyTag(findTags)==true);
        findTags={"meow ","ru ff"};
        REQUIRE(annotationSet.FindAllTags(findTags)==true);
    }
}
