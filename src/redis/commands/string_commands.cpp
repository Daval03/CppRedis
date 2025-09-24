#include "string_commands.h"
#include <chrono>

StringCommands::StringCommands(RedisDatabase& database) : db(database) {
}

std::string StringCommands::cmdSet(const std::vector<std::string>& args) {
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
        
        std::string param = UtilityFunctions::toUpper(args[i]);
        if (param == "EX") {
            if (!UtilityFunctions::isInteger(args[i + 1])) {
                return RESPFormatter::formatError("ERR value is not an integer or out of range");
            }
            redis_value.setExpiry(std::chrono::seconds(UtilityFunctions::parseInt(args[i + 1])));
        } else if (param == "PX") {
            if (!UtilityFunctions::isInteger(args[i + 1])) {
                return RESPFormatter::formatError("ERR value is not an integer or out of range");
            }
            redis_value.setExpiry(std::chrono::milliseconds(UtilityFunctions::parseInt(args[i + 1])));
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

std::string StringCommands::cmdGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'get' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::STRING) {
        return RESPFormatter::formatNull();
    }
    
    return RESPFormatter::formatBulkString(value->string_value);
}

std::string StringCommands::cmdDel(const std::vector<std::string>& args) {
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

std::string StringCommands::cmdExists(const std::vector<std::string>& args) {
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

std::string StringCommands::cmdType(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'type' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value) {
        return RESPFormatter::formatSimpleString("none");
    }
    
    switch (value->type) {
        case RedisType::STRING: return RESPFormatter::formatSimpleString("string");
        case RedisType::LIST: return RESPFormatter::formatSimpleString("list");
        case RedisType::SET: return RESPFormatter::formatSimpleString("set");
        case RedisType::HASH: return RESPFormatter::formatSimpleString("hash");
        case RedisType::ZSET: return RESPFormatter::formatSimpleString("zset");
        default: return RESPFormatter::formatSimpleString("unknown");
    }
}

std::string StringCommands::cmdIncr(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'incr' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    long long current = 0;
    if (value) {
        if (value->type != RedisType::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!UtilityFunctions::isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = UtilityFunctions::parseInt(value->string_value);
    }
    
    current++;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string StringCommands::cmdDecr(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'decr' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    long long current = 0;
    if (value) {
        if (value->type != RedisType::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!UtilityFunctions::isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = UtilityFunctions::parseInt(value->string_value);
    }
    
    current--;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string StringCommands::cmdIncrBy(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'incrby' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    long long increment = UtilityFunctions::parseInt(args[2]);
    
    RedisValue* value = db.getValue(key);
    long long current = 0;
    if (value) {
        if (value->type != RedisType::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!UtilityFunctions::isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = UtilityFunctions::parseInt(value->string_value);
    }
    
    current += increment;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string StringCommands::cmdDecrBy(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'decrby' command");
    }
    
    const std::string& key = args[1];
    if (!UtilityFunctions::isInteger(args[2])) {
        return RESPFormatter::formatError("ERR value is not an integer or out of range");
    }
    long long decrement = UtilityFunctions::parseInt(args[2]);
    
    RedisValue* value = db.getValue(key);
    long long current = 0;
    if (value) {
        if (value->type != RedisType::STRING) {
            return RESPFormatter::formatError("ERR Operation against a key holding the wrong kind of value");
        }
        if (!UtilityFunctions::isInteger(value->string_value)) {
            return RESPFormatter::formatError("ERR value is not an integer or out of range");
        }
        current = UtilityFunctions::parseInt(value->string_value);
    }
    
    current -= decrement;
    db.setValue(key, RedisValue(std::to_string(current)));
    return RESPFormatter::formatInteger(current);
}

std::string StringCommands::cmdStrlen(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'strlen' command");
    }
    
    const std::string& key = args[1];
    RedisValue* value = db.getValue(key);
    
    if (!value || value->type != RedisType::STRING) {
        return RESPFormatter::formatInteger(0);
    }
    
    return RESPFormatter::formatInteger(value->string_value.length());
}

std::string StringCommands::cmdAppend(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'append' command");
    }
    
    const std::string& key = args[1];
    const std::string& append_value = args[2];
    
    RedisValue* value = db.getValue(key);
    std::string result;
    
    if (value && value->type == RedisType::STRING) {
        result = value->string_value + append_value;
    } else {
        result = append_value;
    }
    
    db.setValue(key, RedisValue(result));
    return RESPFormatter::formatInteger(result.length());
}

std::string StringCommands::cmdMget(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'mget' command");
    }
    
    std::vector<std::string> results;
    for (size_t i = 1; i < args.size(); i++) {
        RedisValue* value = db.getValue(args[i]);
        if (value && value->type == RedisType::STRING) {
            results.push_back(value->string_value);
        } else {
            results.push_back(""); // Will be formatted as NULL
        }
    }
    
    return RESPFormatter::formatArray(results);
}

std::string StringCommands::cmdMset(const std::vector<std::string>& args) {
    if (args.size() < 3 || args.size() % 2 == 0) {
        return RESPFormatter::formatError("ERR wrong number of arguments for 'mset' command");
    }
    
    for (size_t i = 1; i < args.size(); i += 2) {
        db.setValue(args[i], RedisValue(args[i + 1]));
    }
    
    return RESPFormatter::formatSimpleString("OK");
}