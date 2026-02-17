//
// WorldPath.cpp by Bryent
//

#include "WorldPath.hpp"

#include <cmath>

WorldPath::WorldPath() = default;

WorldPath::WorldPath(std::vector<Point> points) : mPoints(std::move(points)) {}

///Computes distance between two points
double WorldPath::Dist(const Point& a, const Point& b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

///Finds total length of path
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

///Helper function for floating point sign comparison
static int SignWithEps(double v)
{
    const double eps = 1e-12;
    if (v > eps) return 1;
    if (v < -eps) return -1;
    return 0;
}

///Uses cross product tests to determine intersection
bool WorldPath::SegmentsIntersect(const Point& a, const Point& b, const Point& c, const Point& d)
{
    const double ab_c = Cross(a, b, c);
    const double ab_d = Cross(a, b, d);
    const double cd_a = Cross(c, d, a);
    const double cd_b = Cross(c, d, b);

    const int s1 = SignWithEps(ab_c);
    const int s2 = SignWithEps(ab_d);
    const int s3 = SignWithEps(cd_a);
    const int s4 = SignWithEps(cd_b);

    if (s1 * s2 < 0 && s3 * s4 < 0) return true;

    if (s1 == 0 && OnSegment(c, a, b)) return true;
    if (s2 == 0 && OnSegment(d, a, b)) return true;
    if (s3 == 0 && OnSegment(a, c, d)) return true;
    if (s4 == 0 && OnSegment(b, c, d)) return true;

    return false;
}

bool WorldPath::self_intersect() const
{
    if (mPoints.size() < 4) return false;

    const size_t n = mPoints.size();

    for (size_t i = 0; i + 1 < n; ++i)
    {
        const size_t i2 = i + 1;
        if (i2 >= n) break;

        for (size_t j = i + 2; j + 1 < n; ++j)
        {
            if (SegmentsIntersect(mPoints[i], mPoints[i2], mPoints[j], mPoints[j + 1]))
                return true;
        }
    }

    return false;
}



