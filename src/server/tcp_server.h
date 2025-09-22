#ifndef TCP_SERVER_H
#define TCP_SERVER_H


#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>


#include "connection_manager.h"
#include "../../src/resp/resp_parser.h"
#include "../../src/redis/command_handler.h"
#include "../../src/utils/logger.h"

class TCPServer {
private:
    static const size_t BUFFER_SIZE = 4096;
    const size_t MAX_BUFFER_SIZE = 1024 * 1024; // 1MB máximo

    int server_fd;
    int port;
    std::atomic<bool> running;
    
    // Redis-like storage
    ConnectionManager connection_manager; 
    CommandHandler command_handler;
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