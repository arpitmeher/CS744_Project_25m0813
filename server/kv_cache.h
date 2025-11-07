#pragma once
#include <unordered_map>
#include <list>
#include <string>
#include <mutex>

class KVCache {
    size_t capacity;
    std::list<std::pair<std::string, std::string>> lru;
    std::unordered_map<std::string, decltype(lru.begin())> map;
    std::mutex mtx;

public:
    int hits = 0;
    int misses = 0;

    KVCache(size_t cap);
    void put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    void remove(const std::string& key);
};

