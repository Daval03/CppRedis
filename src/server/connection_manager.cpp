#include "connection_manager.h"

ConnectionManager::~ConnectionManager() {
    stopAllConnections();
}

bool ConnectionManager::canAcceptNewConnection() const {
    return active_connections < MAX_CLIENTS;
}

void ConnectionManager::addConnection(std::function<void()> handler) {
    std::lock_guard<std::mutex> lock(threads_mutex);
    
    // Clean up finished threads before adding new one
    cleanupFinishedThreads();
    
    if (client_threads.size() >= MAX_CLIENTS) {
        throw std::runtime_error("Maximum connections reached");
    }
    
    client_threads.emplace_back([this, handler]() {
        active_connections++;
        try {
            handler();
        } catch (const std::exception& e) {
            std::cerr << "Connection handler error: " << e.what() << std::endl;
        }
        active_connections--;
    });
}

void ConnectionManager::stopAllConnections() {
    std::lock_guard<std::mutex> lock(threads_mutex);
    
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    client_threads.clear();
    active_connections = 0;
}

size_t ConnectionManager::getActiveConnections() const {
    return active_connections;
}

void ConnectionManager::cleanupFinishedThreads() {
    auto it = std::remove_if(client_threads.begin(), client_threads.end(),
        [](std::thread& t) {
            return !t.joinable();
        });
    client_threads.erase(it, client_threads.end());
}