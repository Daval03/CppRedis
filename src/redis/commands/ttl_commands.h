#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "redis/database/redis_database.h"
#include "resp/resp_formatter.h"
#include "redis/database/redis_value.h"
#include "utils/utility_functions.h"

class TTLCommands {
private:
    RedisDatabase& db;

public:
    explicit TTLCommands(RedisDatabase& database);
    ~TTLCommands() = default;

    // TTL command implementations
    std::string cmdExpire(const std::vector<std::string>& args);
    std::string cmdExpireat(const std::vector<std::string>& args);
    std::string cmdTtl(const std::vector<std::string>& args);
    std::string cmdPersist(const std::vector<std::string>& args);
};