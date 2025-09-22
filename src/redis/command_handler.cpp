#include "command_handler.h"

CommandHandler::CommandHandler() : start_time(std::chrono::system_clock::now()) {
    initializeCommands();
}

void CommandHandler::initializeCommands() {
    // String commands
    commands["SET"] = [this](const std::vector<std::string>& args) { return cmdSet(args); };
    commands["GET"] = [this](const std::vector<std::string>& args) { return cmdGet(args); };
    commands["DEL"] = [this](const std::vector<std::string>& args) { return cmdDel(args); };
    commands["EXISTS"] = [this](const std::vector<std::string>& args) { return cmdExists(args); };
    commands["TYPE"] = [this](const std::vector<std::string>& args) { return cmdType(args); };
    commands["INCR"] = [this](const std::vector<std::string>& args) { return cmdIncr(args); };
    commands["DECR"] = [this](const std::vector<std::string>& args) { return cmdDecr(args); };
    commands["INCRBY"] = [this](const std::vector<std::string>& args) { return cmdIncrBy(args); };
    commands["DECRBY"] = [this](const std::vector<std::string>& args) { return cmdDecrBy(args); };
    commands["STRLEN"] = [this](const std::vector<std::string>& args) { return cmdStrlen(args); };
    commands["APPEND"] = [this](const std::vector<std::string>& args) { return cmdAppend(args); };
    commands["MGET"] = [this](const std::vector<std::string>& args) { return cmdMget(args); };
    commands["MSET"] = [this](const std::vector<std::string>& args) { return cmdMset(args); };
    
    // List commands
    commands["LPUSH"] = [this](const std::vector<std::string>& args) { return cmdLpush(args); };
    commands["RPUSH"] = [this](const std::vector<std::string>& args) { return cmdRpush(args); };
    commands["LPOP"] = [this](const std::vector<std::string>& args) { return cmdLpop(args); };
    commands["RPOP"] = [this](const std::vector<std::string>& args) { return cmdRpop(args); };
    commands["LLEN"] = [this](const std::vector<std::string>& args) { return cmdLlen(args); };
    commands["LRANGE"] = [this](const std::vector<std::string>& args) { return cmdLrange(args); };
    commands["LINDEX"] = [this](const std::vector<std::string>& args) { return cmdLindex(args); };
    commands["LSET"] = [this](const std::vector<std::string>& args) { return cmdLset(args); };
    
    // Set commands
    commands["SADD"] = [this](const std::vector<std::string>& args) { return cmdSadd(args); };
    commands["SREM"] = [this](const std::vector<std::string>& args) { return cmdSrem(args); };
    commands["SISMEMBER"] = [this](const std::vector<std::string>& args) { return cmdSismember(args); };
    commands["SCARD"] = [this](const std::vector<std::string>& args) { return cmdScard(args); };
    commands["SMEMBERS"] = [this](const std::vector<std::string>& args) { return cmdSmembers(args); };
    commands["SPOP"] = [this](const std::vector<std::string>& args) { return cmdSpop(args); };
    
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
    
    std::string command = toUpper(args[0]);
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

// String Commands Implementation
std::string CommandHandler::cmdSet(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'set' command");
    }
    
    const std::string& key = args[1];
    const std::string& value = args[2];
    
    RedisValue redis_value(value);
    
    // Handle optional parameters (EX, PX, NX, XX)
    for (size_t i = 3; i < args.size(); i += 2) {
        if (i + 1 >= args.size()) {
            return RESPFormatter::formatError("ERR syntax error");
        }
        
        std::string param = toUpper(args[i]);
        if (param == "EX") {
            if (!isInteger(args[i + 1])) {
                return RESPFormatter::formatError("ERR value is not an integer or out of range");
            }
            redis_value.setExpiry(std::chrono::seconds(parseInt(args[i + 1])));
        } else if (param == "PX") {
            if (!isInteger(args[i + 1])) {
                return RESPFormatter::formatError("ERR value is not an integer or out of range");
            }
            redis_value.setExpiry(std::chrono::milliseconds(parseInt(args[i + 1])));
        } else if (param == "NX") {
            if (db.keyExists(key)) {
                return RESPFormatter::formatNull();
            }
            i--; // NX doesn't have a value
        } else if (param == "XX") {
            if (!db.keyExists(key)) {
                return RESPFormatter::formatNull();
            }
            i--; // XX doesn't have a value
        }
    }
    
    db.setValue(key, redis_value);
    return RESPFormatter::formatSimpleString("OK");
}

std::string CommandHandler::cmdGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'get' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::STRING) {
        return RESPFormatter::formatNull();
    }
    
    return RESPFormatter::formatBulkString(value->string_value);
}

std::string CommandHandler::cmdDel(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'del' command");
    }
    
    int deleted = 0;
    for (size_t i = 1; i < args.size(); i++) {
        if (db.deleteKey(args[i])) {
            deleted++;
        }
    }
    
    return RESPFormatter::formatInteger(deleted);
}

