﻿/*
Theory
Probably, the best explanation of Ukkonen's algorithm:
https://stackoverflow.com/questions/9452701/ukkonens-suffix-tree-algorithm-in-plain-english/9513423#9513423

Though, it doesn't cover all details, which are described further:
https://stackoverflow.com/questions/9452701/ukkonens-suffix-tree-algorithm-in-plain-english/14580102#14580102

For easier understanding of building tree process there is also an
awesome visualizer of Ukkonen's tree step-by-step: https://brenden.github.io/ukkonen-animation/
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>

#ifdef _DEBUG
#include "memcheck_crt.h"
#endif // _DEBUG

class Tree {
public:
    static const size_t INF = std::numeric_limits<size_t>::max();

protected:
    struct Node {
        explicit Node(size_t _l) : l(_l) {}
        explicit Node(size_t _l, size_t _r) : l(_l), r(_r) {}
        size_t l = 0;
        size_t r = INF;
        size_t suff_link = 0;
        std::unordered_map<char, size_t> edges;
    };
public:
    Tree(std::string& _text) : text(_text) {}

    /*
      Build a tree. Function is online, so whenever you extend text, you can continue building a tree calling this function
      except for case when string is terminated.
    */
    void build() {
        assert(!terminated);
        nodes.reserve(text.size() * 2);
        for (; text_length < text.size(); ++text_length) {
            extend();
#ifdef _DEBUG
            //pretty_print();
#endif // _DEBUG
        }
    }

protected:
    size_t get_node_length(const Node& node) {
        return std::min(node.r, text_length + 1) - node.l;
    }

private:
    char active_edge_char() const {
        return text[active_edge_index];
    }

    void add_suffix_link(size_t node_index) {
        if (need_sl_index > 0) {
            nodes[need_sl_index].suff_link = node_index;
        }
        need_sl_index = node_index;
    }

    bool walk_down(size_t node_index) {
        auto edge_length = get_node_length(nodes[node_index]);
        if (active_node_length < edge_length) {
            return false;
        }
        active_edge_index += edge_length;
        active_node_length -= edge_length;
        active_node_index = node_index;
        return true;
    }

    void extend() {
        need_sl_index = 0;
        ++remainder;
        while (remainder) {
            if (active_node_length == 0) {
                active_edge_index = text_length;
            }
            auto& edges = nodes[active_node_index].edges;
            const auto edge_char = active_edge_char();
            const auto& search_iter_ch = edges.find(edge_char);
            if (search_iter_ch == edges.end()) {
                nodes.emplace_back(text_length);
                edges[edge_char] = nodes.size() - 1;
                add_suffix_link(active_node_index);
            } else {
                const auto edge_index = (*search_iter_ch).second;
                // Observation 1:
                // When the final suffix we need to insert is found to exist in the tree already, 
                // the tree itself is not changed at all(we only update the active point and remainder)
                if (walk_down(edge_index)) {
                    continue;
                }

                auto& edge_left_pos = nodes[edge_index].l;
                const auto edge_right_pos = edge_left_pos + active_node_length;
                // Observation 2
                // If at some point active_length is greater or equal to the length of current edge (edge_length), 
                // we move our active point down until edge_length is strictly greater than active_length.
                const auto cur_char = text[text_length];
                if (text[edge_right_pos] == cur_char) {
                    ++active_node_length;
                    // Observation 3
                    // When the symbol we want to add to the tree is already on the edge, we, according to Observation 1,
                    // update only active point and remainder, leaving the tree unchanged.
                    // BUT if there is an internal node marked as needing suffix link,
                    // we must connect that node with our current active node through a suffix link.
                    add_suffix_link(active_node_index);
                    break;
                }
                auto& mid_node = nodes.emplace_back(edge_left_pos, edge_right_pos); // Add a new middle node
                const auto mid_index = nodes.size() - 1;
                nodes[active_node_index].edges[active_edge_char()] = mid_index; // Relink active node edge to middle node
                nodes.emplace_back(text_length); // Add a new leaf node
                auto& mid_node_edges = mid_node.edges;
                mid_node_edges[cur_char] = nodes.size() - 1; // Add mid_node-new_leaf link
                edge_left_pos += active_node_length;
                mid_node_edges[text[edge_left_pos]] = edge_index; // Add mid_node-old_leaf link
                // Rule 2
                // If we create a new internal node OR make an inserter from an internal node, 
                // and this is not the first SUCH internal node at current step, 
                // then we link the previous SUCH node with THIS one through a suffix link.
                add_suffix_link(mid_index);
            }
            --remainder;
            if (active_node_index == 0 && active_node_length > 0) {
                // Rule 1
                // If after an insertion from the active node = root, the active length is greater than 0, then:
                // 1. active node is not changed
                // 2. active length is decremented
                // 3. active edge is shifted right(to the first character of the next suffix we must insert)
                --active_node_length;
                active_edge_index = text_length - remainder + 1;
            } else {
                // Rule 3
                // After an insert from the active node which is not the root node,
                // we must follow the suffix linkand set the active node to the node it points to.
                // If there is no a suffix link, set the active node to the root node.
                // Either way, active edge and active length stay unchanged.
                active_node_index = nodes[active_node_index].suff_link;
            }
        }
    }

    void pretty_print(size_t node_index, std::string pad) {
        const auto& node = nodes[node_index];
        const auto node_length = get_node_length(node);
        const std::string delim = " ";
        const std::string suff_str = node.suff_link ? "(" + std::to_string(node.suff_link) + ")" : "";
        std::cout << pad << node_index << suff_str << delim << text.substr(node.l, node_length) << "\n";
        pad = "  " + pad;
        for (const auto& [_, val] : node.edges) {
            pretty_print(val, pad);
        }
    }

    void pretty_print() {
        std::cout << text.substr(0, text_length + 1) << "\n";
        std::cout << "active index=" << active_node_index << " edge=" << active_edge_char() << " length=" << active_node_length << "\n";
        pretty_print(0, "");
        std::cout << "\n";
    }

