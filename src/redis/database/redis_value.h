#pragma once
#include <string>
#include <list>
#include <set>
#include <unordered_map>
#include <map>
#include <chrono>

struct RedisValue {
    enum class Type {
        STRING,
        LIST,
        SET,
        HASH,
        ZSET,
        STREAM
    };
    
    Type type;
    std::string string_value;
    std::list<std::string> list_value;
    std::set<std::string> set_value;
    std::unordered_map<std::string, std::string> hash_value;
    std::map<double, std::set<std::string>> zset_value;
    
    // TTL support
    std::chrono::system_clock::time_point expiry;
    bool has_expiry = false;
    
    RedisValue() : type(Type::STRING) {}
    explicit RedisValue(Type t) : type(t) {}
    explicit RedisValue(const std::string& str) : type(Type::STRING), string_value(str) {}
    
    bool isExpired() const {
        return has_expiry && std::chrono::system_clock::now() >= expiry;
    }
    
    void setExpiry(std::chrono::milliseconds ttl) {
        expiry = std::chrono::system_clock::now() + ttl;
        has_expiry = true;
    }
    
    void clearExpiry() {
        has_expiry = false;
    }
};