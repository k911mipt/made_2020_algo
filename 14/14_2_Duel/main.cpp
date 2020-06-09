#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <algorithm>
#include <complex>
#include <cmath>
#include <cassert>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

namespace fourier {
    static const long double PI = std::atan(1.0) * 4;

    using base = std::complex<double>;
    using vcd = std::vector<base>;

    class Polynom {
    public:
        Polynom& operator*=(Polynom& rhs) {
            auto& lhs = *this;
            const auto n = lhs.used_size;
            assert(n == rhs.used_size);
            lhs.fft(false);
            rhs.fft(false);
            auto& lhs_coeffs = lhs.coeffs;
            auto& rhs_coeffs = rhs.coeffs;
            for (size_t i = 0; i < n; ++i) {
                lhs_coeffs[i] *= rhs_coeffs[i];
            }
            lhs.fft(true);
            return lhs;
        }
        void fft(bool invert) {
            size_t n = coeffs.size();
            for (size_t i = 1, j = 0; i < n; ++i) {
                size_t bit = n >> 1;
                for (; j >= bit; bit >>= 1) {
                    j -= bit;
                }
                j += bit;
                if (i < j) {
                    std::swap(coeffs[i], coeffs[j]);
                }
            }
            for (size_t length = 2; length <= n; length <<= 1) {
                const double angle = 2 * PI / length * (invert ? -1 : 1);
                base wlen(cos(angle), sin(angle));
                for (size_t i = 0; i < n; i += length) {
                    base w(1);
                    for (size_t j = 0; j < length / 2; ++j) {
                        base u = coeffs[i + j], v = coeffs[i + j + length / 2] * w;
                        coeffs[i + j] = u + v;
                        coeffs[i + j + length / 2] = u - v;
                        w *= wlen;
                    }
                }
            }
            if (invert) {
                for (size_t i = 0; i < n; ++i) {
                    coeffs[i] /= n;
                }
            }
        }
        void resize(const size_t new_size) {
            reserve(new_size);
            used_size = new_size;
        }
        void reserve(const size_t capacity) {
            if (capacity > coeffs.size()) {
                coeffs.resize(capacity);
            }
        }

        size_t used_size = 0;
        vcd coeffs;
    };

    template <const size_t BASE>
    class Bigint {
        using T = int;
    public:
        Bigint() = default;
        friend std::istream& operator>>(std::istream& in, Bigint& number) {
            std::string s;
            in >> s;
            number.read(s);
            return in;
        }
        friend std::ostream& operator<<(std::ostream& out, const Bigint& number) {
            const auto& digits = number.digits;
            auto it = digits.rbegin();
            auto end = digits.rend();
            for (; it < end && *it == 0; ++it);
            if (it == end) {
                return out << '0';
            }
            if (!number.sign) {
                out << "-";
            }
            for (; it < end; ++it) {
                out << *it;
            }
            return out;
        }
        Bigint operator *=(Bigint& rhs) {
            auto& lhs = *this;
            const auto max_n_size = calc_max_size(rhs.used_size);
            lhs.polynom.resize(max_n_size);
            rhs.polynom.resize(max_n_size);
            std::copy(lhs.digits.begin(), lhs.digits.begin() + used_size, std::begin(lhs.polynom.coeffs));
            std::copy(rhs.digits.begin(), rhs.digits.begin() + rhs.used_size, std::begin(rhs.polynom.coeffs));
            lhs.polynom *= rhs.polynom;
            lhs.normalize_coeffs();
            sign = lhs.sign == rhs.sign;
            return lhs;
        }
        Bigint operator * (Bigint& rhs) const& {
            Bigint lhs(*this);
            lhs *= rhs;
            return lhs;
        }
        size_t calc_max_size(size_t rhs_used_size) {
            const auto max_n_size = std::max(used_size, rhs_used_size);
            size_t n = 1;
            while (n < max_n_size) {
                n <<= 1;
            }
            n <<= 1;
            return n;
        }
        template <size_t D = BASE, typename std::enable_if<D == 1, void>::type* = nullptr>
        void normalize_coeffs() {
            const auto& pol = polynom.coeffs;
            digits.resize(pol.size());
            for (size_t i = 0; i < digits.size(); ++i) {
                digits[i] = static_cast<T>(floor(pol[i].real() + 0.5));
            }
        }
        template <size_t D = BASE, typename std::enable_if<D != 1, void>::type* = nullptr>
        void normalize_coeffs() {
            const auto& pol = polynom.coeffs;
            digits.resize(pol.size());
            T carry = 0;
            for (size_t i = 0; i < digits.size(); ++i) {
                digits[i] = static_cast<T>(floor(pol[i].real() + 0.5)) + carry;
                carry = digits[i] / BASE;
                digits[i] %= BASE;
            }
        }
        void resize(const size_t new_size) {
            reserve(new_size);
            used_size = new_size;
        }
        void reserve(const size_t capacity) {
            if (capacity > digits.size()) {
                digits.resize(capacity);
            }
        }
        void read(const std::string& s) {
            const auto n = s.size();
            sign = s[0] != '-';
            resize(n + static_cast<size_t>(sign) - 1);
            for (size_t i_d = digits.size() - 1, i_s = n - i_d - 1; i_s < n; --i_d, ++i_s) {
                digits[i_d] = s[i_s] - '0';
            }
        }

        bool sign;
        size_t used_size = 0;
        std::vector<T> digits;
        Polynom polynom;
    };

}
using namespace fourier;

void solve_duel() {
    Bigint<1> pol;
    std::cin >> pol;
    const auto pol2 = pol * pol;
    const auto& pold = pol.digits;
    const auto& pol2d = pol2.digits;
    uint64_t duels_num = 0;
    for (size_t i = 0; i < pold.size(); ++i) {
        duels_num += pold[i] * (pol2d[2 * i] - 1) / 2;
    }
    std::cout << duels_num;
}

int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    solve_duel();
    return EXIT_SUCCESS;
}
