/*
Наибольшая общая подстрока
ограничение по времени на тест 2 секунды
ограничение по памяти на тест 512 мегабайт
ввод: стандартный ввод
вывод: стандартный вывод

Найдите наибольшую общую подстроку строк s и t.

Входные данные
Первая строка входного файла содержит строку s, вторая — t (1 ≤ |s|, |t| ≤ 100000). Строки состоят из строчных латинских букв.

Выходные данные
Выведите одну строку — наибольшую общую подстроку строк s и t. В случае, если ответ не единственный, выведите минимальный лексикографически.

входные данные
bababb
zabacabba

выходные данные
aba
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

static const char FIRST_LETTER = 'a';
static const char DELIMITER = FIRST_LETTER - 1;
static const char PLACEHOLDER = DELIMITER - 1;
static const size_t VOC_SIZE = 'z' - PLACEHOLDER + 1;

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

auto get_sub_str(const std::string& str, const std::string& str2, const std::vector<size_t>& suffix_arr) {
    const auto lcp = get_lcp(str, suffix_arr);
    size_t common_substr_index = 0;
    size_t common_substr_size = 0;
    const auto str_size = str.size();
    const auto str2_pos = str_size - str2.size() - 1;
    const auto is_suf_in_str2 = [&](size_t i) { return str2_pos <= suffix_arr[i]; };
    for (size_t i = 1; i < str_size; ++i) {
        if (is_suf_in_str2(i - 1) != is_suf_in_str2(i)) {
            const size_t cur_size = lcp[i - 1];
            if (cur_size > common_substr_size) {
                common_substr_size = cur_size;
                common_substr_index = i;
            }
        }
    }
    return std::make_pair(common_substr_index, common_substr_size);
}

int main() {
#ifdef _DEBUG
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);
    std::string str1, str2;
    std::cin >> str1 >> str2;
    const auto str = str1 + DELIMITER + str2 + PLACEHOLDER;
    const auto suffix_arr = get_suffix_array(str);
    const auto [common_substr_index, common_substr_size] = get_sub_str(str, str2, suffix_arr);
    std::cout << str.substr(suffix_arr[common_substr_index], common_substr_size);
    return EXIT_SUCCESS;
}