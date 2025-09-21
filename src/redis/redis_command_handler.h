#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "redis_store.h"
//#include "resp_parser.h"
#include "../../src/resp/resp_parser.h"
#include <algorithm>
#include <iostream>
#include <sstream>

class RedisCommandHandler {
private:
    RedisStore& store;
    int server_port;
    std::unordered_map<std::string, std::function<std::string(const std::vector<std::string>&)>> commands;

    void registerCommands();
    
    // Command implementations
    std::string handlePing(const std::vector<std::string>& args);
    std::string handleEcho(const std::vector<std::string>& args);
    std::string handleSet(const std::vector<std::string>& args);
    std::string handleGet(const std::vector<std::string>& args);
    std::string handleDel(const std::vector<std::string>& args);
    std::string handleExists(const std::vector<std::string>& args);
    std::string handleKeys(const std::vector<std::string>& args);
    std::string handleInfo(const std::vector<std::string>& args);

public:
    explicit RedisCommandHandler(RedisStore& redis_store,int port = 6379);
    std::string processCommand(const std::vector<std::string>& args);

};

#endif