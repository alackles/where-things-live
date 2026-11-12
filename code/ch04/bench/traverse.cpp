// Sum n ints stored three ways: contiguously, in list nodes laid out in
// memory order, and in list nodes scattered through memory.
//   g++ -std=c++20 -O2 ch04/bench/traverse.cpp -o traverse
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>

struct Node {
    int value;
    Node* next;
};

template <typename Work>
double seconds(Work work) {
    auto start = std::chrono::steady_clock::now();
    work();
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

// Links the nodes of `pool` into one list, visiting them in `order`.
Node* link(std::vector<Node>& pool, const std::vector<std::size_t>& order) {
    for (std::size_t i = 0; i + 1 < order.size(); ++i) {
        pool[order[i]].next = &pool[order[i + 1]];
    }
    pool[order.back()].next = nullptr;
    return &pool[order.front()];
}

long long sum_list(const Node* head) {
    long long total = 0;
    for (const Node* p = head; p != nullptr; p = p->next) total += p->value;
    return total;
}

int main() {
    std::mt19937 rng(270);
    std::printf("%10s %14s %16s %16s\n", "n", "vector ns/elt", "list, in order", "list, scattered");
    for (std::size_t n = 1000; n <= 10000000; n *= 10) {
        std::vector<int> vec(n, 1);
        std::vector<Node> pool(n, Node{1, nullptr});
        std::vector<std::size_t> order(n);
        std::iota(order.begin(), order.end(), std::size_t{0});

        const int reps = static_cast<int>(20000000 / n) + 1;
        long long sink = 0;
        double tv = seconds([&] {
            for (int r = 0; r < reps; ++r)
                sink += std::accumulate(vec.begin(), vec.end(), 0LL);
        });
        Node* head = link(pool, order);
        double tl = seconds([&] {
            for (int r = 0; r < reps; ++r) sink += sum_list(head);
        });
        std::shuffle(order.begin(), order.end(), rng);
        head = link(pool, order);
        double ts = seconds([&] {
            for (int r = 0; r < reps; ++r) sink += sum_list(head);
        });
        double per = 1e9 / (static_cast<double>(n) * reps);
        std::printf("%10zu %14.2f %16.2f %16.2f   (%lld)\n", n, tv * per, tl * per, ts * per,
                    sink % 1000);
    }
}
