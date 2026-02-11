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
    mPoints.push_back(point);
}

double WorldPath::length() const
{
    double sum = 0.0;
    for (size_t i = 1; i < mPoints.size(); ++i)
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

double WorldPath::Cross(const Point& o, const Point& a, const Point& b)
{
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

bool WorldPath::OnSegment(const Point& p, const Point& a, const Point& b)
{
    return std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x)
        && std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y);
}

bool WorldPath::SegmentsIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
{
    double d1 = Cross(c, d, a);
    double d2 = Cross(c, d, b);
    double d3 = Cross(a, b, c);
    double d4 = Cross(a, b, d);

    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
        return true;

    if (d1 == 0 && OnSegment(a, c, d)) return true;
    if (d2 == 0 && OnSegment(b, c, d)) return true;
    if (d3 == 0 && OnSegment(c, a, b)) return true;
    if (d4 == 0 && OnSegment(d, a, b)) return true;

    return false;
}

bool WorldPath::self_intersect() const
{
    const size_t n = mPoints.size();
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

double WorldPath::Dist(const Point& a, const Point& b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

