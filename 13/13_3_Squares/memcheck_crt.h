#pragma once
#ifndef MEMCHECK_CRT_
#define MEMCHECK_CRT_

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define ENABLE_CRT\
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);\
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);\
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#else
#define ENABLE_CRT ;
#endif

#endif // !MEMCHECK_CRT_


/**
  * C. Areas
  *
  * input standard input
  * output standard output
  *
  * You're given n straight lines on 2D plane.
  * They divide the plane into parts.
  * Some of them are finite and others are infinite.
  * Find areas of all finite parts.
  *
  * Input
  * First line contains integer n — the number of lines (1 ≤ n ≤ 80).
  * Each of next n lines contain four integers x 1, y 1, x 2 and y 2 — coordinates of two distinct points of i-th line.
  * Coordinates don't exceed 100 by absolute value. All lines are pairwise distinct.
  *
  * Output
  * In the first line output k — number of finite parts.
  * In next k lines output areas of these parts in non-decreasing order.
  * The error shouldn't exceed 10^-4.
  * Ignore and don't output parts with an area less than 10^-8.
  */

  //#define DEBUG

//#include <algorithm>
//#include <assert.h>
//#include <cmath>
//#include <iostream>
//#ifdef DEBUG
//#include <fstream>
//#endif
//#include <vector>
//
//    using value_t = long long;
//
//class KahanAccumulator {
//private:
//    double value_;
//    double delta_;
//public:
//    KahanAccumulator() : value_(0), delta_(0) {}
//    double getValue() const {
//        return value_;
//    }
//    void add(double summand) {
//        double a = summand - delta_;
//        double b = value_ + a;
//        delta_ = (b - value_) - a;
//        value_ = b;
//    }
//};
//
//struct Point {
//    double x_;
//    double y_;
//};
//
//struct HalfPlane {
//    value_t a_;
//    value_t b_;
//    value_t c_;
//    bool border_;
//
//    HalfPlane() : border_(false) {}
//};
//
//static constexpr value_t MAX_COORD = 200000001;
//static const double MIN_SQUARE = 1e-8;
//
//bool belongsToHalfPlane(const Point& p, const HalfPlane& hp) {
//    return hp.a_ * p.x_ + hp.b_ * p.y_ + hp.c_ >= 0;
//}
//
//bool calcIntersection(const HalfPlane& hp0, const HalfPlane& hp1, Point& intersect) {
//    if (hp0.a_ == hp1.a_ && hp0.b_ == hp1.b_) {
//        return false;
//    }
//    value_t detAB = hp0.a_ * hp1.b_ - hp0.b_ * hp1.a_;
//    intersect.x_ = 1.0 * (hp0.b_ * hp1.c_ - hp0.c_ * hp1.b_) / detAB;
//    intersect.y_ = 1.0 * (hp0.c_ * hp1.a_ - hp0.a_ * hp1.c_) / detAB;
//    return true;
//}
//
//bool pointIsBetween(Point& p0, Point& p1, Point& pBetween) {
//    return ((p0.x_ <= pBetween.x_ && pBetween.x_ < p1.x_) || (p0.x_ >= pBetween.x_ && pBetween.x_ > p1.x_))
//        && ((p0.y_ <= pBetween.y_ && pBetween.y_ < p1.y_) || (p0.y_ >= pBetween.y_ && pBetween.y_ > p1.y_));
//}
//
//std::pair<std::vector<Point>, std::vector<HalfPlane>> cut(
//    const std::vector<Point>& polygon,
//    const std::vector<HalfPlane>& polygonLines,
//    const HalfPlane& hp) {
//    std::vector<Point> newPolygon;
//    std::vector<HalfPlane> newPolygonLines;
//    for (std::size_t i = 0; i < polygon.size() - 1; ++i) {
//        bool pBelongs = belongsToHalfPlane(polygon[i], hp);
//        bool pNextBelongs = belongsToHalfPlane(polygon[i + 1], hp);
//        if (pBelongs) {
//            newPolygon.push_back(polygon[i]);
//            newPolygonLines.push_back(polygonLines[i]);
//        }
//        if (pBelongs && !pNextBelongs) {
//            Point pIntersect;
//            calcIntersection(hp, polygonLines[i], pIntersect);
//            newPolygon.push_back(pIntersect);
//            newPolygonLines.push_back(hp);
//        }
//        if (!pBelongs && pNextBelongs) {
//            Point pIntersect;
//            calcIntersection(hp, polygonLines[i], pIntersect);
//            newPolygon.push_back(pIntersect);
//            newPolygonLines.push_back(polygonLines[i]);
//        }
//    }
//    {
//        std::size_t maxIndex = polygon.size() - 1;
//        bool pBelongs = belongsToHalfPlane(polygon[maxIndex], hp);
//        bool pNextBelongs = belongsToHalfPlane(polygon[0], hp);
//        if (pBelongs) {
//            newPolygon.push_back(polygon[maxIndex]);
//            newPolygonLines.push_back(polygonLines[maxIndex]);
//        }
//        if (pBelongs && !pNextBelongs) {
//            Point pIntersect;
//            calcIntersection(hp, polygonLines[maxIndex], pIntersect);
//            newPolygon.push_back(pIntersect);
//            newPolygonLines.push_back(hp);
//        }
//        if (!pBelongs && pNextBelongs) {
//            Point pIntersect;
//            calcIntersection(hp, polygonLines[maxIndex], pIntersect);
//            newPolygon.push_back(pIntersect);
//            newPolygonLines.push_back(polygonLines[maxIndex]);
//        }
//    }
//
//    return std::make_pair(newPolygon, newPolygonLines);
//}
//
//double calcConvexHullSquare(const std::vector<Point>& hull) {
//    const Point a = hull[0];
//    KahanAccumulator square;
//
//    for (std::size_t i = 1; i < hull.size() - 1; ++i) {
//        const Point& b = hull[i];
//        const Point& c = hull[i + 1];
//        const double xab = b.x_ - a.x_;
//        const double yab = b.y_ - a.y_;
//        const double xac = c.x_ - a.x_;
//        const double yac = c.y_ - a.y_;
//        const double abcSquare = 0.5 * (xab * yac - yab * xac);
//        assert(abcSquare >= 0);
//        square.add(abcSquare);
//    }
//
//    return square.getValue();
//}
//
//HalfPlane getCanonicalLineEquation(const value_t x0, const value_t y0, const value_t x1, const value_t y1) {
//    HalfPlane result;
//    result.a_ = y0 - y1;
//    result.b_ = x1 - x0;
//    result.c_ = x0 * y1 - y0 * x1;
//    return result;
//}
//
//int main() {
//#ifdef DEBUG
//    std::ifstream cin("input.txt");
//#else
//    using std::cin;
//    std::ios::sync_with_stdio(false);
//    cin.tie(nullptr);
//    std::cout.tie(nullptr);
//#endif
//    std::cout.precision(12);
//
//    std::vector<Point> polygon(4);
//    polygon[0].x_ = -MAX_COORD;
//    polygon[0].y_ = -MAX_COORD;
//    polygon[1].x_ = MAX_COORD;
//    polygon[1].y_ = -MAX_COORD;
//    polygon[2].x_ = MAX_COORD;
//    polygon[2].y_ = MAX_COORD;
//    polygon[3].x_ = -MAX_COORD;
//    polygon[3].y_ = MAX_COORD;
//
//    std::vector<HalfPlane> polygonLines(4);
//    polygonLines[0].a_ = 0;
//    polygonLines[0].b_ = 1;
//    polygonLines[0].c_ = MAX_COORD;
//    polygonLines[0].border_ = true;
//    polygonLines[1].a_ = -1;
//    polygonLines[1].b_ = 0;
//    polygonLines[1].c_ = MAX_COORD;
//    polygonLines[1].border_ = true;
//    polygonLines[2].a_ = 0;
//    polygonLines[2].b_ = -1;
//    polygonLines[2].c_ = MAX_COORD;
//    polygonLines[2].border_ = true;
//    polygonLines[3].a_ = 1;
//    polygonLines[3].b_ = 0;
//    polygonLines[3].c_ = MAX_COORD;
//    polygonLines[3].border_ = true;
//
//    std::vector<std::pair<std::vector<Point>, std::vector<HalfPlane>>> data;
//    data.push_back(std::make_pair(polygon, polygonLines));
//
//    std::size_t lineCount;
//    cin >> lineCount;
//    for (std::size_t i = 0; i < lineCount; ++i) {
//        value_t x0, y0, x1, y1;
//        cin >> x0 >> y0 >> x1 >> y1;
//        HalfPlane hp0 = getCanonicalLineEquation(x0, y0, x1, y1);
//        HalfPlane hp1;
//        hp1.a_ = -hp0.a_;
//        hp1.b_ = -hp0.b_;
//        hp1.c_ = -hp0.c_;
//
//        std::vector<std::pair<std::vector<Point>, std::vector<HalfPlane>>> newData;
//
//        for (auto& datum : data) {
//            auto newDatum0 = cut(datum.first, datum.second, hp0);
//            if (newDatum0.first.size() && calcConvexHullSquare(newDatum0.first) > MIN_SQUARE) {
//                newData.push_back(newDatum0);
//            }
//            auto newDatum1 = cut(datum.first, datum.second, hp1);
//            if (newDatum1.first.size() && calcConvexHullSquare(newDatum1.first) > MIN_SQUARE) {
//                newData.push_back(newDatum1);
//            }
//        }
//        data = newData;
//    }
//
//    std::vector<double> squares;
//    for (auto& datum : data) {
//        bool isInfinite = false;
//        for (HalfPlane& hp : datum.second) {
//            if (hp.border_) {
//                isInfinite = true;
//                break;
//            }
//        }
//        if (!isInfinite) {
//            double square = calcConvexHullSquare(datum.first);
//            if (square > MIN_SQUARE) {
//                squares.push_back(square);
//            }
//        }
//    }
//
//    std::sort(squares.begin(), squares.end());
//    std::cout << squares.size() << '\n';
//    for (double square : squares) {
//        std::cout << square << '\n';
//    }
//
//    return 0;
//}