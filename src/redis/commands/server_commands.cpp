#include "server_commands.h"

ServerCommands::ServerCommands(RedisDatabase& database, 
                              std::chrono::system_clock::time_point server_start_time,
                              size_t& commands_processed)
    : db(database), start_time(server_start_time), total_commands_processed(commands_processed) {}

std::string ServerCommands::cmdPing(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'ping' command");
    }
    
    if (args.size() == 2) {
        return RESPFormatter::formatBulkString(args[1]);
    }
    
    return RESPFormatter::formatSimpleString("PONG");
}

std::string ServerCommands::cmdEcho(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'echo' command");
    }
    
    return RESPFormatter::formatBulkString(args[1]);
}

std::string ServerCommands::cmdInfo(const std::vector<std::string>& /*args*/) {
    auto now = std::chrono::system_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
    
    std::ostringstream info;
    info << "# Server\r\n";
    info << "redis_version:7.0.0\r\n";
    info << "uptime_in_seconds:" << uptime.count() << "\r\n";
    info << "\r\n";
    info << "# Stats\r\n";
    info << "total_commands_processed:" << total_commands_processed << "\r\n";
    info << "\r\n";
    info << "# Keyspace\r\n";
    info << "db0:keys=" << db.getDatabaseSize() << "\r\n";
    
    return RESPFormatter::formatBulkString(info.str());
}

std::string ServerCommands::cmdFlushall(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'flushall' command");
    }
    
    db.clearDatabase();
    return RESPFormatter::formatSimpleString("OK");
}

std::string ServerCommands::cmdKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'keys' command");
    }
    
    const std::string& pattern = args[1];
    std::vector<std::string> matching_keys = db.getMatchingKeys(pattern);
    
    return RESPFormatter::formatArray(matching_keys);
}

std::string ServerCommands::cmdDbsize(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'dbsize' command");
    }
    
    db.cleanupExpiredKeys();
    return RESPFormatter::formatInteger(db.getDatabaseSize());
}

std::string ServerCommands::cmdTime(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'time' command");
    }
    
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
    
    std::vector<std::string> time_result = {
        std::to_string(time_t_now),
        std::to_string(microseconds)
    };
    
    return RESPFormatter::formatArray(time_result);
}