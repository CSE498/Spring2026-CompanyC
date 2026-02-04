//
// WorldPath.h by Bryent
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

    void clear();
    void addPoint(const Point& point);
    double length() const;
    bool self_intersect() const;
    const Point& start() const;
    const Point& end() const;

private:
    std::vector<Point> mPoints;

    static void ValidatePoint(const Point& point);

    static double Dist(const Point& a, const Point& b);

    static bool SegmentsIntersect(const Point& a, const Point& b, const Point& c, const Point& d);
};
