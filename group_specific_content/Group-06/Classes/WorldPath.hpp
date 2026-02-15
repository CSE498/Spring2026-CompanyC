//
// WorldPath.hpp by Bryent
//

#pragma once

#include <vector>

struct Point
{
    double x = 0.0;
    double y = 0.0;
};

class WorldPath
{
public:
    WorldPath();
    explicit WorldPath(std::vector<Point> points);

    void clear() { mPoints.clear(); }
    void Add(const Point& p) { mPoints.push_back(p); }
    void Reverse() { std::reverse(mPoints.begin(), mPoints.end()); }
    double length() const;
    bool self_intersect() const;
    const Point& start() const;
    const Point& end() const;

private:
    std::vector<Point> mPoints;

    static double Dist(const Point& a, const Point& b);
    static bool SegmentsIntersect(const Point& a, const Point& b, const Point& c, const Point& d);
    static double Cross(const Point& o, const Point& a, const Point& b);
    static bool OnSegment(const Point& p, const Point& a, const Point& b);

};
