#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

using pint = int64_t;
using pdouble = double;
static const pint MAX_ARG = 10000;
static const pint MAX_COORD = 2 * MAX_ARG * MAX_ARG + 1;

template <typename T>
T sqr(T v) {
    return v * v;
}

template <typename T>
struct Point {
    Point(T _x, T _y) : x(_x), y(_y) {}
    Point() : Point(0, 0) {}
    Point(Point from, Point to) : Point(to.x - from.x, to.y - from.y) {}
    T dot(const Point& b) const& { return x * b.y - y * b.x; }

    T x, y;
};
using DPoint = Point<pdouble>;
using IPoint = Point<pint>;

struct Line {
    DPoint get_intersection(const Line& l2) const& {
        const auto& l1 = *this;
        assert(l1.a != l2.a || l1.b != l2.b);
        pdouble det_ab = l1.a * l2.b - l1.b * l2.a;
        return { (l1.b * l2.c - l1.c * l2.b) / det_ab,
                 (l1.c * l2.a - l1.a * l2.c) / det_ab };
    }

    friend std::istream& operator >> (std::istream& in, Line& l) { return in >> l.a >> l.b >> l.c; }

    pint a, b, c;
};

struct HalfPlane : Line {
    template <typename T>
    bool has_point(const Point<T>& p) const {
        return a * p.x + b * p.y + c >= 0;
    }
};

struct Polygon {
    pdouble get_square() {
        const Point a = points[0];
        pdouble square = 0;
        for (size_t i = 1; i < points.size() - 1; ++i) {
            const auto& b = points[i];
            const auto& c = points[i + 1];
            square += Point(a, b).dot(Point(a, c));
        }
        square *= 0.5;
        return square;
    }

    void cut(const HalfPlane& hp) {
        std::vector<DPoint> new_points;
        std::vector<HalfPlane> new_lines;
        auto size = points.size();
        for (size_t i = 0; i < size; ++i) {
            auto j = (i + 1) % size;
            bool p1_belongs = hp.has_point(points[i]);
            bool p2_belongs = hp.has_point(points[j]);
            const auto& line = lines[i];
            if (p1_belongs) {
                new_points.push_back(points[i]);
                new_lines.push_back(line);
                if (!p2_belongs) {
                    new_points.push_back(hp.get_intersection(line));
                    new_lines.push_back(hp);
                }
            } else if (p2_belongs) {
                Point pIntersect = hp.get_intersection(line);
                new_points.push_back(pIntersect);
                new_lines.push_back(line);
            }
        }
        points = std::move(new_points);
        lines = std::move(new_lines);
    }

    std::vector<DPoint> points{ {-MAX_COORD, -MAX_COORD },
                               {+MAX_COORD, -MAX_COORD },
                               {+MAX_COORD, +MAX_COORD },
                               {-MAX_COORD, +MAX_COORD } };
    std::vector<HalfPlane> lines{ {0,1,MAX_COORD},
                                  {-1,0,MAX_COORD},
                                  {0,-1,MAX_COORD},
                                  {1,0,MAX_COORD} };
};

void solve_half_planes_intersection() {
    size_t n;
    std::cin >> n;
    Polygon polygon;
    HalfPlane hp;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> hp;
        polygon.cut(hp);
    }
    std::cout << polygon.get_square();
}


int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    std::cout.precision(11);
    solve_half_planes_intersection();
    return EXIT_SUCCESS;
}
