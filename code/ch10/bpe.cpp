// A miniature byte-pair-encoding trainer, to show the data structures
// involved in learning a tokenizer's vocabulary.
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

// [bpe]
using Word = std::vector<std::string>;   // a word as a sequence of tokens

// Count every adjacent pair of tokens across the corpus.
std::map<std::pair<std::string, std::string>, int> count_pairs(
    const std::vector<std::pair<Word, int>>& corpus) {
    std::map<std::pair<std::string, std::string>, int> counts;
    for (const auto& [word, freq] : corpus) {
        for (std::size_t i = 0; i + 1 < word.size(); ++i) {
            counts[{word[i], word[i + 1]}] += freq;
        }
    }
    return counts;
}

// Replace every occurrence of the pair (a, b) by the single token a+b.
void merge_pair(std::vector<std::pair<Word, int>>& corpus, const std::string& a,
                const std::string& b) {
    for (auto& [word, freq] : corpus) {
        Word merged;
        for (std::size_t i = 0; i < word.size(); ++i) {
            if (i + 1 < word.size() && word[i] == a && word[i + 1] == b) {
                merged.push_back(a + b);
                ++i;   // skip the second half of the pair
            } else {
                merged.push_back(word[i]);
            }
        }
        word = std::move(merged);
    }
}
// [/bpe]

int main() {
    // Each word, split into characters, with its frequency in the corpus.
    // "_" marks the end of a word.
    std::vector<std::pair<Word, int>> corpus{
        {{"l", "o", "w", "_"}, 5},
        {{"l", "o", "w", "e", "r", "_"}, 2},
        {{"n", "e", "w", "e", "s", "t", "_"}, 6},
        {{"w", "i", "d", "e", "s", "t", "_"}, 3},
    };

    for (int step = 1; step <= 8; ++step) {
        auto counts = count_pairs(corpus);
        std::pair<std::string, std::string> best;
        int best_count = 0;
        for (const auto& [pair, n] : counts) {
            if (n > best_count) {
                best = pair;
                best_count = n;
            }
        }
        merge_pair(corpus, best.first, best.second);
        std::cout << "merge " << step << ": " << best.first << " + " << best.second << " -> "
                  << best.first + best.second << "   (seen " << best_count << " times)\n";
    }
    std::cout << "\nthe corpus, tokenized:\n";
    for (const auto& [word, freq] : corpus) {
        for (const std::string& tok : word) std::cout << tok << ' ';
        std::cout << "  x" << freq << '\n';
    }
}
