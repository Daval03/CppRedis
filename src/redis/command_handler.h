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
    
public:
    CommandHandler();
    ~CommandHandler() = default;
    
    // Main processing method
    std::string processCommand(const std::vector<std::string>& args);
    
    // Statistics
    size_t getTotalCommandsProcessed() const { return total_commands_processed; }
    std::chrono::system_clock::time_point getStartTime() const { return start_time; }
};
