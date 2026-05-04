#pragma once
#include <unordered_map>
#include <string>


namespace cse498{
    
    class Building{
        protected:
            // internal counter to track when this building was built, used to determine when to generate resources
            size_t mInternalCounter;
            // map of resource types to their generation periods, used to determine when to generate resources
            std::unordered_map<std::string, size_t> mResources;

        public:
            Building(size_t counter) : mInternalCounter(counter) {}

            void AddResource(const std::string& resourceName, size_t generationPeriod){
                mResources[resourceName] = generationPeriod;
            }

            [[nodiscard]] const std::unordered_map<std::string, size_t>& GetResources() const { return mResources;}

            [[nodiscard]] size_t GetBuiltTime() const { return mInternalCounter; }
    };
}