std::string CommandHandler::cmdExists(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'exists' command");
    }
    
    int count = 0;
    for (size_t i = 1; i < args.size(); i++) {
        if (db.keyExists(args[i])) {
            count++;
        }
    }
    
    return RESPFormatter::formatInteger(count);
}

std::string CommandHandler::cmdType(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'type' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value) {
        return RESPFormatter::formatSimpleString("none");
    }
    
    switch (value->type) {
        case RedisValue::Type::STRING: return RESPFormatter::formatSimpleString("string");
        case RedisValue::Type::LIST: return RESPFormatter::formatSimpleString("list");
        case RedisValue::Type::SET: return RESPFormatter::formatSimpleString("set");
        case RedisValue::Type::HASH: return RESPFormatter::formatSimpleString("hash");
        case RedisValue::Type::ZSET: return RESPFormatter::formatSimpleString("zset");
        default: return RESPFormatter::formatSimpleString("unknown");
    }
}

std::string CommandHandler::cmdIncr(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'incr' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current++;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string CommandHandler::cmdDecr(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'decr' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current--;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string CommandHandler::cmdIncrBy(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'incrby' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    long long increment = parseInt(args[2]);
    
    RedisValue* value = db.getValue(key);
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current += increment;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string CommandHandler::cmdDecrBy(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'decrby' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    long long decrement = parseInt(args[2]);
    
    RedisValue* value = db.getValue(key);
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current -= decrement;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string CommandHandler::cmdStrlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'strlen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::STRING) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->string_value.length());
}

std::string CommandHandler::cmdAppend(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'append' command");
    }
    
    const std::string& key = args[1];
    const std::string& append_value = args[2];
    
    RedisValue* value = db.getValue(key);
    std::string result;
    
    if (value && value->type == RedisValue::Type::STRING) {
        result = value->string_value + append_value;
    } else {
        result = append_value;
    }
    
    db.setValue(key, RedisValue(result));
    return RESPFormatter::formatInteger(result.length());
}

std::string CommandHandler::cmdMget(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'mget' command");
    }
    
    std::vector<std::string> results;
    for (size_t i = 1; i < args.size(); i++) {
        RedisValue* value = db.getValue(args[i]);
        if (value && value->type == RedisValue::Type::STRING) {
            results.push_back(value->string_value);
        } else {
            results.push_back(""); // Will be formatted as NULL
        }
    }
    
    return RESPFormatter::formatArray(results);
}

std::string CommandHandler::cmdMset(const std::vector<std::string>& args) {
    if (args.size() < 3 || args.size() % 2 == 0) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'mset' command");
    }
    
    for (size_t i = 1; i < args.size(); i += 2) {
        db.setValue(args[i], RedisValue(args[i + 1]));
    }
    
    return RESPFormatter::formatSimpleString("OK");
}

// List Commands Implementation
std::string CommandHandler::cmdLpush(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lpush' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (value && value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        db.setValue(key, RedisValue(RedisValue::Type::LIST));
        value = db.getValue(key);
    }
    
    for (size_t i = args.size() - 1; i >= 2; i--) {
        value->list_value.push_front(args[i]);
    }
    
    return RESPFormatter::formatInteger(value->list_value.size());
}

std::string CommandHandler::cmdRpush(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'rpush' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (value && value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        db.setValue(key, RedisValue(RedisValue::Type::LIST));
        value = db.getValue(key);
    }
    
    for (size_t i = 2; i < args.size(); i++) {
        value->list_value.push_back(args[i]);
    }
    
    return RESPFormatter::formatInteger(value->list_value.size());
}

std::string CommandHandler::cmdLpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lpop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::LIST || value->list_value.empty()) {
        return RESPFormatter::formatNull();
    }
    
    std::string result = value->list_value.front();
    value->list_value.pop_front();
    
    if (value->list_value.empty()) {
        db.deleteKey(key);
    }
    
    return RESPFormatter::formatBulkString(result);
}

std::string CommandHandler::cmdRpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'rpop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::LIST || value->list_value.empty()) {
        return RESPFormatter::formatNull();
    }
    
    std::string result = value->list_value.back();
    value->list_value.pop_back();
    
    if (value->list_value.empty()) {
        db.deleteKey(key);
    }
    
    return RESPFormatter::formatBulkString(result);
}

std::string CommandHandler::cmdLlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'llen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->list_value.size());
}

std::string CommandHandler::cmdLrange(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lrange' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2]) || !isInteger(args[3])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    long long start = parseInt(args[2]);
    long long end = parseInt(args[3]);
    
    const auto& list = value->list_value;
    long long list_size = static_cast<long long>(list.size());
    
    // Handle negative indices
    if (start < 0) start += list_size;
    if (end < 0) end += list_size;
    
    // Clamp to valid range
    start = std::max(0LL, std::min(start, list_size - 1));
    end = std::max(0LL, std::min(end, list_size - 1));
    
    std::vector<std::string> result;
    if (start <= end) {
        auto it = list.begin();
        std::advance(it, start);
        
        for (long long i = start; i <= end && it != list.end(); i++, ++it) {
            result.push_back(*it);
        }
    }
    
    return RESPFormatter::formatArray(result);
}

