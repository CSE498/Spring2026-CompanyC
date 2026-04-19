#pragma once

#include "Database.hpp"
#include "WorldBase.hpp"

#include <expected>
#include <string>

namespace cse498 {

//key format: world:<name>:meta, world:<name>:grid, world:<name>:agent:<id>, world:<name>:item:<id>

std::expected<void, DatabaseError> SaveWorld(Database& db, const std::string& world_name, const WorldBase& world);
std::expected<void, DatabaseError> LoadWorld(Database& db, const std::string& world_name, WorldBase& world);

}
