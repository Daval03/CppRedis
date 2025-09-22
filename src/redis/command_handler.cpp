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

std::string CommandHandler::processCommand(const RESPValue& command) {
    RESPParser parser;
    std::vector<std::string> args = parser.toStringVector(command);
    return processCommand(args);
}

std::string CommandHandler::processCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return formatError("ERR empty command");
    }
    
    std::lock_guard<std::mutex> lock(db_mutex);
    total_commands_processed++;
    
    // Clean up expired keys periodically
    if (total_commands_processed % 100 == 0) {
        cleanupExpiredKeys();
    }
    
    std::string command = toUpper(args[0]);
    auto it = commands.find(command);
    
    if (it == commands.end()) {
        return formatError("ERR unknown command '" + args[0] + "'");
    }
    
    try {
        return it->second(args);
    } catch (const std::exception& e) {
        return formatError("ERR " + std::string(e.what()));
    }
}

// String Commands Implementation
std::string CommandHandler::cmdSet(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'set' command");
    }
    
    const std::string& key = args[1];
    const std::string& value = args[2];
    
    RedisValue redis_value(value);
    
    // Handle optional parameters (EX, PX, NX, XX)
    for (size_t i = 3; i < args.size(); i += 2) {
        if (i + 1 >= args.size()) {
            return formatError("ERR syntax error");
        }
        
        std::string param = toUpper(args[i]);
        if (param == "EX") {
            if (!isInteger(args[i + 1])) {
                return formatError("ERR value is not an integer or out of range");
            }
            redis_value.setExpiry(std::chrono::seconds(parseInt(args[i + 1])));
        } else if (param == "PX") {
            if (!isInteger(args[i + 1])) {
                return formatError("ERR value is not an integer or out of range");
            }
            redis_value.setExpiry(std::chrono::milliseconds(parseInt(args[i + 1])));
        } else if (param == "NX") {
            if (keyExists(key)) {
                return formatNull();
            }
            i--; // NX doesn't have a value
        } else if (param == "XX") {
            if (!keyExists(key)) {
                return formatNull();
            }
            i--; // XX doesn't have a value
        }
    }
    
    setValue(key, redis_value);
    return formatSimpleString("OK");
}

std::string CommandHandler::cmdGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'get' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::STRING) {
        return formatNull();
    }
    
    return formatBulkString(value->string_value);
}

std::string CommandHandler::cmdDel(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return formatError("ERR wrong number of arguments for 'del' command");
    }
    
    int deleted = 0;
    for (size_t i = 1; i < args.size(); i++) {
        if (deleteKey(args[i])) {
            deleted++;
        }
    }
    
    return formatInteger(deleted);
}

std::string CommandHandler::cmdExists(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return formatError("ERR wrong number of arguments for 'exists' command");
    }
    
    int count = 0;
    for (size_t i = 1; i < args.size(); i++) {
        if (keyExists(args[i])) {
            count++;
        }
    }
    
    return formatInteger(count);
}

std::string CommandHandler::cmdType(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'type' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value) {
        return formatSimpleString("none");
    }
    
    switch (value->type) {
        case RedisValue::Type::STRING: return formatSimpleString("string");
        case RedisValue::Type::LIST: return formatSimpleString("list");
        case RedisValue::Type::SET: return formatSimpleString("set");
        case RedisValue::Type::HASH: return formatSimpleString("hash");
        case RedisValue::Type::ZSET: return formatSimpleString("zset");
        default: return formatSimpleString("unknown");
    }
}

std::string CommandHandler::cmdIncr(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'incr' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current++;
    setValue(key, RedisValue(std::to_string(current)));
    return formatInteger(current);
}

std::string CommandHandler::cmdDecr(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'decr' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current--;
    setValue(key, RedisValue(std::to_string(current)));
    return formatInteger(current);
}

std::string CommandHandler::cmdIncrBy(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'incrby' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return formatError("ERR value is not an integer or out of range");
    }
    long long increment = parseInt(args[2]);
    
    RedisValue* value = getValue(key);
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current += increment;
    setValue(key, RedisValue(std::to_string(current)));
    return formatInteger(current);
}

std::string CommandHandler::cmdDecrBy(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'decrby' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return formatError("ERR value is not an integer or out of range");
    }
    long long decrement = parseInt(args[2]);
    
    RedisValue* value = getValue(key);
    long long current = 0;
    if (value) {
        if (value->type != RedisValue::Type::STRING) {
            return formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!isInteger(value->string_value)) {
            return formatError("ERR value is not an integer or out of range");
        }
        current = parseInt(value->string_value);
    }
    
    current -= decrement;
    setValue(key, RedisValue(std::to_string(current)));
    return formatInteger(current);
}

