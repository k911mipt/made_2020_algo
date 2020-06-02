#include <iostream>
#include <fstream>
#include <cassert>
#include <random>
#include <functional>
#include <numeric>
#include <ctime>
#include <cstdlib>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

using pint = int64_t;
using pdouble = long double;
static const pdouble EPS = 1e-9;

namespace rmath {
    using val_t = pdouble;
    using buf_type = std::vector<val_t>;
    using row_it = buf_type::iterator;
    using row_cit = buf_type::const_iterator;

    /*
        Common matrix operations are taken from https://www.geeksforgeeks.org/adjoint-inverse-matrix/
    */
    class Matrix {
    public:
        Matrix(size_t _rows, size_t _cols) : rows(_rows), cols(_cols), size(_rows* _cols), buffer(size), row_b(_rows) {
            auto p = buffer.begin();
            for (size_t i = 0; i < _rows; ++i, p += _cols) {
                row_b[i] = p;
            }
        }
        row_it operator[](size_t i)& { return row_b[i]; }
        row_cit operator[](size_t i) const& { return row_b[i]; }
        void assign(std::initializer_list<val_t> lst) {
            auto it = lst.begin();
            for (size_t i = 0; i < lst.size(); ++i, ++it) {
                buffer[i] = *it;
            }
        }
        Matrix transpose() const& {
            const auto& src = *this;
            Matrix tgt(cols, rows);
            for (size_t i = 0; i < rows; ++i) {
                for (size_t j = 0; j < cols; ++j) {
                    tgt[j][i] = src[i][j];
                }
            }
            return tgt;
        }
        Matrix prod(const Matrix& B) const& {
            auto& A = *this;
            assert(A.cols == B.rows);
            size_t M = A.rows;
            size_t K = A.cols;
            size_t N = B.cols;
            Matrix C(M, N);
            for (size_t i = 0; i < M; ++i) {
                for (size_t j = 0; j < N; ++j) {
                    C[i][j] = 0;
                    for (size_t k = 0; k < K; ++k) {
                        C[i][j] += A[i][k] * B[k][j];
                    }
                }
            }
            return C;
        }
        void print() {
            for (const auto& row : row_b) {
                for (size_t j = 0; j < cols; ++j) {
                    std::cout << row[j] << " ";
                }
                std::cout << "\n";
            }
        }
        val_t det() const& { return determinant((*this), cols); }
        Matrix inv() const& {
            size_t N = cols;
            Matrix result(N, N);
            inverse(result);
            return result;
        }
    private:
        void get_cofactor(const Matrix& mat, Matrix& temp, size_t p, size_t q, size_t n) const {
            size_t i = 0, j = 0;
            for (size_t row = 0; row < n; row++) {
                for (size_t col = 0; col < n; col++) {
                    if (row != p && col != q) {
                        temp[i][j++] = mat[row][col];
                        if (j == n - 1) {
                            j = 0;
                            ++i;
                        }
                    }
                }
            }
        }
        val_t determinant(const Matrix& mat, size_t n) const {
            val_t D = 0; // Initialize result 
            if (n == 1) return mat[0][0];
            size_t N = cols;
            Matrix temp(N, N); // To store cofactors 
            int sign = 1;  // To store sign multiplier 
            for (int f = 0; f < n; f++) { // Iterate for each element of first row 
                get_cofactor(mat, temp, 0, f, n); // Getting Cofactor of mat[0][f] 
                D += sign * mat[0][f] * determinant(temp, n - 1);
                sign = -sign; // terms are to be added with alternate sign 
            }
            return D;
        }
        Matrix adjoint() const& {
            const Matrix& A = (*this);
            size_t N = A.cols;
            Matrix adj(N, N);
            if (N == 1) {
                adj[0][0] = 1;
                return adj;
            }
            int sign = 1;
            Matrix temp(N, N);
            for (size_t i = 0; i < N; i++) {
                for (size_t j = 0; j < N; j++) {
                    get_cofactor(A, temp, i, j, N);
                    sign = ((i + j) % 2 == 0) ? 1 : -1;
                    adj[j][i] = (sign) * (determinant(temp, N - 1));
                }
            }
            return adj;
        }
        bool inverse(Matrix& res) const& {
            const Matrix& A = (*this);
            size_t N = A.cols;
            val_t det = determinant(A, N);
            if (det == 0) {
                return false;
            }
            auto adj = adjoint();
            for (size_t i = 0; i < N; i++)
                for (size_t j = 0; j < N; j++)
                    res[i][j] = adj[i][j] / static_cast<val_t>(det);
            return true;
        }
    private:
        size_t rows;
        size_t cols;
        size_t size;
        buf_type buffer;
        std::vector<row_it> row_b; //row buffer
    };

