#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

using pint = int64_t;
using pdouble = double;

const pdouble EPS = 1e-9;

static std::random_device rd;
static std::default_random_engine random_engine(rd());

template <typename T>
T sqr(T v) {
    return v * v;
}

struct Point {
    Point(pdouble _x, pdouble _y) : x(_x), y(_y) {}
    Point() : Point(0, 0) {}
    Point(Point from, Point to) : Point(to.x - from.x, to.y - from.y) {}
    Point& operator /=(pdouble divisor) {
        x /= divisor;
        y /= divisor;
        return *this;
    }
    pdouble x = 0, y = 0;
};

struct Circle {
    Circle(const Point& a, const Point& b) {
        Point dst(a, b);
        init(a, dst /= 2);
    }

    Circle(const Point& a, const Point& b, const Point& c) {
        if (!is_perpendicular(a, b, c)) calc_circle(a, b, c);
        else if (!is_perpendicular(a, c, b)) calc_circle(a, c, b);
        else if (!is_perpendicular(b, a, c)) calc_circle(b, a, c);
        else if (!is_perpendicular(b, c, a)) calc_circle(b, c, a);
        else if (!is_perpendicular(c, b, a)) calc_circle(c, b, a);
        else if (!is_perpendicular(c, a, b)) calc_circle(c, a, b);
        else {
            r = 0;
            r_sq = 0;
            return;
        }
    }

    bool includes(const Point& p) const {
        pdouble x_d = p.x - x;
        pdouble y_d = p.y - y;
        pdouble range_sq = sqr(x_d) + sqr(y_d);
        return r_sq + EPS > range_sq;
    }

    void init(const Point& start, const Point& dst) {
        x = start.x + dst.x;
        y = start.y + dst.y;
        r_sq = sqr(dst.x) + sqr(dst.y);
        r = sqrt(r_sq);
    }

    pdouble calc_circle(const Point& a, const Point& b, const Point& c) {
        Point ab(a, b);
        Point bc(b, c);
        if (abs(ab.x) <= EPS && abs(bc.y) <= EPS) {
            Point start(b.x, a.y);
            Point end(c.x, b.y);
            Point dst(start, end);
            init(start, dst /= 2);
            return r;
        }

        // is_perpendicular() assures that deltas are not zero
        pdouble a_slope = ab.y / ab.x;
        pdouble b_slope = bc.y / bc.x;
        if (abs(a_slope - b_slope) <= EPS) { // checking whether the given points are colinear.
            return -1;
        }

        // calc center
        x = (a_slope * b_slope * (a.y - c.y) + b_slope * (a.x + b.x) - a_slope * (b.x + c.x)) / (2 * (b_slope - a_slope));
        y = -1 * (x - (a.x + b.x) / 2) / a_slope + (a.y + b.y) / 2;
        Point end(x, y);
        Point dst(a, end);
        init(a, dst);
        return r;
    }

    bool is_perpendicular(const Point& a, const Point& b, const Point& c) {
        // Check the given point are perpendicular to x or y axis 
        Point ab(a,b);
        Point bc(b,c);
        if (abs(ab.x) <= EPS && abs(bc.y) <= EPS) {
            return false;
        }
        return (abs(ab.y) <= EPS) || (abs(bc.y) <= EPS) || (abs(ab.x) <= EPS) || (abs(bc.x) <= EPS);
    }

    pdouble x, y, r;
    pdouble r_sq;
};

Circle min_disc_with_two_points(std::vector<Point>& points, size_t q1i, size_t q2i) {
    Circle D(points[q1i], points[q2i]);
    for (size_t i = 0; i < q1i; ++i) {
        if (!D.includes(points[i])) {
            D = Circle(points[i], points[q1i], points[q2i]);
        }
    }
    return D;
}

Circle min_disc_with_point(std::vector<Point>& points, size_t qi) {
    std::shuffle(points.begin(), points.begin() + qi, random_engine);
    Circle D(points[0], points[qi]);
    for (size_t i = 1; i < qi; ++i) {
        if (!D.includes(points[i])) {
            D = min_disc_with_two_points(points, i, qi);
        }
    }
    return D;
}

Circle min_disc(std::vector<Point>& points) {
    std::shuffle(points.begin(), points.end(), random_engine);
    Circle D(points[0], points[1]);
    for (size_t i = 2; i < points.size(); ++i) {
        if (!D.includes(points[i])) {
            D = min_disc_with_point(points, i);
        }
    }
    return D;
}

pdouble dist(pdouble a, pdouble b) {
    return a * a + b * b;
}

void solve_disk_coverage() {
    size_t n;
    std::cin >> n;
    std::vector<Point> points(n);
    for (auto& point : points) {
        std::cin >> point.x >> point.y;
    }
    auto D = min_disc(points);
    std::cout << D.x << " " << D.y << "\n" << D.r;
}

int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    std::cout.precision(20);
    solve_disk_coverage();
    return EXIT_SUCCESS;
}
