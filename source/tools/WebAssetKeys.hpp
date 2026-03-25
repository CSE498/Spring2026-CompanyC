#ifndef CSE498_WEB_ASSET_KEYS_HPP_
#define CSE498_WEB_ASSET_KEYS_HPP_

#include <string>

namespace cse498 {
namespace web_assets {

/**
 * @file WebAssetKeys.hpp
 * @brief Stable asset/icon keys for Group 24's web interface module.
 * @author Sadwal Patel
 * Copyright (c) 2026 Sadwal Patel
 * SPDX-License-Identifier: MIT
 * citations - ChatGPT LLM (OpenAI) was used to help generate parts of this file. The code was then reviewed and heavily edited by the author to ensure correctness and suitability for the project.
 *
 * These keys give the UI one shared naming layer for image/icon assets.
 * Worlds or adapters can expose legend/entity meanings, and UI code can
 * map those meanings to these stable keys instead of hardcoding raw strings
 * throughout the module.
 *
 * This header intentionally avoids newer standard-library features so it
 * stays compatible with older local toolchains.
 */

// Generic / shared
static const char kUnknown[] = "unknown";
static const char kPlayer[] = "player";
static const char kAgent[] = "agent";
static const char kLeader[] = "leader";
static const char kEnemy[] = "enemy";
static const char kFog[] = "fog";
static const char kSelection[] = "selection";
static const char kEmpty[] = "empty";
static const char kGround[] = "ground";
static const char kWall[] = "wall";

// Group 7 style world
static const char kGrass[] = "grass";
static const char kTree[] = "tree";
static const char kStone[] = "stone";
static const char kWheat[] = "wheat";
static const char kSteel[] = "steel";
static const char kWood[] = "wood";
static const char kQuarry[] = "quarry";
static const char kLumberyard[] = "lumberyard";
static const char kFarm[] = "farm";
static const char kSpawner[] = "spawner";
static const char kTownHall[] = "townhall";

// Group 8 style world
static const char kRock[] = "rock";
static const char kBoulder[] = "boulder";
static const char kOre[] = "ore";
static const char kPickaxe[] = "pickaxe";
static const char kPickaxeHead[] = "pickaxe_head";
static const char kPickaxeHandle[] = "pickaxe_handle";
static const char kTape[] = "tape";
static const char kTorch[] = "torch";
static const char kHelmet[] = "helmet";
static const char kFragileWall[] = "fragile_wall";

/**
 * @brief Convert a stable asset key into a project asset path.
 *
 * Example:
 *   AssetPath(kPlayer) -> "assets/player.png"
 */
inline std::string AssetPath(const char* key) {
  return std::string("assets/") + (key != nullptr ? key : "") + ".png";
}

inline std::string AssetPath(const std::string& key) {
  return std::string("assets/") + key + ".png";
}

}  // namespace web_assets
}  // namespace cse498

#endif  // CSE498_WEB_ASSET_KEYS_HPP_