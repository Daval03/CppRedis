#include "list_commands.h"

ListCommands::ListCommands(RedisDatabase& database) : db(database) {}

std::string ListCommands::cmdLpush(const std::vector<std::string>& args) {
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

std::string ListCommands::cmdRpush(const std::vector<std::string>& args) {
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

std::string ListCommands::cmdLpop(const std::vector<std::string>& args) {
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

std::string ListCommands::cmdRpop(const std::vector<std::string>& args) {
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

std::string ListCommands::cmdLlen(const std::vector<std::string>& args) {
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

std::string ListCommands::cmdLrange(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lrange' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2]) || !UtilityFunctions::isInteger(args[3])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    long long start = UtilityFunctions::parseInt(args[2]);
    long long end = UtilityFunctions::parseInt(args[3]);
    
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

std::string ListCommands::cmdLindex(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lindex' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatNull();
    }
    
    long long index = UtilityFunctions::parseInt(args[2]);
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

std::string ListCommands::cmdLset(const std::vector<std::string>& args) {
    if (args.size() != 4) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'lset' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisValue::Type::LIST) {
        return RESPFormatter::formatError("ERR no such key");
    }
    
    long long index = UtilityFunctions::parseInt(args[2]);
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