std::string CommandHandler::cmdLindex(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lindex' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatNull();
    }
    
    long long index = parseInt(args[2]);
    const auto& list = value->list_value;
    long long list_size = static_cast<long long>(list.size());
    
    // Handle negative index
    if (index < 0) index += list_size;
    
    if (index < 0 || index >= list_size) {
        return RESPFormatter::formatNull();
    }
    
    auto it = list.begin();
    std::advance(it, index);
    return RESPFormatter::formatBulkString(*it);
}

std::string CommandHandler::cmdLset(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lset' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatError("ERR no such key");
    }
    
    long long index = parseInt(args[2]);
    auto& list = value->list_value;
    long long list_size = static_cast<long long>(list.size());
    
    // Handle negative index
    if (index < 0) index += list_size;
    
    if (index < 0 || index >= list_size) {
        return RESPFormatter::formatError("ERR index out of range");
    }
    
    auto it = list.begin();
    std::advance(it, index);
    *it = args[3];
    
    return RESPFormatter::formatSimpleString("OK");
}

// Set Commands Implementation
std::string CommandHandler::cmdSadd(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'sadd' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (value && value->type != RedisValue::Type::SET) {
        return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        db.setValue(key, RedisValue(RedisValue::Type::SET));
        value = db.getValue(key);
    }
    
    int added = 0;
    for (size_t i = 2; i < args.size(); i++) {
        if (value->set_value.insert(args[i]).second) {
            added++;
        }
    }
    
    return RESPFormatter::formatInteger(added);
}

std::string CommandHandler::cmdSrem(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'srem' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET) {
        return RESPFormatter::formatInteger(0);
    }
    
    int removed = 0;
    for (size_t i = 2; i < args.size(); i++) {
        if (value->set_value.erase(args[i]) > 0) {
            removed++;
        }
    }
    
    if (value->set_value.empty()) {
        db.deleteKey(key);
    }
    
    return RESPFormatter::formatInteger(removed);
}

std::string CommandHandler::cmdSismember(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'sismember' command");
    }
    
    const std::string& key = args[1];
    const std::string& member = args[2];
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::SET) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->set_value.count(member) > 0 ? 1 : 0);
}

std::string CommandHandler::cmdScard(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'scard' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->set_value.size());
}

std::string CommandHandler::cmdSmembers(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'smembers' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> members(value->set_value.begin(), value->set_value.end());
    return RESPFormatter::formatArray(members);
}

std::string CommandHandler::cmdSpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'spop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET || value->set_value.empty()) {
        return RESPFormatter::formatNull();
    }
    
    // Get random element
    auto it = value->set_value.begin();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, value->set_value.size() - 1);
    std::advance(it, dis(gen));
    
    std::string result = *it;
    value->set_value.erase(it);
    
    if (value->set_value.empty()) {
        db.deleteKey(key);
    }
    
    return RESPFormatter::formatBulkString(result);
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
    if (!isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value) {
        return RESPFormatter::formatInteger(0);
    }
    
    long long seconds = parseInt(args[2]);
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
    if (!isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value) {
        return RESPFormatter::formatInteger(0);
    }
    
    long long timestamp = parseInt(args[2]);
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

bool CommandHandler::isValidKey(const std::string& key) {
    return !key.empty() && key.length() < 512;
}

bool CommandHandler::isInteger(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        if (str.length() == 1) return false;
        start = 1;
    }
    
    for (size_t i = start; i < str.length(); i++) {
        if (!std::isdigit(str[i])) return false;
    }
    
    return true;
}

long long CommandHandler::parseInt(const std::string& str) {
    try {
        return std::stoll(str);
    } catch (...) {
        return 0;
    }
}

std::string CommandHandler::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool CommandHandler::matchPattern(const std::string& pattern, const std::string& str) {
    if (pattern == "*") return true;
    
    // Simple pattern matching - supports * and ?
    size_t p = 0, s = 0;
    size_t star_p = std::string::npos, star_s = 0;
    
    while (s < str.length()) {
        if (p < pattern.length() && (pattern[p] == '?' || pattern[p] == str[s])) {
            p++;
            s++;
        } else if (p < pattern.length() && pattern[p] == '*') {
            star_p = p++;
            star_s = s;
        } else if (star_p != std::string::npos) {
            p = star_p + 1;
            s = ++star_s;
        } else {
            return false;
        }
    }
    
    while (p < pattern.length() && pattern[p] == '*') {
        p++;
    }
    
    return p == pattern.length();
}
