/**
 * @file StringDiff.cpp
 * @author Andrew Shilman
 * 
 * @brief Implementation of StringDiff for computing string diffs
 */

#include "StringDiff.hpp"
#include <functional> // for std::hash
#include <algorithm>  // for std::min
#include <sstream>    // for std::ostringstream, std::istringstream

namespace sim {

//Private helper
std::size_t StringDiff::ComputeHash(const std::string& str) {
    return std::hash<std::string>{}(str);
}










}