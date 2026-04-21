#include "../../source/tools/EndGameScreen.hpp"
//#include "../../source/tools/Timer.hpp"
#include "../../source/core/WorldPosition.hpp"
#include "../../source/tools/DataLog.hpp"
#include "../../source/Worlds/MazeWorld.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

// To run this file "g++ -std=c++23 -Wall -I../source -o test main.cpp ../../source/tools/Timer.cpp"
// This is a file used to help show the functionality of the end game screen file
// written with the help of claude ai
int main() {
    cse498::MazeWorld world; // ex just to access WorldBase owned timer

    // Start session timer (timer usage ex)
    world.GetTimer().Start("Game::Session");

    std::this_thread::sleep_for(std::chrono::milliseconds(750)); // hard coded value for demo 

    world.GetTimer().Stop("Game::Session"); // End session
    const double durationSeconds = world.GetPlaytimeSeconds(); 

    // demo purposes: the world either provides a score or not
    // score provided by worlds: score is title .score not provided: duration is title
    //can toggle value (for now) to see behavior change
    const bool hasScore = false;

    // format seconds cleanly
    std::ostringstream playtimeStream;
    playtimeStream << std::fixed << std::setprecision(2) << durationSeconds;
    const std::string playtimeText = playtimeStream.str();

    std::vector<cse498::Stat> stats;
    
    double score = 0.0;

    if (hasScore) {
        score = 1000.0 - (durationSeconds * 10.0);  // demo placeholder score
        // If score is the title, don't also have it as a stat below
        // still include playtime as a stat
        stats.push_back({"Playtime (sec)", durationSeconds});
    }
    else {
        //no score so playtime will be title
    }

    std::vector<cse498::Stat> example_world_stats = {
        {"Valuables", 10.0},
        {"Accuracy",  94.5},
        {"Kills",     12.0},
        {"Deaths",     3.0},
        {"Damage",  4821.0},
    };

    // Append extra stats so everything appears in one table
    stats.insert(stats.end(), example_world_stats.begin(), example_world_stats.end());

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

    // (title) if score exists, show score at the top. If no score, show playtime at the top instead.
    const std::string title =
        hasScore
            ? ("Score: " + std::to_string(static_cast<long long>(score)))
            : ("Playtime: " + playtimeText + " sec");

    // Build the HTML for the results page
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