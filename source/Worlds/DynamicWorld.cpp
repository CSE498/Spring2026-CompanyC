#include "./DynamicWorld.hpp"

void cse498::DynamicWorld::UpdateWorld(){

    update_counter++;

    // if stone >= 20 & wood >= 20, build a quarry
    // if wood >= 20 & steel >= 20, build a lumberyard
    // if wheat >= 20 & wood >= 20, build a farm
    // if ticks % 60 == 0, build a spawner
    // if wood >= 500 & stone >= 500 & 500 >= stone & wheat >= 500, build a townhall

    if(world_global_counts["wood"] >= 500 && world_global_counts["stone"] >= 500 && world_global_counts["steel"] >= 500 && world_global_counts["wheat"] >= 500){
        Building townhall(update_counter);
        buildings.push_back(townhall);
        world_global_counts["wood"] -= 500;
        world_global_counts["stone"] -= 500;
        world_global_counts["steel"] -= 500;
        world_global_counts["wheat"] -= 500;
        // the game should ideally end here.
        return;
    }

    if(world_global_counts["stone"] >= 20 && world_global_counts["wood"] >= 20){
        Building quarry(update_counter);
        quarry.AddResource("steel", 40);
        quarry.AddResource("stone", 10);
        buildings.push_back(quarry);
        world_global_counts["stone"] -= 20;
        world_global_counts["wood"] -= 20;
    }

    if(world_global_counts["wood"] >= 20 && world_global_counts["steel"] >= 20){
        Building lumberyard(update_counter);
        lumberyard.AddResource("wood", 20);
        buildings.push_back(lumberyard);
        world_global_counts["wood"] -= 20;
        world_global_counts["steel"] -= 20;
    }

    if(world_global_counts["wheat"] >= 20 && world_global_counts["wood"] >= 20){
        Building farm(update_counter);
        farm.AddResource("wheat", 10);
        buildings.push_back(farm);
        world_global_counts["wheat"] -= 20;
        world_global_counts["wood"] -= 20;
    }

    if(update_counter % 60 == 0){
        // is there a need to create a building? or I can just create an agent.
    }


}