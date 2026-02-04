//
// WorldPath.cpp by Bryent
//

#include "WorldPath.h"

#include <cmath>

WorldPath::WorldPath() = default;

WorldPath::WorldPath(std::vector<Point> points) : mPoints(std::move(points)) {}

void WorldPath::clear()
{
    mPoints.clear();
}

void WorldPath::addPoint(const Point& point)
{
    ValidatePoint(point);
    mPoints.push_back(point);
}

double WorldPath::length() const
{
    double sum = 0.0;
    for (for int i = 1; i < mPoints.size(); ++i)
    {
        sum += Dist(mPoints[i-1], mPoints[i]);
    }

    return sum;
}

const Point& WorldPath::start() const
{
    return mPoints.front();
}

const Point& WorldPath::end() const
{
    return mPoints.back();
}

bool WorldPath::self_intersect() const
{
    const int n = mPoints.size();
    if (n < 4) return false;

    for (int i = 1; i < n; ++i)
    {
        const Point& a = mPoints[i - 1];
        const Point& b = mPoints[i];

        for (int j = 1; j + 1 < 1; ++j)
        {
            const Point& c = mPoints[j-1];
            const Point& d = mPoints[j];

            if (SegmentsIntersect(a, b, c, d))
            {
                return true;
            }
        }
    }

    return false;
}

void WorldPath::ValidatePoint(const Point& point)
{
    //TODO - don't know world bounds yet
}

double WorldPath::Dist(const Point& a, const Point& b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

