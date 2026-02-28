//By Sachin Karatha
//
//This class stores tags that are attached to a given object with a unique Id.
//
//Class includes methods to add and remove tags using vectors or strings depending
//on the amount being added. Tags can also be checked for membership whether it be
//only one or multiple. 
//Integration between TagManager made by Shashank Papani with help of chatgpt

#include "AnnotationSet.hpp"
#include "TagManager.hpp"
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>

namespace cse498 {

//new constructor initializes object Id and tags with a default value of {}
AnnotationSet::AnnotationSet(int obj, std::set<std::string> inTags){
    objectId = obj;
    tags = inTags;
}


//adds a tag to the tags
void AnnotationSet::AddTag(std::string tag){
    std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(tag);
    if (!newTagPair.first){
        tags.insert(newTagPair.second);
        if(tagManager){
            tagManager->AddTag(static_cast<TagManager::ObjectId>(objectId),newTagPair.second);
        }
    }
}

//adds tags to the set of tags
void AnnotationSet::AddTags(const std::vector<std::string>& addedTags) {
    for (const auto& rawTag : addedTags) {
        std::string tag = rawTag;
        std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(tag);
        if (!newTagPair.first){
            tags.insert(newTagPair.second);
            if(tagManager){
                tagManager->AddTag(static_cast<TagManager::ObjectId>(objectId),newTagPair.second);
            }
        }
    }
}


//removes tags from the set of tags
//returns true if all were successfull false otherwise
bool AnnotationSet::RemoveTags(const std::vector<std::string>& removedTags){
    bool allRemoved = true;

    for (const auto& tag : removedTags) {
        std::string newTag=tag;
        std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(newTag);
        if (tags.erase(newTagPair.second) == 0) {
            allRemoved = false;
        }
        else{
            if(tagManager){
                tagManager->RemoveTag(static_cast<TagManager::ObjectId>(objectId),newTagPair.second);
            }
        }
    }

    return allRemoved;
}

//removes a tag from the set of tags
//returns true if successfull false otherwise
bool AnnotationSet::RemoveTag(const std::string& tag){
    std::string newTag=tag;
    std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(newTag);
    if (tags.erase(newTagPair.second) == 0) {
        return false;
    }
    else{
        if(tagManager){
            tagManager->RemoveTag(static_cast<TagManager::ObjectId>(objectId),newTagPair.second);
        }
        return true;
    }
}

//checks if a certain tag is attached to object
//returns true if successfull false otherwise
bool AnnotationSet::FindTag(const std::string& tag){
    std::string newTag=tag;
    std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(newTag);
    if(tags.count(newTagPair.second)==1){
        return true;
    }
    else{
        return false;
    }
}

//checks if any of a given set of tags are attached to an object
//returns true if successfull false otherwise
bool AnnotationSet::FindAnyTag(const std::vector<std::string>& searchTags){
    for (const auto& tag : searchTags) {
        std::string newTag=tag;
        std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(newTag);
        if (tags.count(newTagPair.second)==1) {
            return true;
        }
    }
    return false;
}
//checks if all of a given set of tags are attached to an object
//returns true if successfull false otherwise
bool AnnotationSet::FindAllTags(const std::vector<std::string>& searchTags){
    for (const auto& tag : searchTags) {
        std::string newTag=tag;
        std::pair<bool,std::string>newTagPair=RemoveWhiteSpace(newTag);
        if (!tags.count(newTagPair.second)) {
            return false;
        }
    }
    return true;
}

//deletes all tags in the set without deleting the object
void AnnotationSet::DeleteAllTags(){
    if(tagManager){
        for(const auto& tag:tags){
            tagManager->RemoveTag(static_cast<TagManager::ObjectId>(objectId),tag);
        }
    }
    tags.clear();
}

//gets object Id
//returns int of object id
int AnnotationSet::GetObjId()const{
    return objectId;
}

//gets tags
//returns the set containing all tags
std::set<std::string> AnnotationSet::GetTags()const{
    return tags;
}

//gets how many tags are currently in the set
int AnnotationSet::Size()const{
    return tags.size();
}

//helper function for removing white space
//returns a pair with if the tag if empty after and the cleaned tag
std::pair<bool,std::string> AnnotationSet::RemoveWhiteSpace(std::string tag){
    tag.erase(std::remove_if(tag.begin(), tag.end(), [](unsigned char x){ return std::isspace(x); }), tag.end());
    std::pair<bool,std::string> result={tag.empty(),tag};
    return(result);
}

//attaches a tagmanager to the annotationset
void AnnotationSet::AttachTagManager(TagManager& tm){
    tagManager=&tm;
    tm.RegisterObject(static_cast<TagManager::ObjectId>(objectId));
    for(const auto& tag:tags){
        tm.AddTag(static_cast<TagManager::ObjectId>(objectId),tag);
    }
}
}