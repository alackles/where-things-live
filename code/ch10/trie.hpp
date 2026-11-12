// Trie: a set of lowercase words, organized by shared prefixes.
#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class Trie {
    // [node]
    struct Node {
        bool is_word = false;                          // does a word end here?
        std::array<std::unique_ptr<Node>, 26> next;    // one child per letter
    };

    Node root_;
    std::size_t words_ = 0;

    static std::size_t slot(char c) {
        assert(c >= 'a' && c <= 'z');
        return static_cast<std::size_t>(c - 'a');
    }
    // [/node]

    // [walk]
    // Follow the path spelled by s. Returns the node reached, or nullptr
    // if the path leaves the trie.
    const Node* walk(const std::string& s) const {
        const Node* n = &root_;
        for (char c : s) {
            n = n->next[slot(c)].get();
            if (n == nullptr) return nullptr;
        }
        return n;
    }
    // [/walk]

    static void collect(const Node* n, std::string& prefix, std::size_t limit,
                        std::vector<std::string>& out) {
        if (out.size() >= limit) return;
        if (n->is_word) out.push_back(prefix);
        for (std::size_t i = 0; i < 26; ++i) {
            if (n->next[i]) {
                prefix.push_back(static_cast<char>('a' + i));
                collect(n->next[i].get(), prefix, limit, out);
                prefix.pop_back();
            }
        }
    }

public:
    std::size_t size() const { return words_; }

    // [ops]
    void insert(const std::string& word) {
        Node* n = &root_;
        for (char c : word) {
            std::unique_ptr<Node>& child = n->next[slot(c)];
            if (!child) child = std::make_unique<Node>();   // extend the path
            n = child.get();
        }
        if (!n->is_word) {
            n->is_word = true;
            ++words_;
        }
    }

    bool contains(const std::string& word) const {
        const Node* n = walk(word);
        return n != nullptr && n->is_word;
    }

    bool has_prefix(const std::string& prefix) const {
        return walk(prefix) != nullptr;
    }

    // Up to `limit` words that begin with prefix, in alphabetical order.
    std::vector<std::string> complete(const std::string& prefix, std::size_t limit) const {
        std::vector<std::string> out;
        if (const Node* n = walk(prefix)) {
            std::string path = prefix;
            collect(n, path, limit, out);
        }
        return out;
    }
    // [/ops]
};
