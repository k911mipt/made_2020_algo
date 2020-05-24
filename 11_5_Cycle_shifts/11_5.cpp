/*
Циклические сдвиги
ограничение по времени на тест 2 секунды
ограничение по памяти на тест 512 мегабайт
ввод: стандартный ввод
вывод: стандартный вывод

k-м циклическим сдвигом строки S называется строка, полученная перестановкой k первых символов строки S в конец строки.
Рассмотрим все различные циклические сдвиги строки S и отсортируем их по возрастанию. Требуется вычислить i-ю строчку этого массива.
Например, для строки abacabac существует четыре различных циклических сдвига: 
нулевой (abacabac), первый (bacabaca), второй (acabacab) и третий (cabacaba). 
После сортировки по возрастанию получится такой массив: abacabac, acabacab, bacabaca, cabacaba.

Входные данные
В первой строке входного файла записана строка S, длиной не более 100000 символов с ASCII-кодами от 32 до 126. 
Во второй строке содержится единственное целое число k (1 ≤ k ≤ 100000).

Выходные данные
В выходной файл выведите k-й по возрастанию циклический сдвиг строки S, или слово IMPOSSIBLE, если такого сдвига не существует.

входные данные
abacabac
4
выходные данные
cabacaba

входные данные
abacabac
5
выходные данные
IMPOSSIBLE
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

static const char FIRST_LETTER = static_cast<char>(32);
static const char PLACEHOLDER = FIRST_LETTER - 1;
static const size_t VOC_SIZE = static_cast<char>(126) - PLACEHOLDER + 1;

size_t inline char_addr(char ch) {
    return ch - PLACEHOLDER;
}

std::vector<size_t> get_suffix_array(const std::string& str) {
    const auto n = str.length ();
    size_t count[VOC_SIZE] = { 0 };
    std::vector<size_t> paddr(n);
    
    // Sort first prefix letters
    for (const auto ch : str) 
        ++count[char_addr(ch)]; 
    for (size_t i = 1; i < VOC_SIZE; ++i)
        count[i] += count[i - 1];
    for (size_t i = 0; i < n; ++i)
        paddr[--count[char_addr(str[i])]] = i;

    size_t classes_count = 1;
    std::vector<size_t> classes(n);
    classes[paddr[0]] = 0;
    for (int i = 1; i < n; ++i) {
        classes_count += (str[paddr[i]] != str[paddr[i - 1]]) ? 1 : 0;
        classes[paddr[i]] = classes_count - 1;
    }

    // Sort pairs
    std::vector<size_t> pn(n);
    std::vector<size_t> cn(n);
    for (size_t k = 0, sub_len = 1ull << k; sub_len < n; ++k, sub_len = 1ull << k) {
        std::vector<size_t> cnt(classes_count);
        for (size_t i = 0; i < n; ++i) 
            pn[i] = ((paddr[i] < sub_len) ? n : 0) + paddr[i] - sub_len;
        for (size_t i = 0; i < n; ++i)
            ++cnt[classes[pn[i]]];
        for (size_t i = 1; i < classes_count; ++i)
            cnt[i] += cnt[i - 1];
        for (size_t i = n - 1; i < n; --i)
            paddr[--cnt[classes[pn[i]]]] = pn[i];
        cn[paddr[0]] = 0;
        classes_count = 1;
        for (size_t i = 1; i < n; ++i) {
            size_t mid1 = (paddr[i] + sub_len) % n;
            size_t mid2 = (paddr[i - 1] + sub_len) % n;
            classes_count += (classes[paddr[i]] != classes[paddr[i - 1]] || classes[mid1] != classes[mid2]) ? 1 : 0;
            cn[paddr[i]] = classes_count - 1;
        }
        std::copy(cn.begin(), cn.end(), classes.begin());
    }

    return paddr;
}

std::vector <size_t> get_suffix_pos(const std::vector<size_t>& suffix_array) {
    std::vector<size_t> suffix_positions(suffix_array.size());
    for (size_t i = 0; i < suffix_array.size(); ++i) {
        suffix_positions[suffix_array[i]] = i;
    }
    return suffix_positions;
}

std::vector<size_t> get_lcp(const std::string& str, const std::vector<size_t>& suffix_array) {
    const auto n = str.size();
    std::vector<size_t> result(n);
    size_t cur = 0;
    const auto pos = get_suffix_pos(suffix_array);
    for (size_t i = 0; i < n - 1; ++i) {
        const auto& posi = pos[i];
        cur -= (cur > 0) ? 1 : 0;
        if (posi == n - 1) {
            result[n - 1] = 0;
            cur = 0;
        } else {
            for (auto j = suffix_array[posi + 1]; std::max(i + cur, j + cur) < n && str[i + cur] == str[j + cur]; ++cur);
        }
        result[pos[i]] = cur;
    }
    return result;
}

int main() {
#ifdef _DEBUG
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    std::string str;
    int expected_cycles;
    std::cin >> str >> expected_cycles;
    str += str + PLACEHOLDER;
    const auto suffix_arr = get_suffix_array(str);
    const auto lcp = get_lcp(str, suffix_arr);

    const auto str_size = str.size();
    int cycle_count = 0;
    for (size_t i = 1; i < str_size; ++i) {
        const auto lcp_val = lcp[i];
        const auto suffix_size = str_size - suffix_arr[i] - 1;
        cycle_count += ((lcp_val < suffix_size) ? 1 : 0);
        if (cycle_count == expected_cycles) {
            std::cout << str.substr(suffix_arr[i], str_size / 2);
            return EXIT_SUCCESS;
        }
    }
    std::cout << "IMPOSSIBLE";
    return EXIT_SUCCESS;
}

