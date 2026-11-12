// Counts how often each word occurs in its input.
//   ./wordcount < some_text_file
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

int main() {
    // [count]
    std::unordered_map<std::string, int> count;
    std::string word;
    while (std::cin >> word) {
        ++count[word];   // inserts word with count 0 if it is new
    }
    // [/count]

    // [report]
    // A hash table has no useful order, so copy the entries into a
    // vector and sort that: most frequent first, ties alphabetically.
    std::vector<std::pair<std::string, int>> entries(count.begin(), count.end());
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) {
                  if (a.second != b.second) return a.second > b.second;
                  return a.first < b.first;
              });
    for (std::size_t i = 0; i < entries.size() && i < 5; ++i) {
        std::cout << entries[i].second << ' ' << entries[i].first << '\n';
    }
    // [/report]
}
