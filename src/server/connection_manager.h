#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <algorithm>
#include <iostream>

class ConnectionManager {
private:
    
    std::vector<std::thread> client_threads;
    std::mutex threads_mutex;
    std::atomic<size_t> active_connections{0};
    
    void cleanupFinishedThreads();

public:
    static const int MAX_CLIENTS = 100;
    ~ConnectionManager();
    
    bool canAcceptNewConnection() const;
    void addConnection(std::function<void()> handler);
    void stopAllConnections();
    size_t getActiveConnections() const;
};

#endif
