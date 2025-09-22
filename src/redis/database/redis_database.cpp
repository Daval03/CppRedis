#include "redis_database.h"

bool RedisDatabase::keyExists(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = database.find(key);
    if (it != database.end() && !it->second.isExpired()) {
        return true;
    }
    if (it != database.end() && it->second.isExpired()) {
        database.erase(it);
    }
    return false;
}

RedisValue* RedisDatabase::getValue(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = database.find(key);
    if (it != database.end()) {
        if (it->second.isExpired()) {
            database.erase(it);
            return nullptr;
        }
        return &it->second;
    }
    return nullptr;
}

void RedisDatabase::setValue(const std::string& key, RedisValue value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    database[key] = std::move(value);
}

bool RedisDatabase::deleteKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    return database.erase(key) > 0;
}

void RedisDatabase::clearDatabase() {
    std::lock_guard<std::mutex> lock(db_mutex);
    database.clear();
}

size_t RedisDatabase::getDatabaseSize() const {
    std::lock_guard<std::mutex> lock(db_mutex);
    return database.size();
}

void RedisDatabase::cleanupExpiredKeys() {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = database.begin();
    while (it != database.end()) {
        if (it->second.isExpired()) {
            it = database.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<std::string> RedisDatabase::getMatchingKeys(const std::string& pattern) const {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> matching_keys;
    
    // Convert Redis pattern to regex
    std::string regex_pattern;
    for (char c : pattern) {
        if (c == '*') {
            regex_pattern += ".*";
        } else if (c == '?') {
            regex_pattern += ".";
        } else if (c == '.' || c == '^' || c == '$' || c == '+' || c == '|' || 
                  c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
            regex_pattern += '\\';
            regex_pattern += c;
        } else {
            regex_pattern += c;
        }
    }
    
    std::regex pattern_regex(regex_pattern);
    
    for (const auto& pair : database) {
        if (!pair.second.isExpired() && std::regex_match(pair.first, pattern_regex)) {
            matching_keys.push_back(pair.first);
        }
    }
    
    return matching_keys;
}