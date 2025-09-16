#include "tcp_server.h"

TCPServer::TCPServer(int port) : port(port), running(false) {
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
}

TCPServer::~TCPServer() {
    stop();
    if (server_fd != -1) {
        close(server_fd);
    }
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

    if (listen(server_fd, 5) < 0) {
        throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
    }

    running = true;
    std::cout << "Server started on port " << port << std::endl;
    
    acceptConnections();
}

void TCPServer::stop() {
    if (!running) return;
    
    running = false;
    
    // Close the server socket to break out of accept()
    if (server_fd != -1) {
        shutdown(server_fd, SHUT_RDWR);
    }
    
    // Join all client threads
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    client_threads.clear();
    
    std::cout << "Server stopped" << std::endl;
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

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New connection from: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        // Handle client in a separate thread
        client_threads.emplace_back(&TCPServer::handleClient, this, client_socket);
    }
}

void TCPServer::handleClient(int client_socket) {
    char buffer[1024] = {0};
    
    while (running) {
        // Read data from client
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::cout << "Client disconnected gracefully" << std::endl;
            } else {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
            }
            break; // Client disconnected or error occurred
        }

        buffer[bytes_read] = '\0';
        
        // Remove newline characters for cleaner output
        std::string message(buffer);
        if (!message.empty() && message.back() == '\n') {
            message.pop_back();
        }
        if (!message.empty() && message.back() == '\r') {
            message.pop_back();
        }
        
        std::cout << "Received from client: '" << message << "'" << std::endl;

        // Echo back to client
        std::string response = "Echo: " + message + "\n";
        if (send(client_socket, response.c_str(), response.length(), 0) < 0) {
            std::cerr << "Send failed: " << strerror(errno) << std::endl;
            break;
        }

        // Check for exit command
        if (message == "exit" || message == "quit") {
            std::string goodbye = "Goodbye!\n";
            send(client_socket, goodbye.c_str(), goodbye.length(), 0);
            break;
        }
    }

    close(client_socket);
}