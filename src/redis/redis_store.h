#ifndef REDIS_STORE_H
#define REDIS_STORE_H

#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>
#include <algorithm>
#include <iostream>

class RedisStore {
private:
    std::unordered_map<std::string, std::string> data;
    mutable std::mutex mutex;

public:
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool exists(const std::string& key);
    int del(const std::vector<std::string>& keys);
    std::vector<std::string> keys(const std::string& pattern = "*");
    size_t size() const;
    void clear();
};

#endif