protected:
    bool terminated = false;
    size_t need_sl_index = 0; // node, that needs a suffix link
    size_t active_node_index = 0; // index of active node
    size_t active_edge_index = 0; // index of active edge char in text
    size_t active_node_length = 0; // distance between current position and beggining of active node position
    size_t remainder = 0; // how many leafs to add to make tree completed (with active_node_length=0)
    size_t text_length = 0; // length of processed text
    std::string& text;
    std::vector<Node> nodes{ Node(0, 0) };
};


class SolverTree : public Tree {
private:
    static const char TERMINATOR = '$';
public:
    SolverTree() : Tree(text) {}

    /*
      Print a tree in a plain table
    */
    void print_table() {
        auto nodes_num = nodes.size();
        std::cout << nodes_num << " " << nodes_num - 1 << "\n";
        for (size_t i = 0; i < nodes_num; ++i) {
            const auto& node = nodes[i];
            for (const auto [_, j] : node.edges) {
                const auto& child = nodes[j];
                std::cout << i + 1 << " " << j + 1 << " " << child.l + 1 << " " << std::min(child.r, text_length) << "\n";
            }
        }
    }

    /*
      Count number of all substrings in given text
    */
    unsigned long long get_sub_sum() {
        if (!terminated) {
            terminate();
        }
        unsigned long long size = 0;
        for (const auto& node : nodes) {
            size += std::min(node.r, text_length) - node.l;
            size -= node.r == INF ? 1 : 0;
        }
        return size;
    }

    size_t get_inclusion_nums(const std::string& word) {
        auto node_index = walk_word(word);
        return node_index < INF ? inclusion_nums[node_index] : 0;
    }

    void prepare_inclusions_num() {
        terminate();
        inclusion_nums.resize(nodes.size());
        prepare_inclusions_num(0);
    }

    size_t walk_word(const std::string& word) {
        size_t node_index = 0;
        size_t ch_pos = 0;
        size_t depth = 0;
        while (ch_pos < word.size()) {
            auto& node = nodes[node_index];
            auto ch = word[ch_pos];
            if (get_node_length(node) == depth) {
                const auto& edges = node.edges;
                const auto& search_iter_ch = edges.find(ch);
                if (search_iter_ch == edges.end()) {
                    return INF;
                }
                node_index = (*search_iter_ch).second;
                depth = 0;
            } else if (text[node.l + depth] != ch) {
                return INF;
            }
            ++depth;
            ++ch_pos;
        }
        return node_index;
    }

    void reserve(size_t text_size) {
        nodes.reserve(text_size * 2);
    }

