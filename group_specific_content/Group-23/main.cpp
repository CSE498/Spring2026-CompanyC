#include "../../source/tools/EndGameScreen.hpp"
#include "../../source/tools/Timer.hpp"
#include "../../source/core/WorldPosition.hpp"
#include "../../source/tools/DataLog.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

// To run this file "g++ -std=c++23 -Wall -I../source -o test main.cpp ../../source/tools/Timer.cpp"
// This is a file used to help show the functionality of the end game screen file
// written with the help of claude ai
int main() {
    cse498::Timer timer;

    // Start session timer (this simulates "Start Game" being pressed)
    timer.Start("Game::Session");

    // hard coded value for now: this will be the duration temporarily. 
    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    // End session
    timer.Stop("Game::Session");
    const double durationSeconds = timer.Last("Game::Session");

    // Simple score example for now:
    // valuables = 10, time penalty = durationSeconds
    const double valuables = 10.0; //hard coded value for now
    const double score = (valuables * 100.0) - (durationSeconds * 10.0);

    std::vector<cse498::Stat> stats = {
        {"Playtime (sec)", durationSeconds},
        {"Valuables", valuables},
        {"Score", score},
    };

    std::vector<cse498::Stat> extra_stats = {
        {"Accuracy",  94.5},
        {"Kills",     12.0},
        {"Deaths",     3.0},
        {"Damage",  4821.0},
    };

    cse498::DataLog<cse498::WorldPosition> heatmap;
    for (int i = 0; i < 10; ++i){
        int count = 0;
        for (int j = 0; j < 10; ++j){
            for (int k = 0; k < count; ++k){
                cse498::WorldPosition coord(i,j);
                heatmap.Add(coord); 
            }
            ++count;
        }
    }

    const std::string title = "Score: " + std::to_string(static_cast<long long>(score));
    stats.insert(stats.end(), extra_stats.begin(), extra_stats.end());
    const std::string html = cse498::generateScoreScreen(title, stats, heatmap, 10);

    std::ofstream out("result.html");
    if (!out) {
        std::cerr << "Failed to open output file\n";
        return 1;
    }
    out << html;
    std::cout << "Written to result.html\n";
    return 0;
}