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

namespace geometry::primitives {
    using pint = int64_t;
    using pdouble = double;

    static const pdouble EPS = 1e-9;

    template <typename T>
    inline constexpr T sqr(T v) {
        return v * v;
    }

    template <typename T>
    struct Point {
        Point(T _x, T _y) : x(_x), y(_y) {}
        Point() : Point(0, 0) {}
        Point(Point from, Point to) : Point(to.x - from.x, to.y - from.y) {}
        template <typename U>
        operator Point<U>() const { return { static_cast<U>(x), static_cast<U>(y) }; }
        bool operator == (const Point& p) const& { return abs(x - p.x) < EPS && abs(y - p.y) < EPS; }
        Point operator -() const& { return Point(-x, -y); }
        Point operator + (const Point& rhs) const& { return Point(-rhs, *this); }
        Point operator - (const Point& rhs) const& { return (*this) + -rhs; }
        Point& operator *= (T multiplier) {
            x *= multiplier;
            y *= multiplier;
            return *this;
        }
        Point& operator /= (T divisor) { // if Point is of integral type, require chords to be divisable by divisor
            assert(!std::is_integral<T>() || ((x / divisor) * divisor == x && (y / divisor) * divisor == y));
            x /= divisor;
            y /= divisor;
            return *this;
        }
        Point operator / (T divisor) const& { return Point(*this) /= divisor; }
        friend std::istream& operator >> (std::istream& in, Point& p) { return in >> p.x >> p.y; }
        friend std::ostream& operator << (std::ostream& out, Point p) { return out << p.x << " " << p.y; }
        T dot(const Point& b) const& { return x * b.y - y * b.x; }
        T scalar(const Point& b) const& { return x * b.x + y * b.y; }
        T sqr_length() const& { return sqr(x) + sqr(y); }
        template <typename U>
        pdouble cos(const Point<U>& b) const& { return scalar(b) / sqrt(static_cast<pdouble>(sqr_length() * b.sqr_length())); }

        T x, y;
    };
    using IPoint = Point<pint>;
    using DPoint = Point<pdouble>;

    template <typename T>
    struct Line {
        Line(T _a, T _b, T _c) : a(_a), b(_b), c(_c) {}
        Line() : Line(0, 0, 0) {}
        Line(Point<T> a, Point<T> b) : Line(a.y - b.y, b.x - a.x, a.dot(b)) {}
        bool operator == (const Line& l2) const& {
            if (a != 0) {
                return ((abs(1.0 * l2.b * a / l2.a - b) < EPS) && (abs(1.0 * l2.c * a / l2.a - c) < EPS));
            } else if (b != 0) {
                return ((abs(1.0 * l2.a * b / l2.b - a) < EPS) && (abs(1.0 * l2.c * b / l2.b - c) < EPS));
            } else if (c != 0) {
                return ((abs(1.0 * l2.a * c / l2.c - a) < EPS) && (abs(1.0 * l2.b * c / l2.c - b) < EPS));
            }
            return false;
        }
        Line operator -() const& { return { -a, -b, -c }; }
        friend std::istream& operator >> (std::istream& in, Line& l) {
            return in >> l.a >> l.b >> l.c;
        }
        bool is_collinear(const Line& l2) const& {
            if (a != 0) {
                return (abs(1.0 * l2.b * a / l2.a - b) < EPS);
            } else if (b != 0) {
                return (abs(1.0 * l2.a * b / l2.b - a) < EPS);
            }
            return false;
        }
        static Line get_perpendicular_bisector(Point<T> A, Point<T> B) {
            T a = A.x - B.x;
            T b = A.y - B.y;
            Point M(A + (B - A) / 2);
            T c = -a * M.x - b * M.y;
            return Line(a, b, c);
        }
        std::pair<DPoint, pdouble> get_intersection_details(const Line& l2) const& {
            const auto& l1 = *this;
            assert(l1.a != l2.a || l1.b != l2.b);
            pdouble det_ab = static_cast<pdouble>(l1.a * l2.b - l1.b * l2.a);
            return std::make_pair(Point((l1.b * l2.c - l1.c * l2.b) / det_ab,
                                        (l1.c * l2.a - l1.a * l2.c) / det_ab),
                                  det_ab);
        }
        DPoint get_intersection(const Line& l2) const& {
            return get_intersection_details(l2).first;
        }

        T a, b, c;
    };
    using ILine = Line<pint>;
    using DLine = Line<pdouble>;

    template <typename T>
    struct HalfPlane : Line<T> {
        using Line<T>::Line;
        HalfPlane(const Line<T>& l) : Line<T>(l) {}

        template <typename U>
        bool has_point(const Point<U>& p) const {
            return this->a * p.x + this->b * p.y + this->c >= -EPS;
        }
    };
    using IHalfPlane = HalfPlane<pint>;
    using DHalfPlane = HalfPlane<pdouble>;

