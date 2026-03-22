#include "./DynamicWorld.hpp"

void cse498::DynamicWorld::UpdateWorld(){

    update_counter++;

    for(auto building : buildings){
        // each building can start producing the resources after it's built,
        // and produces them at a rate determined by the building type.
        for(auto resource : building.GetResources()){
            size_t ticks_since_built = update_counter - building.GetBuiltTime();
            if(ticks_since_built % resource.second == 0){
                world_global_counts[resource.first] += 1;
            }
        }
    }


}