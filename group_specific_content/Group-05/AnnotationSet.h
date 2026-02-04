#pragma once

#include <vector>
#include <string>
#include <set>
class AnnotationSet{
    private:
        int objectId; //id of attached object
        std::set<std::string> tags; //collection of all tags
    public:
        //default constructor deleted
        AnnotationSet()=delete;
        AnnotationSet(int obj, std::set<std::string> tags={});
        void AddTag(std::string);
        void AddTags(std::vector<std::string>);
        bool RemoveTags(std::vector<std::string>);
        bool RemoveTag(std::string);
        bool FindTag(std::string);
        bool FindAnyTag(std::vector<std::string>);
        bool FindAllTags(std::vector<std::string>);
        void DeleteAllTags();
        int GetObjId();
        std::set<std::string> GetTags();
};