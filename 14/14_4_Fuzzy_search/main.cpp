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
        void prepare_polynom(size_t max_n_size) {
            polynom.resize(max_n_size);
            std::copy(digits.begin(), digits.begin() + used_size, polynom.coeffs.begin());
            std::fill(polynom.coeffs.begin() + used_size, polynom.coeffs.end(), 0);
        }
        Bigint operator *=(Bigint& rhs) {
            auto& lhs = *this;
            const auto max_n_size = calc_max_size(rhs.used_size);
            lhs.prepare_polynom(max_n_size);
            rhs.prepare_polynom(max_n_size);
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

void solve_fuzzy_search() {
    size_t Slength, Plength, k;
    std::cin >> Slength >> Plength >> k;
    std::string sS, sP;
    std::cin >> sS >> sP;
    Bigint<1> S, P;
    S.resize(Slength);
    P.resize(Plength);
    auto& Sdigits = S.digits;
    auto& Pdigits = P.digits;
    std::vector<int> Csum(Slength);
    const std::vector<char> chars = { 'A', 'C', 'G', 'T' };
    const size_t zero = 0;
    for (auto ch : chars) {
        S.resize(Slength);
        std::fill(Sdigits.begin(), Sdigits.end(), 0);
        for (size_t i = 0; i < Slength; ++i) {
            if (sS[i] == ch) {
                Sdigits[std::max(i, k) - k] += 1;
                if (i + k + 1 < Slength) {
                    Sdigits[i + k + 1] -= 1;
                }
            }
        }
        for (size_t i = 1; i < Slength; ++i) {
            Sdigits[i] += Sdigits[i - 1];
        }
        for (size_t i = 0; i < Slength; ++i) {
            Sdigits[i] = Sdigits[i] > 0;
        }
        for (size_t i = 0; i < Plength; ++i) {
            Pdigits[Plength - i - 1] = sP[i] == ch;
        }
        auto& C = S;
        C *= P;
        auto& Cdigits = C.digits;
        for (size_t k = 0; k < Slength; ++k) {
            Csum[k] += Cdigits[Plength - 1 + k];
        }
    }
    size_t entries = 0;
    for (size_t i = 0; i < Slength; ++i) {
        entries += Csum[i] == Plength;
    }
    std::cout << entries;
}

int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    solve_fuzzy_search();
    return EXIT_SUCCESS;
}