std::string CommandHandler::cmdStrlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'strlen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::STRING) {
        return formatInteger(0);
    }
    
    return formatInteger(value->string_value.length());
}

std::string CommandHandler::cmdAppend(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'append' command");
    }
    
    const std::string& key = args[1];
    const std::string& append_value = args[2];
    
    RedisValue* value = getValue(key);
    std::string result;
    
    if (value && value->type == RedisValue::Type::STRING) {
        result = value->string_value + append_value;
    } else {
        result = append_value;
    }
    
    setValue(key, RedisValue(result));
    return formatInteger(result.length());
}

std::string CommandHandler::cmdMget(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return formatError("ERR wrong number of arguments for 'mget' command");
    }
    
    std::vector<std::string> results;
    for (size_t i = 1; i < args.size(); i++) {
        RedisValue* value = getValue(args[i]);
        if (value && value->type == RedisValue::Type::STRING) {
            results.push_back(value->string_value);
        } else {
            results.push_back(""); // Will be formatted as NULL
        }
    }
    
    return formatArray(results);
}

std::string CommandHandler::cmdMset(const std::vector<std::string>& args) {
    if (args.size() < 3 || args.size() % 2 == 0) {
        return formatError("ERR wrong number of arguments for 'mset' command");
    }
    
    for (size_t i = 1; i < args.size(); i += 2) {
        setValue(args[i], RedisValue(args[i + 1]));
    }
    
    return formatSimpleString("OK");
}

// List Commands Implementation
std::string CommandHandler::cmdLpush(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'lpush' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (value && value->type != RedisValue::Type::LIST) {
        return formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        setValue(key, RedisValue(RedisValue::Type::LIST));
        value = getValue(key);
    }
    
    for (size_t i = args.size() - 1; i >= 2; i--) {
        value->list_value.push_front(args[i]);
    }
    
    return formatInteger(value->list_value.size());
}

std::string CommandHandler::cmdRpush(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'rpush' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (value && value->type != RedisValue::Type::LIST) {
        return formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        setValue(key, RedisValue(RedisValue::Type::LIST));
        value = getValue(key);
    }
    
    for (size_t i = 2; i < args.size(); i++) {
        value->list_value.push_back(args[i]);
    }
    
    return formatInteger(value->list_value.size());
}

std::string CommandHandler::cmdLpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'lpop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::LIST || value->list_value.empty()) {
        return formatNull();
    }
    
    std::string result = value->list_value.front();
    value->list_value.pop_front();
    
    if (value->list_value.empty()) {
        deleteKey(key);
    }
    
    return formatBulkString(result);
}

std::string CommandHandler::cmdRpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'rpop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::LIST || value->list_value.empty()) {
        return formatNull();
    }
    
    std::string result = value->list_value.back();
    value->list_value.pop_back();
    
    if (value->list_value.empty()) {
        deleteKey(key);
    }
    
    return formatBulkString(result);
}

std::string CommandHandler::cmdLlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'llen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::LIST) {
        return formatInteger(0);
    }
    
    return formatInteger(value->list_value.size());
}

std::string CommandHandler::cmdLrange(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return formatError("ERR wrong number of arguments for 'lrange' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2]) || !isInteger(args[3])) {
        return formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return formatArray(std::vector<std::string>());
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
    
    return formatArray(result);
}

std::string CommandHandler::cmdLindex(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'lindex' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return formatNull();
    }
    
    long long index = parseInt(args[2]);
    const auto& list = value->list_value;
    long long list_size = static_cast<long long>(list.size());
    
    // Handle negative index
    if (index < 0) index += list_size;
    
    if (index < 0 || index >= list_size) {
        return formatNull();
    }
    
    auto it = list.begin();
    std::advance(it, index);
    return formatBulkString(*it);
}

std::string CommandHandler::cmdLset(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return formatError("ERR wrong number of arguments for 'lset' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return formatError("ERR no such key");
    }
    
    long long index = parseInt(args[2]);
    auto& list = value->list_value;
    long long list_size = static_cast<long long>(list.size());
    
    // Handle negative index
    if (index < 0) index += list_size;
    
    if (index < 0 || index >= list_size) {
        return formatError("ERR index out of range");
    }
    
    auto it = list.begin();
    std::advance(it, index);
    *it = args[3];
    
    return formatSimpleString("OK");
}

// Set Commands Implementation
std::string CommandHandler::cmdSadd(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'sadd' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (value && value->type != RedisValue::Type::SET) {
        return formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        setValue(key, RedisValue(RedisValue::Type::SET));
        value = getValue(key);
    }
    
    int added = 0;
    for (size_t i = 2; i < args.size(); i++) {
        if (value->set_value.insert(args[i]).second) {
            added++;
        }
    }
    
    return formatInteger(added);
}

