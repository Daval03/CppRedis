#include "redis_store.h"


void RedisStore::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex);
    data[key] = value;
    std::cout << "SET: " << key << " = " << value << " (Store size: " << data.size() << ")" << std::endl;
}

std::string RedisStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return ""; // Return empty string for not found (NULL will be handled by formatter)
}

bool RedisStore::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex);
    return data.find(key) != data.end();
}

int RedisStore::del(const std::vector<std::string>& keys) {
    std::lock_guard<std::mutex> lock(mutex);
    int deleted_count = 0;
    for (const auto& key : keys) {
        if (data.erase(key) > 0) {
            deleted_count++;
        }
    }
    std::cout << "DEL: Deleted " << deleted_count << " keys (Store size: " << data.size() << ")" << std::endl;
    return deleted_count;
}

std::vector<std::string> RedisStore::keys(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<std::string> matching_keys;
    
    for (const auto& pair : data) {
        // Simple pattern matching (only supports * for now)
        if (pattern == "*" || pair.first.find(pattern) != std::string::npos) {
            matching_keys.push_back(pair.first);
        }
    }
    
    return matching_keys;
}

size_t RedisStore::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return data.size();
}

void RedisStore::clear() {
    std::lock_guard<std::mutex> lock(mutex);
    data.clear();
    std::cout << "Redis store cleared" << std::endl;
}