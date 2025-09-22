#include "ttl_commands.h"

TTLCommands::TTLCommands(RedisDatabase& database) : db(database) {}

std::string TTLCommands::cmdExpire(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'expire' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value) {
        return RESPFormatter::formatInteger(0);
    }
    
    long long seconds = UtilityFunctions::parseInt(args[2]);
    if (seconds <= 0) {
        db.deleteKey(key);
        return RESPFormatter::formatInteger(1);
    }
    
    value->setExpiry(std::chrono::seconds(seconds));
    return RESPFormatter::formatInteger(1);
}

std::string TTLCommands::cmdExpireat(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'expireat' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value) {
        return RESPFormatter::formatInteger(0);
    }
    
    long long timestamp = UtilityFunctions::parseInt(args[2]);
    auto expiry_time = std::chrono::system_clock::from_time_t(timestamp);
    
    if (expiry_time <= std::chrono::system_clock::now()) {
        db.deleteKey(key);
        return RESPFormatter::formatInteger(1);
    }
    
    value->expiry = expiry_time;
    value->has_expiry = true;
    return RESPFormatter::formatInteger(1);
}

std::string TTLCommands::cmdTtl(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'ttl' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value) {
        return RESPFormatter::formatInteger(-2); // Key doesn't exist
    }
    
    if (!value->has_expiry) {
        return RESPFormatter::formatInteger(-1); // Key exists but has no expiry
    }
    
    auto now = std::chrono::system_clock::now();
    if (now >= value->expiry) {
        db.deleteKey(key);
        return RESPFormatter::formatInteger(-2); // Key expired
    }
    
    auto ttl_seconds = std::chrono::duration_cast<std::chrono::seconds>(value->expiry - now);
    return RESPFormatter::formatInteger(ttl_seconds.count());
}

std::string TTLCommands::cmdPersist(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'persist' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value) {
        return RESPFormatter::formatInteger(0);
    }
    
    if (!value->has_expiry) {
        return RESPFormatter::formatInteger(0);
    }
    
    value->clearExpiry();
    return RESPFormatter::formatInteger(1);
}