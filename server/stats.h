#pragma once
#include <atomic>

struct Stats {
    std::atomic<int> total_puts{0};
    std::atomic<int> valid_puts{0};
    std::atomic<int> invalid_puts{0};
};

