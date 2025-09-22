#pragma once

#include <string>

#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <sstream>
#include <random>
#include <ctime>
#include <iomanip>
#include "../../src/resp/resp_parser.h"

// Forward declarations
class RESPValue;
enum class RESPType;

// Data structures for different Redis data types
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

// Command handler class
class CommandHandler {
private:
    std::unordered_map<std::string, RedisValue> database;
    mutable std::mutex db_mutex;
    
    // Server info
    std::chrono::system_clock::time_point start_time;
    size_t total_commands_processed = 0;
    
    // Command function type
    using CommandFunc = std::function<std::string(const std::vector<std::string>&)>;
    std::unordered_map<std::string, CommandFunc> commands;
    
    // Helper methods
    void initializeCommands();
    std::string formatResponse(const RESPValue& value);
    std::string formatError(const std::string& message);
    std::string formatSimpleString(const std::string& str);
    std::string formatBulkString(const std::string& str);
    std::string formatInteger(long long value);
    std::string formatArray(const std::vector<std::string>& items);
    std::string formatNull();
    
    bool isValidKey(const std::string& key);
    void cleanupExpiredKeys();
    bool keyExists(const std::string& key);
    RedisValue* getValue(const std::string& key);
    void setValue(const std::string& key, RedisValue value);
    bool deleteKey(const std::string& key);
    
    // String commands
    std::string cmdSet(const std::vector<std::string>& args);
    std::string cmdGet(const std::vector<std::string>& args);
    std::string cmdDel(const std::vector<std::string>& args);
    std::string cmdExists(const std::vector<std::string>& args);
    std::string cmdType(const std::vector<std::string>& args);
    std::string cmdIncr(const std::vector<std::string>& args);
    std::string cmdDecr(const std::vector<std::string>& args);
    std::string cmdIncrBy(const std::vector<std::string>& args);
    std::string cmdDecrBy(const std::vector<std::string>& args);
    std::string cmdStrlen(const std::vector<std::string>& args);
    std::string cmdAppend(const std::vector<std::string>& args);
    std::string cmdMget(const std::vector<std::string>& args);
    std::string cmdMset(const std::vector<std::string>& args);
    
    // List commands
    std::string cmdLpush(const std::vector<std::string>& args);
    std::string cmdRpush(const std::vector<std::string>& args);
    std::string cmdLpop(const std::vector<std::string>& args);
    std::string cmdRpop(const std::vector<std::string>& args);
    std::string cmdLlen(const std::vector<std::string>& args);
    std::string cmdLrange(const std::vector<std::string>& args);
    std::string cmdLindex(const std::vector<std::string>& args);
    std::string cmdLset(const std::vector<std::string>& args);
    
    // Set commands
    std::string cmdSadd(const std::vector<std::string>& args);
    std::string cmdSrem(const std::vector<std::string>& args);
    std::string cmdSismember(const std::vector<std::string>& args);
    std::string cmdScard(const std::vector<std::string>& args);
    std::string cmdSmembers(const std::vector<std::string>& args);
    std::string cmdSpop(const std::vector<std::string>& args);
    
    // Hash commands
    std::string cmdHset(const std::vector<std::string>& args);
    std::string cmdHget(const std::vector<std::string>& args);
    std::string cmdHdel(const std::vector<std::string>& args);
    std::string cmdHexists(const std::vector<std::string>& args);
    std::string cmdHlen(const std::vector<std::string>& args);
    std::string cmdHkeys(const std::vector<std::string>& args);
    std::string cmdHvals(const std::vector<std::string>& args);
    std::string cmdHgetall(const std::vector<std::string>& args);
    
    // TTL commands
    std::string cmdExpire(const std::vector<std::string>& args);
    std::string cmdExpireat(const std::vector<std::string>& args);
    std::string cmdTtl(const std::vector<std::string>& args);
    std::string cmdPersist(const std::vector<std::string>& args);
    
    // Server commands
    std::string cmdPing(const std::vector<std::string>& args);
    std::string cmdEcho(const std::vector<std::string>& args);
    std::string cmdInfo(const std::vector<std::string>& args);
    std::string cmdFlushall(const std::vector<std::string>& args);
    std::string cmdKeys(const std::vector<std::string>& args);
    std::string cmdDbsize(const std::vector<std::string>& args);
    std::string cmdTime(const std::vector<std::string>& args);
    
    // Utility functions
    bool isInteger(const std::string& str);
    long long parseInt(const std::string& str);
    std::string toUpper(const std::string& str);
    bool matchPattern(const std::string& pattern, const std::string& str);
    
public:
    CommandHandler();
    ~CommandHandler() = default;
    
    // Main processing method
    std::string processCommand(const RESPValue& command);
    std::string processCommand(const std::vector<std::string>& args);
    
    // Database management
    void clearDatabase();
    size_t getDatabaseSize() const;
    
    // Statistics
    size_t getTotalCommandsProcessed() const { return total_commands_processed; }
    std::chrono::system_clock::time_point getStartTime() const { return start_time; }
};