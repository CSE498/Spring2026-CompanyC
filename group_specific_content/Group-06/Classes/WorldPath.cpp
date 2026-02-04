//
// WorldPath.cpp by Bryent
//

#include "WorldPath.h"

#include <cmath>

WorldPath::WorldPath() = default;

WorldPath::WorldPath(std::vector<Point> points) : mPoints(points) {}