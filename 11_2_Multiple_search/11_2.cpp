/*
Множественный поиск
ограничение по времени на тест 3 секунды
ограничение по памяти на тест 1024 мегабайта
ввод: стандартный ввод
вывод: стандартный вывод

Дан массив строк s_i и строка t. Требуется для каждой строки s_i определить, встречается ли она в t как подстрока.

Входные данные
Первая строка входного файла содержит целое число n — число элементов в s (1 ≤ n ≤ 10^6).
Следующие n строк содержат по одной строке s_i. Сумма длин всех строк из s не превосходит 10^6. 
Последняя строка входного файла содержит t (1 ≤ t ≤ 10^6). Все строки состоят из строчных латинских букв.

Выходные данные
Для каждой строки s_i выведите «YES», если она встречается в t и «NO» в противном случае. Строки нумеруются в порядке появления во входном файле.

входные данные
3
abc
abcdr
abcde
xabcdef
выходные данные
YES
NO
YES
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

static const char FIRST_LETTER = 'a';
static const char PLACEHOLDER = FIRST_LETTER - 1;
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

inline size_t bin_search(const std::string& str, const std::string& text, const std::vector<size_t>& suffix_arr) {
    size_t l = 0;
    size_t r = text.size();
    while (r - l > 1) {
        size_t m = l + (r - l) / 2;
        const auto substr = text.substr(suffix_arr[m], str.size());
		l = (substr <= str) ? m : l;
		r = (substr <= str) ? r : m;
    }
    return l;
}

int main() {
#ifdef _DEBUG
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);

    size_t n;
    std::cin >> n;

    std::vector<std::string> strs(n);
    for (auto& s : strs) {
        std::cin >> s;
    }

    std::string text;
    std::cin >> text;
    text += PLACEHOLDER;

    const auto suffix_arr = get_suffix_array(text);
    for (const auto& str : strs) {
        const size_t pos = bin_search(str, text, suffix_arr);
        std::cout << (text.substr(suffix_arr[pos], str.size()) == str ? "YES" : "NO") << "\n";
    }

    return EXIT_SUCCESS;
}