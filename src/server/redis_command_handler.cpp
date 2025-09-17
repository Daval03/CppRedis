#include "redis_command_handler.h"

RedisCommandHandler::RedisCommandHandler(RedisStore& redis_store, int port) 
    : store(redis_store), server_port(port) {
    registerCommands();
}

void RedisCommandHandler::registerCommands() {
    commands = {
        {"PING", [this](const auto& args) { return handlePing(args); }},
        {"ECHO", [this](const auto& args) { return handleEcho(args); }},
        {"SET", [this](const auto& args) { return handleSet(args); }},
        {"GET", [this](const auto& args) { return handleGet(args); }},
        {"DEL", [this](const auto& args) { return handleDel(args); }},
        {"EXISTS", [this](const auto& args) { return handleExists(args); }},
        {"KEYS", [this](const auto& args) { return handleKeys(args); }},
        {"INFO", [this](const auto& args) { return handleInfo(args); }},
        {"COMMAND", [](const auto&) { return "*0\r\n"; }}, // Empty array for COMMAND
        {"QUIT", [](const auto&) { return "+OK\r\n"; }}
    };
}

std::string RedisCommandHandler::processCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return RESPParser::formatError("Protocol error: empty command");
    }
    
    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    auto it = commands.find(cmd);
    if (it != commands.end()) {
        return it->second(args);
    } else {
        return RESPParser::formatError("unknown command '" + args[0] + "', with args beginning with:");
    }
}

std::string RedisCommandHandler::handlePing(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        return RESPParser::formatSimpleString("PONG");
    } else if (args.size() == 2) {
        return RESPParser::formatBulkString(args[1]);
    } else {
        return RESPParser::formatError("wrong number of arguments for 'ping' command");
    }
}

std::string RedisCommandHandler::handleEcho(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPParser::formatError("wrong number of arguments for 'echo' command");
    }
    
    // Concatenate all arguments except the command
    std::string message;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) message += " ";
        message += args[i];
    }
    
    return RESPParser::formatBulkString(message);
}

std::string RedisCommandHandler::handleSet(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return RESPParser::formatError("wrong number of arguments for 'set' command");
    }
    
    store.set(args[1], args[2]);
    return RESPParser::formatSimpleString("OK");
}

std::string RedisCommandHandler::handleGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPParser::formatError("wrong number of arguments for 'get' command");
    }
    
    std::string value = store.get(args[1]);
    if (value.empty()) {
        return RESPParser::formatNull();
    }
    return RESPParser::formatBulkString(value);
}

std::string RedisCommandHandler::handleDel(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPParser::formatError("wrong number of arguments for 'del' command");
    }
    
    std::vector<std::string> keys_to_delete(args.begin() + 1, args.end());
    int deleted_count = store.del(keys_to_delete);
    return RESPParser::formatInteger(deleted_count);
}

std::string RedisCommandHandler::handleExists(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return RESPParser::formatError("wrong number of arguments for 'exists' command");
    }
    
    int exists_count = 0;
    for (size_t i = 1; i < args.size(); ++i) {
        if (store.exists(args[i])) {
            exists_count++;
        }
    }
    
    return RESPParser::formatInteger(exists_count);
}

std::string RedisCommandHandler::handleKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return RESPParser::formatError("wrong number of arguments for 'keys' command");
    }
    
    std::vector<std::string> matching_keys = store.keys(args[1]);
    return RESPParser::formatArray(matching_keys);
}

std::string RedisCommandHandler::handleInfo(const std::vector<std::string>& args) {
    // Si se proporciona una sección específica
    if (args.size() > 1) {
        std::string section = args[1];
        std::transform(section.begin(), section.end(), section.begin(), ::tolower);
        
        if (section == "server") {
            std::string info = "# Server\r\n";
            info += "redis_version:7.0.0-mock\r\n";
            info += "redis_mode:standalone\r\n";
            info += "process_id:1234\r\n";
            info += "tcp_port:" + std::to_string(server_port) + "\r\n";
            return RESPParser::formatBulkString(info);
        }
        else if (section == "clients") {
            std::string info = "# Clients\r\n";
            info += "connected_clients:0\r\n";
            return RESPParser::formatBulkString(info);
        }
        else if (section == "memory") {
            std::string info = "# Memory\r\n";
            info += "used_memory_dataset:" + std::to_string(store.size() * 64) + "\r\n";
            return RESPParser::formatBulkString(info);
        }
        else if (section == "keyspace") {
            std::string info = "# Keyspace\r\n";
            if (store.size() > 0) {
                info += "db0:keys=" + std::to_string(store.size()) + ",expires=0,avg_ttl=0\r\n";
            }
            return RESPParser::formatBulkString(info);
        }
        else {
            // Sección no reconocida - devolver toda la información
            return handleInfo({args[0]}); // Llamar recursivamente sin sección
        }
    }
    
    // Si no se especifica sección, devolver toda la información
    std::string info = "# Server\r\n";
    info += "redis_version:7.0.0-mock\r\n";
    info += "redis_mode:standalone\r\n";
    info += "process_id:1234\r\n";
    info += "tcp_port:" + std::to_string(server_port) + "\r\n";
    info += "\r\n";
    info += "# Clients\r\n";
    info += "connected_clients:0\r\n";
    info += "\r\n";
    info += "# Memory\r\n";
    info += "used_memory_dataset:" + std::to_string(store.size() * 64) + "\r\n";
    info += "\r\n";
    info += "# Keyspace\r\n";
    if (store.size() > 0) {
        info += "db0:keys=" + std::to_string(store.size()) + ",expires=0,avg_ttl=0\r\n";
    }
    
    return RESPParser::formatBulkString(info);
}