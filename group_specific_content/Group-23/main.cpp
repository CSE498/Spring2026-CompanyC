#include "../../source/tools/EndGameScreen.hpp"
#include <fstream>
#include <iostream>

// This is a file used to help show the functionality of the end game screen file
// written with the help of claude ai
int main() {
    std::vector<cse498::Stat> stats = {
        {"Accuracy",  94.5},
        {"Kills",     12.0},
        {"Deaths",     3.0},
        {"Damage",  4821.0},
    };

    std::vector<std::vector<int>> heatmap = {
        {1,  4,  9,  2},
        {5,  8,  3,  7},
        {2,  6, 10,  4},
        {0,  3,  5,  8},
    };

    std::string html = cse498::generateScoreScreen("Score: 8420", stats, heatmap);

    std::ofstream out("result.html");
    if (!out) {
        std::cerr << "Failed to open output file\n";
        return 1;
    }
    out << html;
    std::cout << "Written to result.html\n";
    return 0;
}