std::string CommandHandler::cmdSrem(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'srem' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET) {
        return formatInteger(0);
    }
    
    int removed = 0;
    for (size_t i = 2; i < args.size(); i++) {
        if (value->set_value.erase(args[i]) > 0) {
            removed++;
        }
    }
    
    if (value->set_value.empty()) {
        deleteKey(key);
    }
    
    return formatInteger(removed);
}

std::string CommandHandler::cmdSismember(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'sismember' command");
    }
    
    const std::string& key = args[1];
    const std::string& member = args[2];
    
    RedisValue* value = getValue(key);
    if (!value || value->type != RedisValue::Type::SET) {
        return formatInteger(0);
    }
    
    return formatInteger(value->set_value.count(member) > 0 ? 1 : 0);
}

std::string CommandHandler::cmdScard(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'scard' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET) {
        return formatInteger(0);
    }
    
    return formatInteger(value->set_value.size());
}

std::string CommandHandler::cmdSmembers(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'smembers' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET) {
        return formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> members(value->set_value.begin(), value->set_value.end());
    return formatArray(members);
}

std::string CommandHandler::cmdSpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'spop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::SET || value->set_value.empty()) {
        return formatNull();
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
        deleteKey(key);
    }
    
    return formatBulkString(result);
}

// Hash Commands Implementation
std::string CommandHandler::cmdHset(const std::vector<std::string>& args) {
    if (args.size() < 4 || args.size() % 2 != 0) {
        return formatError("ERR wrong number of arguments for 'hset' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (value && value->type != RedisValue::Type::HASH) {
        return formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        setValue(key, RedisValue(RedisValue::Type::HASH));
        value = getValue(key);
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
    
    return formatInteger(added);
}

std::string CommandHandler::cmdHget(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'hget' command");
    }
    
    const std::string& key = args[1];
    const std::string& field = args[2];
    
    RedisValue* value = getValue(key);
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatNull();
    }
    
    auto it = value->hash_value.find(field);
    if (it == value->hash_value.end()) {
        return formatNull();
    }
    
    return formatBulkString(it->second);
}

std::string CommandHandler::cmdHdel(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return formatError("ERR wrong number of arguments for 'hdel' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatInteger(0);
    }
    
    int deleted = 0;
    for (size_t i = 2; i < args.size(); i++) {
        if (value->hash_value.erase(args[i]) > 0) {
            deleted++;
        }
    }
    
    if (value->hash_value.empty()) {
        deleteKey(key);
    }
    
    return formatInteger(deleted);
}

std::string CommandHandler::cmdHexists(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'hexists' command");
    }
    
    const std::string& key = args[1];
    const std::string& field = args[2];
    
    RedisValue* value = getValue(key);
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatInteger(0);
    }
    
    return formatInteger(value->hash_value.count(field) > 0 ? 1 : 0);
}

std::string CommandHandler::cmdHlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'hlen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatInteger(0);
    }
    
    return formatInteger(value->hash_value.size());
}

std::string CommandHandler::cmdHkeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'hkeys' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> keys;
    for (const auto& pair : value->hash_value) {
        keys.push_back(pair.first);
    }
    
    return formatArray(keys);
}

std::string CommandHandler::cmdHvals(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'hvals' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> values;
    for (const auto& pair : value->hash_value) {
        values.push_back(pair.second);
    }
    
    return formatArray(values);
}

std::string CommandHandler::cmdHgetall(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'hgetall' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value || value->type != RedisValue::Type::HASH) {
        return formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> result;
    for (const auto& pair : value->hash_value) {
        result.push_back(pair.first);
        result.push_back(pair.second);
    }
    
    return formatArray(result);
}

// TTL Commands Implementation
std::string CommandHandler::cmdExpire(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'expire' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = getValue(key);
    if (!value) {
        return formatInteger(0);
    }
    
    long long seconds = parseInt(args[2]);
    if (seconds <= 0) {
        deleteKey(key);
        return formatInteger(1);
    }
    
    value->setExpiry(std::chrono::seconds(seconds));
    return formatInteger(1);
}

std::string CommandHandler::cmdExpireat(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return formatError("ERR wrong number of arguments for 'expireat' command");
    }
    
    const std::string& key = args[1];
    if (!isInteger(args[2])) {
        return formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = getValue(key);
    if (!value) {
        return formatInteger(0);
    }
    
    long long timestamp = parseInt(args[2]);
    auto expiry_time = std::chrono::system_clock::from_time_t(timestamp);
    
    if (expiry_time <= std::chrono::system_clock::now()) {
        deleteKey(key);
        return formatInteger(1);
    }
    
    value->expiry = expiry_time;
    value->has_expiry = true;
    return formatInteger(1);
}

