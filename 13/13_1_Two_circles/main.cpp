#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <cmath>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

using pint = int64_t;

template <typename T>
T sqr(T v) {
    return v * v;
}

inline void out_lin_shift(pint x1, pint y1, double x, double y) {
    std::cout << x1 + x << " " << y1 + y << "\n";
}

static pint const PINT_MAX = std::numeric_limits<pint>::max();
void solve_two_circles() {
    std::cout.precision(11);
    size_t n;
    std::cin >> n;
    pint x1, y1, x2, y2, r1, r2;
    double x0, y0;
    double ax, ay, bx, by;
    for (size_t i = 0; i < n; ++i) {
        std::cin >> x1 >> y1 >> r1;
        std::cin >> x2 >> y2 >> r2;
        x2 = x2 - x1;
        y2 = y2 - y1;
        if (x2 == 0 && y2 == 0) { // same centers
            if (r1 == r2) { // Inf points
                std::cout << "3\n";
                continue;
            } else { // Zero points
                std::cout << "0\n";
                continue;
            }
        }
        pint A = -2 * x2;
        pint B = -2 * y2;
        pint r_sq = sqr(r1);
        pint C = sqr(x2) + sqr(y2) - sqr(r2) + r_sq;
        pint a_sq = sqr(A);
        pint b_sq = sqr(B);
        pint a_sq_b_sq = a_sq + b_sq;
        pint c_sq = sqr(C);
        pint r_sq_a_sq_b_sq = r_sq * a_sq_b_sq;
        if (c_sq > r_sq_a_sq_b_sq) { // Zero points
            std::cout << "0\n";
            continue;
        }
        double CC = static_cast<double>(C);
        x0 = -A * CC / a_sq_b_sq;
        y0 = -B * CC / a_sq_b_sq;
        if (c_sq == r_sq_a_sq_b_sq) { // One point
            std::cout << "1\n";
            out_lin_shift(x1, y1, x0, y0);
            continue;
        }
        // Two points
        double csq_div_asq_p_b_sq = static_cast<double>(c_sq) / a_sq_b_sq;
        double d_sq = r_sq - csq_div_asq_p_b_sq;
        double mult = sqrt(d_sq / a_sq_b_sq);
        ax = x0 + B * mult;
        ay = y0 - A * mult;
        bx = x0 - B * mult;
        by = y0 + A * mult;
        double OH = sqrt(csq_div_asq_p_b_sq);
        double HP = sqrt(d_sq);
        std::cout << "2\n";
        out_lin_shift(x1, y1, x0, y0);
        std::cout << OH << " " << HP << "\n";
        out_lin_shift(x1, y1, ax, ay);
        out_lin_shift(x1, y1, bx, by);
    }
}


int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    solve_two_circles();
    return EXIT_SUCCESS;
}
