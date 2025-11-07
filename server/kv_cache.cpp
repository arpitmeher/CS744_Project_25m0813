#include "kv_cache.h"

KVCache::KVCache(size_t cap) : capacity(cap) {}

void KVCache::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    if (map.count(key)) {
        lru.erase(map[key]);
    } else if (lru.size() >= capacity) {
        auto last = lru.back();
        map.erase(last.first);
        lru.pop_back();
    }
    lru.push_front({key, value});
    map[key] = lru.begin();
}

bool KVCache::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!map.count(key)) {
        ++misses;
        return false;
    }
    lru.splice(lru.begin(), lru, map[key]);
    value = map[key]->second;
    ++hits;
    return true;
}

void KVCache::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    if (map.count(key)) {
        lru.erase(map[key]);
        map.erase(key);
    }
}

