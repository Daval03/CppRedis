#include "tcp_server.h"

TCPServer::TCPServer(int port) : port(port), running(false), command_handler(redis_store) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        close(server_fd);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }
    
    std::cout << "TCP Server initialized on port " << port << std::endl;
}

TCPServer::~TCPServer() {
    stop();
    if (server_fd != -1) {
        close(server_fd);
    }
    std::cout << "TCP Server destroyed. Final store size: " << redis_store.size() << " keys" << std::endl;
}

void TCPServer::start() {
    if (running) {
        std::cout << "Server is already running" << std::endl;
        return;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
    }

    if (listen(server_fd, ConnectionManager::MAX_CLIENTS) < 0) {  // Use ConnectionManager constant
        throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
    }

    running = true;
    std::cout << "Redis Mock Server started on port " << port << std::endl;
    std::cout << "Max connections: " << ConnectionManager::MAX_CLIENTS << std::endl;  // Use ConnectionManager constant
    std::cout << "Usage: redis-cli -p " << port << " or telnet localhost " << port << std::endl;

    acceptConnections();
}

void TCPServer::stop() {
    if (!running) return;
    
    std::cout << "Stopping Redis server..." << std::endl;
    running = false;
    
    // Close the server socket to break out of accept()
    if (server_fd != -1) {
        shutdown(server_fd, SHUT_RDWR);
    }    
    // Stop all connections through ConnectionManager
    connection_manager.stopAllConnections();
    
    std::cout << "Redis server stopped. Final store size: " << redis_store.size() << " keys" << std::endl;
}

void TCPServer::acceptConnections() {
    while (running) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (running) {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            }
            continue;
        }

        // Check connection limit using ConnectionManager
        if (!connection_manager.canAcceptNewConnection()) {
            std::cerr << "Maximum connections reached, rejecting client" << std::endl;
            close(client_socket);
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New Redis connection from: " << client_ip << ":" << ntohs(client_addr.sin_port) 
                  << " (Active: " << connection_manager.getActiveConnections() + 1 << ")" << std::endl;

        // Handle client through ConnectionManager
        try {
            connection_manager.addConnection([this, client_socket]() {
                handleClient(client_socket);
            });
        } catch (const std::exception& e) {
            std::cerr << "Failed to add connection: " << e.what() << std::endl;
            close(client_socket);
        }
    }
}

size_t TCPServer::getActiveConnections() {
    return connection_manager.getActiveConnections();
}

void TCPServer::handleClient(int client_socket) {
    std::string buffer;            // persistent buffer
    std::vector<char> tmp(BUFFER_SIZE);

    while (running) {
        ssize_t n = read(client_socket, tmp.data(), tmp.size());
        if (n <= 0) break;

        buffer.append(tmp.data(), n);

        while (true) {
            std::vector<std::string> args;
            size_t consumed = 0;
            if (!RESPParser::parse(buffer, args, consumed))
                break; // msg not complete

            logCommand(args, consumed);
            
            buffer.erase(0, consumed);

            std::string resp = command_handler.processCommand(args);
            if (!resp.empty())
                send(client_socket, resp.c_str(), resp.size(), 0);
        }
    }
    close(client_socket);
}