    void build_lcp_and_suffix_array() {
        if (!terminated) terminate();
        if (suffix_array.size()) return;
        suffix_array.reserve(text_length);
        lcp_array.reserve(text_length);
        node_depths.resize(nodes.size());
        min_node_index = 0;
        --text_length;
        dfs_lcp_and_suffix(0, 0);
        ++text_length;
    }

private:
    template <typename K, typename V>
    std::vector<std::pair<K, V>> get_sorted_kv_pairs(const std::unordered_map<K, V> map) {
        std::vector<std::pair<K, V>> result(map.begin(), map.end());
        std::sort(result.begin(), result.end());
        return result;
    }

    void dfs_lcp_and_suffix(size_t node_index, size_t path_length) {
        const auto& node = nodes[node_index];
        path_length += get_node_length(node);
        node_depths[node_index] = path_length;
        if (node.edges.size()) {
            for (const auto& kv : get_sorted_kv_pairs(node.edges)) {
                if (path_length < node_depths[min_node_index]) {
                    min_node_index = node_index;
                }
                dfs_lcp_and_suffix(kv.second, path_length);
            }
        } else {
            suffix_array.emplace_back(text_length + 1 - path_length);
            lcp_array.emplace_back(node_depths[min_node_index]);
            min_node_index = node_index;
        }
    }

    size_t prepare_inclusions_num(size_t node_index) {
        size_t inclusion_num = 0;
        const auto& node = nodes[node_index];
        if (node.r != INF) {
            for (const auto [_, edge_num] : node.edges) {
                inclusion_num += prepare_inclusions_num(edge_num);
            }
        } else {
            inclusion_num = 1;
        }
        inclusion_nums[node_index] = inclusion_num;
        return inclusion_num;
    }

    /*
        Force completing tree by adding a unique char to the end
        After this online additive building becomes a nonsence operation
    */
    void terminate() {
        assert(text.back() != TERMINATOR);
        text += TERMINATOR;
        build();
        terminated = true;
    }

public:
    std::string text;
    std::vector<size_t> lcp_array;
    std::vector<size_t> suffix_array;
private:
    size_t min_node_index;
    std::vector<size_t> node_depths;
    std::vector<size_t> inclusion_nums;
};


void solve_suffix_tree() {
    SolverTree utree;
    std::cin >> utree.text;
    utree.build();
    utree.print_table();
}

void solve_substrings_amount() {
    SolverTree utree;
    std::cin >> utree.text;
    utree.build();
    std::cout << utree.get_sub_sum();
}

void solve_multiple_search() {
    int n;
    std::cin >> n;
    std::vector<std::string> words(n);
    for (auto& word : words) {
        std::cin >> word;
    }
    SolverTree utree;
    std::cin >> utree.text;
    utree.build();
    utree.prepare_inclusions_num();
    for (auto& word : words) {
        std::cout << utree.get_inclusion_nums(word) << "\n";
    }
}

void solve_nenokku() {
    SolverTree utree;
    utree.reserve(100000);
    char action;
    std::string line;
    std::string word;
    const size_t w_start = 2;
    while (std::getline(std::cin, line)) {
        action = line[0];
        if (action == '?') {
            if (line.size() <= w_start) {
                std::cout << "YES\n";
                continue;
            }
            word.resize(line.size() - w_start);
            std::transform(line.begin() + w_start, line.end(), word.begin(), tolower);
            std::cout << ((utree.walk_word(word) != Tree::INF) ? "YES\n" : "NO\n");
        } else {
            if (line.size() <= w_start) {
                continue;
            }
            word.resize(line.size() - w_start);
            std::transform(line.begin() + w_start, line.end(), word.begin(), tolower);
            utree.text += word;
            utree.build();
        }
    }
}

void solve_suffix_array() {
    SolverTree utree;
    std::cin >> utree.text;
    utree.build_lcp_and_suffix_array();
    for (size_t i = 1; i < utree.suffix_array.size(); ++i) {
        std::cout << utree.suffix_array[i] + 1 << " ";
    }
    std::cout << '\n';

    for (size_t i = 2; i < utree.lcp_array.size(); ++i) {
        std::cout << utree.lcp_array[i] << " ";
    }
}


int main() {
#ifdef _DEBUG
    ENABLE_CRT;
    std::ifstream input_stream("input.txt");
    std::cin.rdbuf(input_stream.rdbuf());
#endif // _DEBUG
    std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);

    //solve_suffix_tree();
    //solve_substrings_amount();
    //solve_multiple_search();
    //solve_nenokku();
    solve_suffix_array();

    return EXIT_SUCCESS;
}
