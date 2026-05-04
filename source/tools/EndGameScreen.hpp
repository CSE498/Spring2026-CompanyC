/**
 * @file EndGameScreen.hpp
 * @author Group 23
 *
 * @brief Generates a string holding the HTML code needed to create the end game screen
 */

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "../core/WorldPosition.hpp"
#include "DataLog.hpp"
 
namespace cse498
{ 
    // Represents a single labeled stat
    struct Stat {
        std::string label;
        double value;
    };

	// Used claude ai to help with this function
    // Internal helper: maps a normalized value [0,1] to an RGB heatmap color (blue -> green -> red)
    static std::string heatmapColor(double t) {
        // Clamp
        t = std::max(0.0, std::min(1.0, t));

        int r, g, b;
        if (t < 0.5) {
            // Blue -> Green
            double s = t * 2.0;
            r = 0;
            g = static_cast<int>(s * 200);
            b = static_cast<int>((1.0 - s) * 220);
        } else {
            // Green -> Red
            double s = (t - 0.5) * 2.0;
            r = static_cast<int>(s * 220);
            g = static_cast<int>((1.0 - s) * 200);
            b = 0;
        }

        std::ostringstream oss;
        oss << "rgb(" << r << "," << g << "," << b << ")";
        return oss.str();
    }

	// Forward declaration so helpers can call
	inline std::string generateScoreScreen(
	const std::string& title,
	const std::vector<Stat>& stats,
	const cse498::DataLog<cse498::WorldPosition>& dataLogHM,
	int size,
	int precision = 2,
	bool playerOnlyHeatmap = false
	);

	// builds the title + stats table
	// if hasScore is true, title = Score: X, include playtime/duration as a stat
	// if hasScore is false, title = Playtime: Y sec, score not included
	inline std::string BuildEndGameHtml(
	bool hasScore,
	double scoreValue, // ignored if hasScore is false
	double playtimeSeconds,
	std::vector<Stat> statsFromWorld,  //whatever the world wants to show (Valuables, Kills, etc.)
	const cse498::DataLog<cse498::WorldPosition>& dataLogHM,
	int size,
	int precision = 2,
	bool playerOnlyHeatmap = false
	) {
	// format playtime decimals
	std::ostringstream playtimeStream;
	playtimeStream << std::fixed << std::setprecision(precision) << playtimeSeconds;
	const std::string playtimeText = playtimeStream.str();

	// add playtime as a stat when Score is the title 
	if (hasScore) {
		statsFromWorld.insert(statsFromWorld.begin(), {"Playtime (sec)", playtimeSeconds});
	}

	const std::string title =
		hasScore
		? ("Score: " + std::to_string(static_cast<long long>(scoreValue)))
		: ("Playtime: " + playtimeText + " sec");

	return generateScoreScreen(title, statsFromWorld, dataLogHM, size, precision, playerOnlyHeatmap);
	}


