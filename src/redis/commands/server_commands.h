#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "redis/database/redis_database.h"
#include "resp/resp_formatter.h"
#include "redis/database/redis_value.h"
#include "utils/utility_functions.h"
#include "enum/redis_type.h"
class ServerCommands {
private:
    RedisDatabase& db;
    std::chrono::system_clock::time_point start_time;
    size_t& total_commands_processed;

public:
    ServerCommands(RedisDatabase& database, std::chrono::system_clock::time_point server_start_time, size_t& commands_processed);
    ~ServerCommands() = default;

    // Server command implementations
    std::string cmdPing(const std::vector<std::string>& args);
    std::string cmdEcho(const std::vector<std::string>& args);
    std::string cmdInfo(const std::vector<std::string>& args);
    std::string cmdFlushall(const std::vector<std::string>& args);
    std::string cmdKeys(const std::vector<std::string>& args);
    std::string cmdDbsize(const std::vector<std::string>& args);
    std::string cmdTime(const std::vector<std::string>& args);
};