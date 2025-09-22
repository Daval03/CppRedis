#pragma once
#include "redis_value.h"
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <regex>

class RedisDatabase {
private:
    std::unordered_map<std::string, RedisValue> database;
    mutable std::mutex db_mutex;

public:
    bool keyExists(const std::string& key);
    RedisValue* getValue(const std::string& key);
    void setValue(const std::string& key, RedisValue value);
    bool deleteKey(const std::string& key);
    void clearDatabase();
    size_t getDatabaseSize() const;
    void cleanupExpiredKeys();
    
    // Iterator support for KEYS command
    std::vector<std::string> getMatchingKeys(const std::string& pattern) const;
    
};