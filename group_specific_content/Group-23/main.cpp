#include "../../source/tools/EndGameScreen.hpp"
#include "../../source/tools/Timer.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

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

    std::vector<std::vector<int>> heatmap = {
        {1,  4,  9,  2},
        {5,  8,  3,  7},
        {2,  6, 10,  4},
        {0,  3,  5,  8},
    };

    const std::string title = "Score: " + std::to_string(static_cast<long long>(score));
    stats.insert(stats.end(), extra_stats.begin(), extra_stats.end());
    const std::string html = cse498::generateScoreScreen(title, stats, heatmap);

    std::ofstream out("result.html");
    if (!out) {
        std::cerr << "Failed to open output file\n";
        return 1;
    }
    out << html;
    std::cout << "Written to result.html\n";
    return 0;
}