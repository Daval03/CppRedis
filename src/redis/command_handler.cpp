#include "command_handler.h"

CommandHandler::CommandHandler() 
    : start_time(std::chrono::system_clock::now()),
      string_commands(std::make_unique<StringCommands>(db)),
      list_commands(std::make_unique<ListCommands>(db)),
      set_commands(std::make_unique<SetCommands>(db)) {
    initializeCommands();
}

void CommandHandler::initializeCommands() {
    // String commands
    commands["SET"] = [this](const std::vector<std::string>& args) { return string_commands->cmdSet(args); };
    commands["GET"] = [this](const std::vector<std::string>& args) { return string_commands->cmdGet(args); };
    commands["DEL"] = [this](const std::vector<std::string>& args) { return string_commands->cmdDel(args); };
    commands["EXISTS"] = [this](const std::vector<std::string>& args) { return string_commands->cmdExists(args); };
    commands["TYPE"] = [this](const std::vector<std::string>& args) { return string_commands->cmdType(args); };
    commands["INCR"] = [this](const std::vector<std::string>& args) { return string_commands->cmdIncr(args); };
    commands["DECR"] = [this](const std::vector<std::string>& args) { return string_commands->cmdDecr(args); };
    commands["INCRBY"] = [this](const std::vector<std::string>& args) { return string_commands->cmdIncrBy(args); };
    commands["DECRBY"] = [this](const std::vector<std::string>& args) { return string_commands->cmdDecrBy(args); };
    commands["STRLEN"] = [this](const std::vector<std::string>& args) { return string_commands->cmdStrlen(args); };
    commands["APPEND"] = [this](const std::vector<std::string>& args) { return string_commands->cmdAppend(args); };
    commands["MGET"] = [this](const std::vector<std::string>& args) { return string_commands->cmdMget(args); };
    commands["MSET"] = [this](const std::vector<std::string>& args) { return string_commands->cmdMset(args); };
    
    // List commands
    commands["LPUSH"] = [this](const std::vector<std::string>& args) { return list_commands->cmdLpush(args); };
    commands["RPUSH"] = [this](const std::vector<std::string>& args) { return list_commands->cmdRpush(args); };
    commands["LPOP"] = [this](const std::vector<std::string>& args) { return list_commands->cmdLpop(args); };
    commands["RPOP"] = [this](const std::vector<std::string>& args) { return list_commands->cmdRpop(args); };
    commands["LLEN"] = [this](const std::vector<std::string>& args) { return list_commands->cmdLlen(args); };
    commands["LRANGE"] = [this](const std::vector<std::string>& args) { return list_commands->cmdLrange(args); };
    commands["LINDEX"] = [this](const std::vector<std::string>& args) { return list_commands->cmdLindex(args); };
    commands["LSET"] = [this](const std::vector<std::string>& args) { return list_commands->cmdLset(args); };
    
    // Set commands
    commands["SADD"] = [this](const std::vector<std::string>& args) { return set_commands->cmdSadd(args); };
    commands["SREM"] = [this](const std::vector<std::string>& args) { return set_commands->cmdSrem(args); };
    commands["SISMEMBER"] = [this](const std::vector<std::string>& args) { return set_commands->cmdSismember(args); };
    commands["SCARD"] = [this](const std::vector<std::string>& args) { return set_commands->cmdScard(args); };
    commands["SMEMBERS"] = [this](const std::vector<std::string>& args) { return set_commands->cmdSmembers(args); };
    commands["SPOP"] = [this](const std::vector<std::string>& args) { return set_commands->cmdSpop(args); };
    
    // Hash commands
    commands["HSET"] = [this](const std::vector<std::string>& args) { return cmdHset(args); };
    commands["HGET"] = [this](const std::vector<std::string>& args) { return cmdHget(args); };
    commands["HDEL"] = [this](const std::vector<std::string>& args) { return cmdHdel(args); };
    commands["HEXISTS"] = [this](const std::vector<std::string>& args) { return cmdHexists(args); };
    commands["HLEN"] = [this](const std::vector<std::string>& args) { return cmdHlen(args); };
    commands["HKEYS"] = [this](const std::vector<std::string>& args) { return cmdHkeys(args); };
    commands["HVALS"] = [this](const std::vector<std::string>& args) { return cmdHvals(args); };
    commands["HGETALL"] = [this](const std::vector<std::string>& args) { return cmdHgetall(args); };
    
    // TTL commands
    commands["EXPIRE"] = [this](const std::vector<std::string>& args) { return cmdExpire(args); };
    commands["EXPIREAT"] = [this](const std::vector<std::string>& args) { return cmdExpireat(args); };
    commands["TTL"] = [this](const std::vector<std::string>& args) { return cmdTtl(args); };
    commands["PERSIST"] = [this](const std::vector<std::string>& args) { return cmdPersist(args); };
    
    // Server commands
    commands["PING"] = [this](const std::vector<std::string>& args) { return cmdPing(args); };
    commands["ECHO"] = [this](const std::vector<std::string>& args) { return cmdEcho(args); };
    commands["INFO"] = [this](const std::vector<std::string>& args) { return cmdInfo(args); };
    commands["FLUSHALL"] = [this](const std::vector<std::string>& args) { return cmdFlushall(args); };
    commands["KEYS"] = [this](const std::vector<std::string>& args) { return cmdKeys(args); };
    commands["DBSIZE"] = [this](const std::vector<std::string>& args) { return cmdDbsize(args); };
    commands["TIME"] = [this](const std::vector<std::string>& args) { return cmdTime(args); };
}

