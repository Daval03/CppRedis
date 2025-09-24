#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "redis/database/redis_database.h"
#include "resp/resp_formatter.h"
#include "redis/database/redis_value.h"
#include "utils/utility_functions.h"
#include "enum/redis_type.h"
class ListCommands {
private:
    RedisDatabase& db;

public:
    explicit ListCommands(RedisDatabase& database);
    ~ListCommands() = default;

    // List command implementations
    std::string cmdLpush(const std::vector<std::string>& args);
    std::string cmdRpush(const std::vector<std::string>& args);
    std::string cmdLpop(const std::vector<std::string>& args);
    std::string cmdRpop(const std::vector<std::string>& args);
    std::string cmdLlen(const std::vector<std::string>& args);
    std::string cmdLrange(const std::vector<std::string>& args);
    std::string cmdLindex(const std::vector<std::string>& args);
    std::string cmdLset(const std::vector<std::string>& args);
};