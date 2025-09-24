#include "hash_commands.h"

HashCommands::HashCommands(RedisDatabase& database) : db(database) {}

std::string HashCommands::cmdHset(const std::vector<std::string>& args) {
    if (args.size() < 4 || args.size() % 2 != 0) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hset' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (value && value->type != RedisType::HASH) {
        return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        db.setValue(key, RedisValue(RedisType::HASH));
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

std::string HashCommands::cmdHget(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hget' command");
    }
    
    const std::string& key = args[1];
    const std::string& field = args[2];
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisType::HASH) {
        return RESPFormatter::formatNull();
    }
    
    auto it = value->hash_value.find(field);
    if (it == value->hash_value.end()) {
        return RESPFormatter::formatNull();
    }
    
    return RESPFormatter::formatBulkString(it->second);
}

std::string HashCommands::cmdHdel(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hdel' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::HASH) {
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

std::string HashCommands::cmdHexists(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hexists' command");
    }
    
    const std::string& key = args[1];
    const std::string& field = args[2];
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisType::HASH) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->hash_value.count(field) > 0 ? 1 : 0);
}

std::string HashCommands::cmdHlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hlen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::HASH) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->hash_value.size());
}

std::string HashCommands::cmdHkeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hkeys' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::HASH) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> keys;
    for (const auto& pair : value->hash_value) {
        keys.push_back(pair.first);
    }
    
    return RESPFormatter::formatArray(keys);
}

std::string HashCommands::cmdHvals(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hvals' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::HASH) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> values;
    for (const auto& pair : value->hash_value) {
        values.push_back(pair.second);
    }
    
    return RESPFormatter::formatArray(values);
}

std::string HashCommands::cmdHgetall(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'hgetall' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::HASH) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> result;
    for (const auto& pair : value->hash_value) {
        result.push_back(pair.first);
        result.push_back(pair.second);
    }
    
    return RESPFormatter::formatArray(result);
}