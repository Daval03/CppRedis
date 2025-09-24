#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <random>
#include "redis/database/redis_database.h"
#include "resp/resp_formatter.h"
#include "redis/database/redis_value.h"
#include "utils/utility_functions.h"
#include "enum/redis_type.h"
class SetCommands {
private:
    RedisDatabase& db;

public:
    explicit SetCommands(RedisDatabase& database);
    ~SetCommands() = default;

    // Set command implementations
    std::string cmdSadd(const std::vector<std::string>& args);
    std::string cmdSrem(const std::vector<std::string>& args);
    std::string cmdSismember(const std::vector<std::string>& args);
    std::string cmdScard(const std::vector<std::string>& args);
    std::string cmdSmembers(const std::vector<std::string>& args);
    std::string cmdSpop(const std::vector<std::string>& args);
};