    template <typename TLine, typename TPoint>
    struct Ray {
        Ray(HalfPlane<TLine> _hp, DPoint _p, bool _sign, pdouble _cos) : hp(_hp), p(_p), sign(_sign), cos(_cos) {}
        Ray(HalfPlane<TLine> hp0, HalfPlane<TLine> ihp) : hp(hp0) {
            auto intersection = hp0.get_intersection_details(ihp);
            p = intersection.first;
            sign = std::signbit(intersection.second);
            cos = Point(hp0.b, -hp0.a).cos(Point(ihp.b, -ihp.a));
        }
        bool operator<(const Ray& rhs) const& { return (compare(rhs) == -1); }
        bool operator<=(const Ray& rhs) const& { return (compare(rhs) != 1); }
        bool operator==(const Ray& rhs) const& { return (compare(rhs) == 0); }
        bool operator>(const Ray& rhs) const& { return !(*this <= rhs); }
        bool operator>=(const Ray& rhs) const& { return !(*this < rhs); }
        bool operator!=(const Ray& rhs) const& { return !(*this == rhs); }
        /*
         * this <  rhs : -1
         * this == rhs :  0
         * this >  rhs :  1
         */
        int compare(const Ray& rhs) const& {
            if (p == rhs.p) {
                if (cos < rhs.cos + EPS) return -1;
                if (cos > rhs.cos - EPS) return 1;
                return 0;
            }
            Point<TPoint> n(hp.a, hp.b);
            Point<TPoint> v(p, rhs.p);
            return n.dot(v) < 0 ? -1 : 1;
        }

        HalfPlane<TLine> hp;
        Point<TPoint> p;
        bool sign;
        pdouble cos;
    };
    using IDRay = Ray<pint, pdouble>;

    template <typename T>
    struct Circle {
        Circle(const Point<T>& _o, const T _r_sq) : o(_o), r_sq(_r_sq), r(sqrt(r_sq)) {}
        Circle() : Circle(Point<T>(), 0) {}
        template <typename U>
        Circle(const Point<U>& a, const Point<U>& b) : // a, b are on diameter
            o(Point<T>(a, b) / 2 + a),
            r_sq((o - a).sqr_length()),
            r(sqrt(r_sq)) {}
        template <typename U>
        Circle(const Point<U>& a, const Point<U>& b, const Point<U>& c) : // a, b, c are not collinear
            o(Line<U>::get_perpendicular_bisector(a, b).get_intersection(Line<U>::get_perpendicular_bisector(a, c))),
            r_sq((o - a).sqr_length()),
            r(sqrt(r_sq)) {}
        template <typename U>
        Circle& operator /= (U divisor) {
            o /= divisor;
            r /= divisor;
            return *this;
        }
        template <typename U>
        Circle operator / (U divisor) const& { return Circle(*this) /= divisor; }
        friend std::istream& operator >> (std::istream& in, Circle& c) { return in >> c.o >> c.r; }
        friend std::ostream& operator << (std::ostream& out, Circle c) { return out << c.o << "\n" << c.r; }
        bool includes(const IPoint& P) const { return r_sq + EPS > DPoint(o, P).sqr_length(); }

        Point<T> o;
        T r_sq;
        T r;
    };
    using ICircle = Circle<pint>;
    using DCircle = Circle<pdouble>;
}

using namespace geometry::primitives;

static const pint MAX_ARG = 10000;
static const pint MAX_COORD = 2 * sqr(MAX_ARG) + 1;
static const pdouble MIN_AREA = 1e-8;

struct Polygon {
    pdouble calc_area() {
        if (!points.size()) return 0;
        const Point a = points[0];
        area = 0;
        for (size_t i = 1; i < points.size() - 1; ++i) {
            const auto& b = points[i];
            const auto& c = points[i + 1];
            area += Point(a, b).dot(Point(a, c));
        }
        area /= 2;
        return area;
    }

    void cut(const IHalfPlane& hp) {
        is_finite = true;
        std::vector<DPoint> new_points;
        std::vector<IHalfPlane> new_lines;
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

    pdouble area = 0;
    bool is_finite = false;
    std::vector<DPoint> points{ {-MAX_COORD, -MAX_COORD },
                               {+MAX_COORD, -MAX_COORD },
                               {+MAX_COORD, +MAX_COORD },
                               {-MAX_COORD, +MAX_COORD } };
    std::vector<IHalfPlane> half_planes{ {0,1,MAX_COORD},
                                        {-1,0,MAX_COORD},
                                        {0,-1,MAX_COORD},
                                        {1,0,MAX_COORD} };
};

void extend_polygons(std::vector<Polygon>& container, Polygon polygon, IHalfPlane hp) {
    polygon.cut(hp);
    if (polygon.calc_area() >= MIN_AREA) {
        container.emplace_back(std::move(polygon));
    }
}

void solve_areas() {
    size_t n;
    std::cin >> n;
    IHalfPlane hp;
    std::vector<Polygon> polygons(1);
    std::vector<Polygon> temp_polygons;
    auto p_polygons = &polygons;
    auto p_temp_polygons = &temp_polygons;
    IPoint p1, p2;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> p1 >> p2;
        IHalfPlane hp(p1, p2);
        p_temp_polygons->clear();
        for (auto& polygon : *p_polygons) {
            extend_polygons(*p_temp_polygons, polygon, hp);
            extend_polygons(*p_temp_polygons, polygon, -hp);
        }
        std::swap(p_polygons, p_temp_polygons);
    }
    std::vector<pdouble> areas;
    for (auto& polygon : *p_polygons) {
        if (polygon.is_finite) {
            pdouble area = polygon.area;
            if (area >= MIN_AREA) {
                areas.push_back(area);
            }
        }
    }
    std::sort(areas.begin(), areas.end());
    std::cout << areas.size() << '\n';
    for (auto area : areas) {
        std::cout << area << '\n';
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
    solve_areas();
    return EXIT_SUCCESS;
}
