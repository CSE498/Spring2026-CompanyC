#include "./DynamicWorld.hpp"

void cse498::DynamicWorld::UpdateWorld(){

    mUpdateCounter++;

    for(auto building : mBuildings){
        // each building can start producing the resources after it's built,
        // and produces them at a rate determined by the building type.
        for(auto resource : building.GetResources()){
            size_t ticks_since_built = mUpdateCounter - building.GetBuiltTime();
            if(ticks_since_built % resource.second == 0){
                mWorldGlobalCounts[resource.first] += 1;
            }
        }
    }


}