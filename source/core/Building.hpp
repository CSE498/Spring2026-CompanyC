#pragma once
#include <map>
#include <string>


namespace cse498{
    
    class Building{
        protected:
            // internal counter to track when this building was built, used to determine when to generate resources
            size_t internalCounter;
            // map of resource types to their generation periods, used to determine when to generate resources
            std::map<std::string, size_t> resources;

        public:
            Building(size_t counter) : internalCounter(counter) {}

            void AddResource(std::string resourceName, size_t generationPeriod){
                resources[resourceName] = generationPeriod;
            }
            
            std::map<std::string, size_t> GetResources() const { return resources;}

            size_t GetBuiltTime() const { return internalCounter; }
    };
}