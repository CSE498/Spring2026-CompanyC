//By Sachin Karatha
//
//This class stores tags that are attached to a given object with a unique Id.
//
//Class includes methods to add and remove tags using vectors or strings depending
//on the amount being added. Tags can also be checked for membership whether it be
//only one or multiple. 

#include "group_specific_content/Group-05/AnnotationSet.h" //Have to change this later
#include <set>
#include <string>


//new constructor initializes object Id and tags with a default value of {}
AnnotationSet::AnnotationSet(int obj, std::set<std::string> inTags={}){
    objectId=obj;
    tags=inTags;
}

//adds a tag to the tags
void AnnotationSet::AddTag(std::string tag){
    tags.insert(tag);
}

//adds tags to the set of tags
void AnnotationSet::AddTags(std::vector<std::string> addedTags){
    for(int i=0;i<addedTags.size();i++){
        tags.insert(addedTags[i]);
    }
}

//removes tags from the set of tags
//returns true if all were successfull false otherwise
bool AnnotationSet::RemoveTags(std::vector<std::string> removedTags){
    bool removed=true;
    for(int i=0;i<removedTags.size();i++){
        bool rem=tags.erase(removedTags[i]);
        if(rem==false){
            removed=false;
        }

    }
    return removed;
}

//removes a tag from the set of tags
//returns true if successfull false otherwise
bool AnnotationSet::RemoveTag(std::string tag){
    return tags.erase(tag);
}

//checks if a certain tag is attached to object
//returns true if successfull false otherwise
bool AnnotationSet::FindTag(std::string tag){
    if(tags.count(tag)==1){
        return true;
    }
    else{
        return false;
    }
}

//checks if any of a given set of tags are attached to an object
//returns true if successfull false otherwise
bool AnnotationSet::FindAnyTag(std::vector<std::string> searchTags){
    for(int i=0;i<searchTags.size();i++){
        std::string currSearched=searchTags[i];
        if (tags.count(currSearched)==1){
            return true;
        }
    }
    return false;
}
//checks if all of a given set of tags are attached to an object
//returns true if successfull false otherwise
bool AnnotationSet::FindAllTags(std::vector<std::string> searchTags){
    for(int i=0;i<searchTags.size();i++){
        std::string currSearched=searchTags[i];
        if (tags.count(currSearched)==0){
            return false;
        }
    }
    return true;
}

//deletes all tags in the set without deleting the object
void AnnotationSet::DeleteAllTags(){
    tags.clear();
}

//gets object Id
//returns int of object id
int AnnotationSet::GetObjId(){
    return objectId;
}

//gets tags
//returns the set containing all tags
std::set<std::string> AnnotationSet::GetTags(){
    return tags;
}