	// Used claude ai to help with the implementation of the heatmap
    /**
     * Generates an HTML string displaying a score screen.
     *
     * @param title      The score/title shown at the top (e.g. "Score: 8420")
     * @param stats      A vector of Stat structs (label + numeric value)
     * @param dataLogHM  A vector holding every coordinate that an agent has ever been om
	 * @param size		 The size of the map
     * @param precision  Decimal places to show for stat values (default: 2)
     * @return           A complete HTML document string ready to display or write to file
     */
    inline std::string generateScoreScreen(
    const std::string& title,
    const std::vector<Stat>& stats,
	const cse498::DataLog<cse498::WorldPosition>& dataLogHM,
    const int size,
    int precision,
	bool playerOnlyHeatmap)
		{
			std::vector<std::vector<int>> heatmap(size, std::vector<int>(size, 0));
			for (const auto& cord : dataLogHM.Values("player")) {
				heatmap[cord.X()][cord.Y()] += 1;
			}

			if (!playerOnlyHeatmap) {
				for (const auto& cord : dataLogHM.Values("nonplayer")) {
					heatmap[cord.X()][cord.Y()] += 1;
				}
			}

			// --- Normalize heatmap values ---
			int minVal = heatmap[0][0];
			int maxVal = heatmap[0][0];
	
			for (const auto& row : heatmap) {
					for (int v : row) {
							minVal = std::min(minVal, v);
							maxVal = std::max(maxVal, v);
					}
			}
	
			double range = static_cast<double>(maxVal - minVal);
			if (range == 0.0) range = 1.0; // Avoid division by zero (flat grid)
	
			// --- Build stat rows ---
			std::ostringstream statRows;
			for (const auto& s : stats) {
					statRows
							<< "      <tr>\n"
							<< "        <td class=\"stat-label\">" << s.label << "</td>\n"
							<< "        <td class=\"stat-value\">"
							<< std::fixed << std::setprecision(precision) << s.value
							<< "</td>\n"
							<< "      </tr>\n";
			}
	
			// --- Build heatmap cells ---
			std::ostringstream heatmapGrid;
			for (const auto& row : heatmap) {
					heatmapGrid << "      <tr>\n";
					for (int v : row) {
							double t = static_cast<double>(v - minVal) / range;
							std::string color = heatmapColor(t);
							// Show the raw value as a tooltip on hover
							std::ostringstream valStr;
							valStr << std::fixed << std::setprecision(precision) << v;
							heatmapGrid
									<< "        <td class=\"heat-cell\" "
									<< "style=\"background:" << color << ";\" "
									<< "title=\"" << valStr.str() << "\"></td>\n";
					}
					heatmapGrid << "      </tr>\n";
			}
	
			// --- Assemble full HTML ---
			std::ostringstream html;
			html << R"(<!DOCTYPE html>
			<html lang="en">
			<head>
			<meta charset="UTF-8" />
			<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
			<title>Score Screen</title>
			<style>
				*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
	
			body {
				font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
				background: #f0f2f5;
				display: flex;
				justify-content: center;
				align-items: center;
				min-height: 100vh;
			}
	
			.screen {
				background: #ffffff;
				border: 1px solid #c8cdd4;
				border-radius: 6px;
				width: 600px;
				overflow: hidden;
				box-shadow: 0 4px 16px rgba(0,0,0,0.1);
			}
	
			/* --- Title bar --- */
			.title-bar {
				text-align: center;
				padding: 14px 20px;
				font-size: 1.3rem;
				font-weight: 600;
				color: #1a1a2e;
				border-bottom: 1px solid #c8cdd4;
				background: #f7f8fa;
				letter-spacing: 0.02em;
			}
	
			/* --- Two-panel body --- */
			.body {
				display: flex;
				min-height: 280px;
			}
	
			/* --- Left: Stats panel --- */
			.stats-panel {
				flex: 1;
				border-right: 1px solid #c8cdd4;
				padding: 20px;
				overflow-y: auto;
			}
	
			.stats-panel h3 {
				font-size: 0.7rem;
				font-weight: 700;
				text-transform: uppercase;
				letter-spacing: 0.1em;
				color: #888;
				margin-bottom: 12px;
			}
	
			.stats-panel table {
				width: 100%;
				border-collapse: collapse;
			}
	
			.stat-label {
				font-size: 0.9rem;
				color: #444;
				padding: 5px 0;
			}
	
			.stat-value {
				font-size: 0.9rem;
				font-weight: 600;
				color: #1a1a2e;
				text-align: right;
				padding: 5px 0;
			}
	
			.stats-panel tr + tr td {
				border-top: 1px solid #f0f0f0;
			}
	
			/* --- Right: Heatmap panel --- */
			.heatmap-panel {
				flex: 1;
				padding: 20px;
				display: flex;
				flex-direction: column;
			}
	
			.heatmap-panel h3 {
				font-size: 0.7rem;
				font-weight: 700;
				text-transform: uppercase;
				letter-spacing: 0.1em;
				color: #888;
				margin-bottom: 12px;
			}
	
			.heatmap-panel table {
				border-collapse: collapse;
				width: 100%;
				flex: 1;
			}
	
			.heat-cell {
				border: 1px solid rgba(255,255,255,0.3);
				transition: opacity 0.15s;
				cursor: default;
				min-width: 10px;
				min-height: 10px;
			}
	
			.heat-cell:hover {
				opacity: 0.75;
				outline: 2px solid #1a1a2e;
			}
	
			/* Legend */
			.legend {
				display: flex;
				align-items: center;
				gap: 8px;
				margin-top: 10px;
				font-size: 0.7rem;
				color: #888;
			}
	
			.legend-bar {
				flex: 1;
				height: 8px;
				border-radius: 4px;
				background: linear-gradient(to right, rgb(0,0,220), rgb(0,200,0), rgb(220,0,0));
			}
			</style>
			</head>
			<body>
			<div class="screen">
			<div class="title-bar">)";
	
			html << title;
	
			html << R"(</div>
			<div class="body">
	
				<!-- Stats Panel -->
				<div class="stats-panel">
					<h3>Stats</h3>
					<table>
			)";
			html << statRows.str();
			html << R"(        </table>
				</div>
	
				<!-- Heatmap Panel -->
				<div class="heatmap-panel">
					<h3>Heatmap</h3>
					<table>
			)";
			html << heatmapGrid.str();
			html << R"(        </table>
					<div class="legend">
						<span>Low</span>
						<div class="legend-bar"></div>
						<span>High</span>
					</div>
				</div>
	
					</div>
				</div>
			</body>
			</html>
			)";
	
			return html.str();
	}
}