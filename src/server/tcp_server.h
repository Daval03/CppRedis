#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cctype>

class TCPServer {
private:
    static const size_t BUFFER_SIZE = 4096;
    static const size_t MAX_STRING_LENGTH = 1024 * 1024; // 1MB max string
    static const int MAX_CLIENTS = 100;

    int server_fd;
    int port;
    std::atomic<bool> running;
    std::vector<std::thread> client_threads;
    std::mutex threads_mutex;
    
    // Redis-like storage
    std::unordered_map<std::string, std::string> redis_store;
    std::mutex store_mutex;

    void acceptConnections();
    void handleClient(int client_socket);
    std::string processRedisCommand(const std::vector<std::string>& args);
    std::vector<std::string> parseRESP(const std::string& input);
    void cleanupFinishedThreads();
    
    // Redis command implementations
    std::string handlePing(const std::vector<std::string>& args);
    std::string handleEcho(const std::vector<std::string>& args);
    std::string handleSet(const std::vector<std::string>& args);
    std::string handleGet(const std::vector<std::string>& args);
    std::string handleDel(const std::vector<std::string>& args);
    std::string handleExists(const std::vector<std::string>& args);
    std::string handleKeys(const std::vector<std::string>& args);
    std::string handleInfo(const std::vector<std::string>& args);

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