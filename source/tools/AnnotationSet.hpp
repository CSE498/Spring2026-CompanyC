#pragma once

#include <vector>
#include <string>
#include <set>

namespace cse498 {
    class TagManager;

    class AnnotationSet{
        private:
            int objectId; //id of attached object
            std::set<std::string> tags; //collection of all tags
            std::pair<bool,std::string> RemoveWhiteSpace(std::string);//helper function that removes white space
            TagManager* tagManager=nullptr;
        public:
            //default constructor deleted
            AnnotationSet()=delete;
            AnnotationSet(int obj, std::set<std::string> tags={});
            void AddTag(std::string);
            void AddTags(const std::vector<std::string>& addedTags);
            bool RemoveTags(const std::vector<std::string>&);
            bool RemoveTag(const std::string&);
            bool FindTag(const std::string&);
            bool FindAnyTag(const std::vector<std::string>&);
            bool FindAllTags(const std::vector<std::string>&);
            void DeleteAllTags();
            int GetObjId()const;
            int Size()const;
            std::set<std::string> GetTags()const;

            //integration code with tagmanager made with Shashank Papani and Chatgpt
            void AttachTagManager(TagManager& tm);
    };      

}