    Matrix linalg_solve(const Matrix& A, const Matrix& y) {
        auto AT = A.transpose();
        auto invAT_A = AT.prod(A).inv();
        return invAT_A.prod(AT).prod(y);
    }
}
using namespace rmath;

struct Point {
    Point(pdouble _x, pdouble _y) : x(_x), y(_y) {}
    Point() : Point(0, 0) {}
    Point(Point from, Point to) : Point(to.x - from.x, to.y - from.y) {}
    bool operator==(const Point& rhs) const& { return (abs(x - rhs.x) < EPS) && (abs(y - rhs.y) < EPS); }
    bool operator!=(const Point& rhs) const& { return !(*this == rhs); }
    Point operator-() const& { return Point(-x, -y); }
    friend std::istream& operator >> (std::istream& in, Point& p) { return in >> p.x >> p.y; }
    pdouble sqr_length() const& { return x * x + y * y; }
    pdouble dot(const Point& b) const& { return x * b.y - y * b.x; }
    pdouble scalar(const Point& b) const& { return x * b.x + y * b.y; }
    pdouble cos(const Point& b) const& {
        const auto& a = (*this);
        pdouble la = a.sqr_length();
        pdouble lb = b.sqr_length();
        return a.scalar(b) / sqrt(la * lb);
    }

    pdouble x, y;
};

struct Line {
    Line(pint _a, pint _b, pint _c) : a(_a), b(_b), c(_c) {}
    std::pair<Point, bool> get_intersection(const Line& l2) const& {
        const auto& l1 = *this;
        assert(l1.a != l2.a || l1.b != l2.b);
        pdouble det_ab = static_cast<pdouble>(l1.a * l2.b - l1.b * l2.a);
        Point bc(l1.b, l1.c);
        Point ca(l1.c, l1.a);
        return std::make_pair(Point((l1.b * l2.c - l1.c * l2.b) / det_ab,
            (l1.c * l2.a - l1.a * l2.c) / det_ab), std::signbit(det_ab));
    }
    bool operator==(const Line& l2) const& {
        if (a != 0) {
            return ((abs(1.0 * l2.b * a / l2.a - b) < EPS) &&
                (abs(1.0 * l2.c * a / l2.a - c) < EPS));
        } else if (b != 0) {
            return ((abs(1.0 * l2.a * b / l2.b - a) < EPS) &&
                (abs(1.0 * l2.c * b / l2.b - c) < EPS));
        } else if (c != 0) {
            return ((abs(1.0 * l2.a * c / l2.c - a) < EPS) &&
                (abs(1.0 * l2.b * c / l2.c - b) < EPS));
        }
        return false;
    }
    bool is_collinear(const Line& l2) const& {
        if (a != 0) {
            return (abs(1.0 * l2.b * a / l2.a - b) < EPS);
        } else if (b != 0) {
            return (abs(1.0 * l2.a * b / l2.b - a) < EPS);
        }
        return false;
    }

    pint a, b, c;
};

struct HalfPlane : Line {
    HalfPlane(pint _a, pint _b, pint _c) : Line(_a, _b, _c) {}
    HalfPlane() : HalfPlane(0, 0, 0) {}
    HalfPlane(Point a, Point b) : HalfPlane(a.y - b.y, b.x - a.x, a.dot(b)) {}
    HalfPlane operator-() const& { return { -a, -b, -c }; }
    bool has_point(const Point& p) const& { return a * p.x + b * p.y + c >= -EPS; }
};

