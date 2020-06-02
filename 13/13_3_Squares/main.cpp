#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <list>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

template <typename T>
constexpr T sqr(T v) {
    return v * v;
}

using pint = int64_t;
using pdouble = double;
static const pint MAX_ARG = 10000;
static const pint MAX_COORD = 2 * sqr(MAX_ARG) + 1;
static const pdouble MIN_SQUARE = 1e-8;

template <typename T>
struct Point {
    Point(T _x, T _y) : x(_x), y(_y) {}
    Point() : Point(0, 0) {}
    Point(Point from, Point to) : Point(to.x - from.x, to.y - from.y) {}
    T dot(const Point& b) const& { return x * b.y - y * b.x; }
    friend std::istream& operator >> (std::istream& in, Point& p) { return in >> p.x >> p.y; }

    T x, y;
};
using DPoint = Point<pdouble>;
using IPoint = Point<pint>;

struct Line {
    Line(pint _a, pint _b, pint _c) : a(_a), b(_b), c(_c) {}
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
    HalfPlane(pint _a, pint _b, pint _c) : Line(_a, _b, _c) {}
    HalfPlane() : HalfPlane(0, 0, 0) {}
    HalfPlane(IPoint a, IPoint b) : HalfPlane(a.y - b.y, b.x - a.x, a.dot(b)) {}
    HalfPlane operator-() const& {
        return { -a, -b, -c };
    }

    template <typename T>
    bool has_point(const Point<T>& p) const {
        return a * p.x + b * p.y + c >= 0;
    }
};

struct Polygon {
    pdouble calc_square() {
        if (!points.size()) return 0;
        const Point a = points[0];
        square = 0;
        for (size_t i = 1; i < points.size() - 1; ++i) {
            const auto& b = points[i];
            const auto& c = points[i + 1];
            square += Point(a, b).dot(Point(a, c));
        }
        square /= 2;
        return square;
    }

    void cut(const HalfPlane& hp) {
        is_finite = true;
        std::vector<DPoint> new_points;
        std::vector<HalfPlane> new_lines;
        auto size = points.size();
        for (size_t i = 0; i < size; ++i) {
            auto j = (i + 1) % size;
            bool p1_belongs = hp.has_point(points[i]);
            bool p2_belongs = hp.has_point(points[j]);
            const auto& line = half_planes[i];
            if (p1_belongs) {
                new_points.push_back(points[i]);
                new_lines.push_back(line);
                is_finite &= (abs(line.c) != MAX_COORD);
                if (!p2_belongs) {
                    new_points.push_back(hp.get_intersection(line));
                    new_lines.push_back(hp);
                }
            } else if (p2_belongs) {
                new_points.push_back(hp.get_intersection(line));
                new_lines.push_back(line);
                is_finite &= (abs(line.c) != MAX_COORD);
            }
        }
        points = std::move(new_points);
        half_planes = std::move(new_lines);
    }

    pdouble square = 0;
    bool is_finite = false;
    std::vector<DPoint> points{ {-MAX_COORD, -MAX_COORD },
                               {+MAX_COORD, -MAX_COORD },
                               {+MAX_COORD, +MAX_COORD },
                               {-MAX_COORD, +MAX_COORD } };
    std::vector<HalfPlane> half_planes{ {0,1,MAX_COORD},
                                        {-1,0,MAX_COORD},
                                        {0,-1,MAX_COORD},
                                        {1,0,MAX_COORD} };
};

void extend_polygons(std::vector<Polygon>* container, Polygon polygon, HalfPlane hp) {
    polygon.cut(hp);
    if (polygon.calc_square() >= MIN_SQUARE) {
        container->push_back(polygon);
    }
}

void solve_squares() {
    size_t n;
    std::cin >> n;
    HalfPlane hp;
    std::vector<Polygon> polygons(1);
    std::vector<Polygon> temp_polygons;
    auto p_polygons = &polygons;
    auto p_temp_polygons = &temp_polygons;
    IPoint p1, p2;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> p1 >> p2;
        HalfPlane hp(p1, p2);
        p_temp_polygons->clear();
        for (auto& polygon : *p_polygons) {
            extend_polygons(p_temp_polygons, polygon, hp);
            extend_polygons(p_temp_polygons, polygon, -hp);
        }
        std::swap(p_polygons, p_temp_polygons);
    }
    std::vector<pdouble> squares;
    for (auto& polygon : *p_polygons) {
        if (polygon.is_finite) {
            pdouble square = polygon.square;
            if (square >= MIN_SQUARE) {
                squares.push_back(square);
            }
        }
    }
    std::sort(squares.begin(), squares.end());
    std::cout << squares.size() << '\n';
    for (auto square : squares) {
        std::cout << square << '\n';
    }
}


int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    std::cout.precision(17);
    solve_squares();
    return EXIT_SUCCESS;
}
