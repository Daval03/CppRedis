#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "redis/database/redis_database.h"
#include "resp/resp_formatter.h"
#include "redis/database/redis_value.h"
#include "utils/utility_functions.h"

class StringCommands {
private:
    RedisDatabase& db;

public:
    explicit StringCommands(RedisDatabase& database);
    ~StringCommands() = default;

    // String command implementations
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
};