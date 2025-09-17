#ifndef TCP_SERVER_H
#define TCP_SERVER_H


#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "resp_parser.h"
#include "redis_store.h"
#include "redis_command_handler.h"
#include "connection_manager.h"

class TCPServer {
private:
    static const size_t BUFFER_SIZE = 4096;

    int server_fd;
    int port;
    std::atomic<bool> running;
    
    // Redis-like storage
    RedisStore redis_store;
    RedisCommandHandler command_handler;
    ConnectionManager connection_manager; 
    void acceptConnections();
    void handleClient(int client_socket);
    
public:
    TCPServer(int port = 6379);
    ~TCPServer();

    // Prevent copying
    TCPServer(const TCPServer&) = delete;
    TCPServer& operator=(const TCPServer&) = delete;

    void start();
    void stop();
    bool isRunning() const { return running; }
    int getPort() const { return port; }
    size_t getActiveConnections();
};

#endif // TCP_SERVER_H