struct Ray {
    Ray(HalfPlane _hp, Point _p, bool _sign, pdouble _cos) : hp(_hp), p(_p), sign(_sign), cos(_cos) {}
    Ray(HalfPlane hp0, HalfPlane ihp) : hp(hp0) {
        auto intersection = hp0.get_intersection(ihp);
        p = intersection.first;
        sign = intersection.second;
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
        Point n(hp.a, hp.b);
        Point v(p, rhs.p);
        return n.dot(v) < 0 ? -1 : 1;
    }

    HalfPlane hp;
    Point p;
    bool sign;
    pdouble cos;
};

struct Polygon {
    Polygon(size_t _n) : n(_n), points(n), indexes(n), hps(n) {}

    Point find_linreg_point() {
        Matrix X(n, 2);
        Matrix y(n, 1);
        for (size_t i = 0; i < n; ++i) {
            const auto& hp = hps[i].first;
            X[i][0] = hp.a;
            X[i][1] = hp.b;
            y[i][0] = -hp.c;
        }
        auto w = linalg_solve(X, y);
        return Point(w[0][0], w[1][0]);
    }

    Point find_min_y_polygon_point() {
        Point min_p = points[0];
        for (size_t i = 1; i < points.size(); ++i) {
            if (points[i].y < min_p.y) {
                min_p = points[i];
            }
        }
        return min_p;
    }

    bool find_beam_point(size_t i, size_t step) {
        const auto& hp0 = hps[i].first;
        size_t i_first = hps[i].second;
        size_t i_second = (i_first + step) % n;
        Ray first_forward(hp0, points[i_first], true, -1);
        Ray last_backward(hp0, points[i_second], false, 1);
        for (size_t j = 0; j < i; ++j) {
            const auto& hp = hps[j].first;
            Ray b(hp0, hp);
            if (b.sign && b > first_forward) {
                first_forward = b;
            } else if (!b.sign && b < last_backward) {
                last_backward = b;
            }
            if (first_forward >= last_backward) {
                return false;
            }
        }
        const auto p_l = first_forward.p;
        const auto p_r = last_backward.p;
        min_p = p_l.y < p_r.y ? p_l : p_r;
        return true;
    }

    bool check_base_alive(size_t num_cuts) {
        const size_t step = num_cuts + 1;
        pepare_halfplanes(step);
        for (size_t i = 0; i < n; ++i) {
            if (!hps[i].first.has_point(min_p) && !find_beam_point(i, step)) {
                return false;
            }
        }
        return true;
    }

    void pepare_halfplanes(size_t step) {
        for (size_t i = 0; i < n; ++i) {
            auto idx = indexes[i];
            hps[idx].first = { points[i], points[(i + step) % n] };
            hps[idx].second = i;
        }
    }

    void prepare() {
        std::reverse(points.begin(), points.end());
        std::iota(indexes.begin(), indexes.end(), 0);
        srand(time(0));
        auto seed = rand();
        std::shuffle(indexes.begin(), indexes.end(), std::default_random_engine(seed));
        // A little fun. choose start point with linear regression method or just find a minimum polygon point
        if (seed % 2) {
            min_p = find_min_y_polygon_point();
        } else {
            size_t exp_half = get_biased_half(n);
            if (n - exp_half >= 3) {
                pepare_halfplanes(exp_half);
                min_p = find_linreg_point();
            }
        }
    }

    size_t get_biased_half(size_t val) {
        return (val + 1) / 2 - 1;
    }

    size_t n;
    Point min_p;
    std::vector<size_t> indexes;
    std::vector<Point> points;
    std::vector<std::pair<HalfPlane, size_t>> hps;
};

size_t bin_search(size_t l, size_t r, std::function<bool(size_t)> predicate) {
    while (r != l) {
        size_t m = l + (r - l) / 2;
        if (predicate(m)) {
            l = m + 1;
        } else {
            r = m;
        }
    }
    return r;
}

size_t solve_jungle_base() {
    size_t n;
    std::cin >> n;
    Polygon polygon(n);
    for (auto& point : polygon.points) {
        std::cin >> point;
    }
    polygon.prepare();
    return bin_search(1, polygon.get_biased_half(n), [&polygon](size_t num_cuts) { return polygon.check_base_alive(num_cuts); });
}

int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    std::cout << solve_jungle_base() << "\n";
    return EXIT_SUCCESS;
}