std::string CommandHandler::cmdTtl(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'ttl' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value) {
        return formatInteger(-2); // Key doesn't exist
    }
    
    if (!value->has_expiry) {
        return formatInteger(-1); // Key exists but has no expiry
    }
    
    auto now = std::chrono::system_clock::now();
    if (now >= value->expiry) {
        deleteKey(key);
        return formatInteger(-2); // Key expired
    }
    
    auto ttl_seconds = std::chrono::duration_cast<std::chrono::seconds>(value->expiry - now);
    return formatInteger(ttl_seconds.count());
}

std::string CommandHandler::cmdPersist(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'persist' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = getValue(key);
    
    if (!value) {
        return formatInteger(0);
    }
    
    if (!value->has_expiry) {
        return formatInteger(0);
    }
    
    value->clearExpiry();
    return formatInteger(1);
}

// Server Commands Implementation
std::string CommandHandler::cmdPing(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        return formatError("ERR wrong number of arguments for 'ping' command");
    }
    
    if (args.size() == 2) {
        return formatBulkString(args[1]);
    }
    
    return formatSimpleString("PONG");
}

std::string CommandHandler::cmdEcho(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'echo' command");
    }
    
    return formatBulkString(args[1]);
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
    info << "db0:keys=" << database.size() << "\r\n";
    
    return formatBulkString(info.str());
}

std::string CommandHandler::cmdFlushall(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        return formatError("ERR wrong number of arguments for 'flushall' command");
    }
    
    database.clear();
    return formatSimpleString("OK");
}

std::string CommandHandler::cmdKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return formatError("ERR wrong number of arguments for 'keys' command");
    }
    
    const std::string& pattern = args[1];
    std::vector<std::string> matching_keys;
    
    for (const auto& pair : database) {
        if (!pair.second.isExpired() && matchPattern(pattern, pair.first)) {
            matching_keys.push_back(pair.first);
        }
    }
    
    return formatArray(matching_keys);
}

std::string CommandHandler::cmdDbsize(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return formatError("ERR wrong number of arguments for 'dbsize' command");
    }
    
    cleanupExpiredKeys();
    return formatInteger(database.size());
}

std::string CommandHandler::cmdTime(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return formatError("ERR wrong number of arguments for 'time' command");
    }
    
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
    
    std::vector<std::string> time_result = {
        std::to_string(time_t_now),
        std::to_string(microseconds)
    };
    
    return formatArray(time_result);
}

// Helper Methods Implementation
std::string CommandHandler::formatError(const std::string& message) {
    return "-" + message + "\r\n";
}

std::string CommandHandler::formatSimpleString(const std::string& str) {
    return "+" + str + "\r\n";
}

std::string CommandHandler::formatBulkString(const std::string& str) {
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string CommandHandler::formatInteger(long long value) {
    return ":" + std::to_string(value) + "\r\n";
}

std::string CommandHandler::formatArray(const std::vector<std::string>& items) {
    std::string response = "*" + std::to_string(items.size()) + "\r\n";
    for (const auto& item : items) {
        if (item.empty()) {
            response += formatNull();
        } else {
            response += formatBulkString(item);
        }
    }
    return response;
}

std::string CommandHandler::formatNull() {
    return "$-1\r\n";
}

bool CommandHandler::isValidKey(const std::string& key) {
    return !key.empty() && key.length() < 512;
}

void CommandHandler::cleanupExpiredKeys() {
    auto it = database.begin();
    while (it != database.end()) {
        if (it->second.isExpired()) {
            it = database.erase(it);
        } else {
            ++it;
        }
    }
}

bool CommandHandler::keyExists(const std::string& key) {
    auto it = database.find(key);
    if (it != database.end() && !it->second.isExpired()) {
        return true;
    }
    if (it != database.end() && it->second.isExpired()) {
        database.erase(it);
    }
    return false;
}

RedisValue* CommandHandler::getValue(const std::string& key) {
    auto it = database.find(key);
    if (it != database.end()) {
        if (it->second.isExpired()) {
            database.erase(it);
            return nullptr;
        }
        return &it->second;
    }
    return nullptr;
}

void CommandHandler::setValue(const std::string& key, RedisValue value) {
    database[key] = std::move(value);
}

bool CommandHandler::deleteKey(const std::string& key) {
    return database.erase(key) > 0;
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

void CommandHandler::clearDatabase() {
    std::lock_guard<std::mutex> lock(db_mutex);
    database.clear();
}

size_t CommandHandler::getDatabaseSize() const {
    std::lock_guard<std::mutex> lock(db_mutex);
    return database.size();
}
