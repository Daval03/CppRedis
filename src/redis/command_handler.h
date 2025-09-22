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

#include "resp/resp_value.h"
#include "resp/resp_parser.h"
#include "resp/resp_formatter.h"
#include "utils/utility_functions.h"
#include "redis/database/redis_database.h"
#include "redis/commands/string_commands.h"
#include "redis/commands/list_commands.h"
#include "redis/commands/set_commands.h"
#include "redis/commands/hash_commands.h"
#include "redis/commands/ttl_commands.h"
#include "redis/commands/server_commands.h"

// Command handler class
class CommandHandler {
private:

    //Database
    RedisDatabase db;
    
    // Server info
    std::chrono::system_clock::time_point start_time;
    size_t total_commands_processed = 0;
    
    // Command function type
    using CommandFunc = std::function<std::string(const std::vector<std::string>&)>;
    std::unordered_map<std::string, CommandFunc> commands;
    
    // Helper methods
    void initializeCommands();
    
    // Command handlers
    std::unique_ptr<StringCommands> string_commands;
    std::unique_ptr<ListCommands> list_commands;
    std::unique_ptr<SetCommands> set_commands;
    std::unique_ptr<HashCommands> hash_commands;
    std::unique_ptr<TTLCommands> ttl_commands;
    std::unique_ptr<ServerCommands> server_commands;
 
public:
    CommandHandler();
    ~CommandHandler() = default;
    
    // Main processing method
    std::string processCommand(const std::vector<std::string>& args);
    
    // Statistics
    size_t getTotalCommandsProcessed() const { return total_commands_processed; }
    std::chrono::system_clock::time_point getStartTime() const { return start_time; }
};
