#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "redis/database/redis_database.h"
#include "resp/resp_formatter.h"
#include "redis/database/redis_value.h"
#include "utils/utility_functions.h"

class HashCommands {
private:
    RedisDatabase& db;

public:
    explicit HashCommands(RedisDatabase& database);
    ~HashCommands() = default;

    // Hash command implementations
    std::string cmdHset(const std::vector<std::string>& args);
    std::string cmdHget(const std::vector<std::string>& args);
    std::string cmdHdel(const std::vector<std::string>& args);
    std::string cmdHexists(const std::vector<std::string>& args);
    std::string cmdHlen(const std::vector<std::string>& args);
    std::string cmdHkeys(const std::vector<std::string>& args);
    std::string cmdHvals(const std::vector<std::string>& args);
    std::string cmdHgetall(const std::vector<std::string>& args);
};