std::string CommandHandler::processCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return RESPFormatter::formatError("ERR empty command");
    }
    
    total_commands_processed++;
    
    // Clean up expired keys periodically
    if (total_commands_processed % 100 == 0) {
        db.cleanupExpiredKeys();
    }
    
    std::string command = UtilityFunctions::toUpper(args[0]);
    auto it = commands.find(command);
    
    if (it == commands.end()) {
        return RESPFormatter::formatError("ERR unknown command '" + args[0] + "'");
    }
    
    try {
        return it->second(args);
    } catch (const std::exception& e) {
        return RESPFormatter::formatError("ERR " + std::string(e.what()));
    }
}

// Hash Commands Implementation
std::string CommandHandler::cmdHset(const std::vector<std::string>& args) {
    if (args.size() < 4 || args.size() % 2 != 0) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hset' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (value && value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        db.setValue(key, RedisValue(RedisValue::Type::HASH));
        value = db.getValue(key);
    }
    
    int added = 0;
    for (size_t i = 2; i < args.size(); i += 2) {
        const std::string& field = args[i];
        const std::string& field_value = args[i + 1];
        
        if (value->hash_value.find(field) == value->hash_value.end()) {
            added++;
        }
        value->hash_value[field] = field_value;
    }
    
    return RESPFormatter::formatInteger(added);
}

std::string CommandHandler::cmdHget(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hget' command");
    }
    
    const std::string& key = args[1];
    const std::string& field = args[2];
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatNull();
    }
    
    auto it = value->hash_value.find(field);
    if (it == value->hash_value.end()) {
        return RESPFormatter::formatNull();
    }
    
    return RESPFormatter::formatBulkString(it->second);
}

std::string CommandHandler::cmdHdel(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hdel' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatInteger(0);
    }
    
    int deleted = 0;
    for (size_t i = 2; i < args.size(); i++) {
        if (value->hash_value.erase(args[i]) > 0) {
            deleted++;
        }
    }
    
    if (value->hash_value.empty()) {
        db.deleteKey(key);
    }
    
    return RESPFormatter::formatInteger(deleted);
}

std::string CommandHandler::cmdHexists(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hexists' command");
    }
    
    const std::string& key = args[1];
    const std::string& field = args[2];
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->hash_value.count(field) > 0 ? 1 : 0);
}

std::string CommandHandler::cmdHlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hlen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->hash_value.size());
}

std::string CommandHandler::cmdHkeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hkeys' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> keys;
    for (const auto& pair : value->hash_value) {
        keys.push_back(pair.first);
    }
    
    return RESPFormatter::formatArray(keys);
}

std::string CommandHandler::cmdHvals(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hvals' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> values;
    for (const auto& pair : value->hash_value) {
        values.push_back(pair.second);
    }
    
    return RESPFormatter::formatArray(values);
}

std::string CommandHandler::cmdHgetall(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hgetall' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> result;
    for (const auto& pair : value->hash_value) {
        result.push_back(pair.first);
        result.push_back(pair.second);
    }
    
    return RESPFormatter::formatArray(result);
}

// TTL Commands Implementation
std::string CommandHandler::cmdExpire(const std::vector<std::string>& args) {
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

std::string CommandHandler::cmdExpireat(const std::vector<std::string>& args) {
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

std::string CommandHandler::cmdTtl(const std::vector<std::string>& args) {
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

std::string CommandHandler::cmdPersist(const std::vector<std::string>& args) {
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

// Server Commands Implementation
std::string CommandHandler::cmdPing(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'ping' command");
    }
    
    if (args.size() == 2) {
        return RESPFormatter::formatBulkString(args[1]);
    }
    
    return RESPFormatter::formatSimpleString("PONG");
}

std::string CommandHandler::cmdEcho(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'echo' command");
    }
    
    return RESPFormatter::formatBulkString(args[1]);
}

std::string CommandHandler::cmdInfo(const std::vector<std::string>& /*args*/) {
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

std::string CommandHandler::cmdFlushall(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'flushall' command");
    }
    
    db.clearDatabase();
    return RESPFormatter::formatSimpleString("OK");
}

std::string CommandHandler::cmdKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'keys' command");
    }
    
    const std::string& pattern = args[1];
    std::vector<std::string> matching_keys = db.getMatchingKeys(pattern); // Usar db
    
    return RESPFormatter::formatArray(matching_keys);
}

std::string CommandHandler::cmdDbsize(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'dbsize' command");
    }
    
    db.cleanupExpiredKeys(); // Cambiar a db
    return RESPFormatter::formatInteger(db.getDatabaseSize()); // Cambiar a db
}

std::string CommandHandler::cmdTime(const std::vector<std::string>& args) {
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