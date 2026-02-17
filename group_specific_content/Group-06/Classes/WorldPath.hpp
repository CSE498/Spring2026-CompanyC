//
// WorldPath.hpp by Bryent
//

#pragma once

#include <vector>
#include <algorithm>

///2D Point structure
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

    ///Removes all points
    void clear() { mPoints.clear(); }
    ///Adds a point to the end of the path
    void Add(const Point& p) { mPoints.push_back(p); }
    ///Reverses the order of the points in the path
    void Reverse() { std::reverse(mPoints.begin(), mPoints.end()); }
    ///Returns the length of the path
    double length() const;
    ///Returns whether or not the path intersects itself
    bool self_intersect() const;
    ///Returns the first point
    const Point& start() const;
    ///Returns the last point
    const Point& end() const;

private:
    ///Ordered list of path points
    std::vector<Point> mPoints;

    ///Euclidean distance between two points
    static double Dist(const Point& a, const Point& b);
    ///Determines if two line legments intersect
    static bool SegmentsIntersect(const Point& a, const Point& b, const Point& c, const Point& d);
    ///Calculates cross products
    static double Cross(const Point& o, const Point& a, const Point& b);
    ///Check if a point is on line segment AB
    static bool OnSegment(const Point& p, const Point& a, const Point& b);

};
