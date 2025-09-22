#include "command_handler.h"

CommandHandler::CommandHandler() 
    : start_time(std::chrono::system_clock::now()),
      string_commands(std::make_unique<StringCommands>(db)),
      list_commands(std::make_unique<ListCommands>(db)),
      set_commands(std::make_unique<SetCommands>(db)),
      hash_commands(std::make_unique<HashCommands>(db)),
      ttl_commands(std::make_unique<TTLCommands>(db)),
      server_commands(std::make_unique<ServerCommands>(db, start_time, total_commands_processed)) {
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
    commands["HSET"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHset(args); };
    commands["HGET"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHget(args); };
    commands["HDEL"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHdel(args); };
    commands["HEXISTS"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHexists(args); };
    commands["HLEN"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHlen(args); };
    commands["HKEYS"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHkeys(args); };
    commands["HVALS"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHvals(args); };
    commands["HGETALL"] = [this](const std::vector<std::string>& args) { return hash_commands->cmdHgetall(args); };
    
    // TTL commands
    commands["EXPIRE"] = [this](const std::vector<std::string>& args) { return ttl_commands->cmdExpire(args); };
    commands["EXPIREAT"] = [this](const std::vector<std::string>& args) { return ttl_commands->cmdExpireat(args); };
    commands["TTL"] = [this](const std::vector<std::string>& args) { return ttl_commands->cmdTtl(args); };
    commands["PERSIST"] = [this](const std::vector<std::string>& args) { return ttl_commands->cmdPersist(args); };
    
    // Server commands
    commands["PING"] = [this](const std::vector<std::string>& args) { return server_commands->cmdPing(args); };
    commands["ECHO"] = [this](const std::vector<std::string>& args) { return server_commands->cmdEcho(args); };
    commands["INFO"] = [this](const std::vector<std::string>& args) { return server_commands->cmdInfo(args); };
    commands["FLUSHALL"] = [this](const std::vector<std::string>& args) { return server_commands->cmdFlushall(args); };
    commands["KEYS"] = [this](const std::vector<std::string>& args) { return server_commands->cmdKeys(args); };
    commands["DBSIZE"] = [this](const std::vector<std::string>& args) { return server_commands->cmdDbsize(args); };
    commands["TIME"] = [this](const std::vector<std::string>& args) { return server_commands->cmdTime(args); };
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