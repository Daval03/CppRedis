#include "set_commands.h"

SetCommands::SetCommands(RedisDatabase& database) : db(database) {}

std::string SetCommands::cmdSadd(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'sadd' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (value && value->type != RedisType::SET) {
        return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
    }
    
    if (!value) {
        db.setValue(key, RedisValue(RedisType::SET));
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

std::string SetCommands::cmdSrem(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'srem' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::SET) {
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

std::string SetCommands::cmdSismember(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'sismember' command");
    }
    
    const std::string& key = args[1];
    const std::string& member = args[2];
    
    RedisValue* value = db.getValue(key);
    if (!value || value->type != RedisType::SET) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->set_value.count(member) > 0 ? 1 : 0);
}

std::string SetCommands::cmdScard(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'scard' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::SET) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->set_value.size());
}

std::string SetCommands::cmdSmembers(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'smembers' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::SET) {
        return RESPFormatter::formatArray(std::vector<std::string>());
    }
    
    std::vector<std::string> members(value->set_value.begin(), value->set_value.end());
    return RESPFormatter::formatArray(members);
}

std::string SetCommands::cmdSpop(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'spop' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::SET || value->set_value.empty()) {
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