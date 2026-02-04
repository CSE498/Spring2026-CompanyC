//
// WorldPath.h by Bryent
//

#pragma once

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

    double mTotalLength = 0.0;
    bool mLengthDirty = false;

    bool mSelfIntersectKnown = true;
    bool mSelfIntersectValue - false;

    void ValidatePoint(const Point& point);

    double Dist(const Point& a, const Point& b);
    double Dist2(const Point& a, const Point& b);

    void RebuildCachesFromScratch() const;
    void RebuildLengthCache() const;

    double Cross(const Point& a, const Point& b, const Point& c);
    bool OnSegment(const Point& a, const Point& b, const Point& p);
    int Sign(double v);
    bool SegmentsIntersect(const Point& a, const Point& b, const Point& c, const Point& d);
    bool ComputeSelfIntersect() const;
};
