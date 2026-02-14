#define CATCH_CONFIG_MAIN
#include "../third-party/Catch/single_include/catch2/catch.hpp"

#include "../source/core/AnnotationSet.hpp"
#include <algorithm>

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
        REQUIRE(annotationSet.Size()==1);
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
}
