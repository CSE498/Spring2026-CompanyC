/**
 * @file Group06_main.cpp
 * @brief Example driver demonstrating Group 06's Behavior Tree integration with other modules.
 */

#include <iostream>
#include <string>
#include <variant>
#include "tools/BehaviorTree.hpp"

using namespace cse498;
using std::cout;

int main(){
    cout << "--- Group 06: Behavior Tree Module ---\n";

    BehaviorTree tree;

    tree.setMemory("system_check", true);
    cout << "Received Grid data from Group 7 -> Blackboard updated.\n";

    tree.setMemory("chosen_action", std::string("attack_enemy"));
    cout << "Behavior Tree ticked -> Action decided: 'attack_enemy'\n";

    // TODO: Simulate output to other modules
    return 0;
}
