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
    std::cout << "Redis server started on port " << port << std::endl;
    std::cout << "Use: redis-cli -p " << port << " or telnet localhost " << port << std::endl;

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
    
    std::cout << "Redis server stopped" << std::endl;
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

std::string TCPServer::processRedisCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "";
    }
    
    std::string cmd = args[0];
    
    // Convert to uppercase for case-insensitive comparison
    for (char& c : cmd) {
        c = std::toupper(c);
    }
    
    if (cmd == "PING") {
        return "+PONG\r\n";
    }
    else if (cmd == "COMMAND") {
        return "*0\r\n";
    }
    else if (cmd == "QUIT") {
        return "+OK\r\n";
    }
    else if (cmd == "ECHO") {
        if (args.size() < 2) {
            return "-ERR wrong number of arguments for 'echo' command\r\n";
        }
        
        // Linking all args of ECHO
        std::string message;
        for (size_t i = 1; i < args.size(); ++i) {
            if (i > 1) message += " ";  // Adding space between arg
            message += args[i];
        }
        
        return "$" + std::to_string(message.length()) + "\r\n" + message + "\r\n";
    }
    else {
        return "-ERR unknown command '" + cmd + "'\r\n";
    }
}

std::vector<std::string> TCPServer::parseRESP(const std::string& input) {
    std::vector<std::string> result;
    
    if (input.empty() || input[0] != '*') {
        // Fallback for commands of flat text
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            result.push_back(token);
        }
        return result;
    }

    std::istringstream ss(input);
    std::string line;
    
    // Read first line *<num>
    if (!std::getline(ss, line)) return result;
    
    if (line.empty() || line[0] != '*') return result;
    
    // Extracting all args
    int num_args;
    try {
        num_args = std::stoi(line.substr(1));
    } catch (...) {
        return result;
    }
    
    // Read each arg
    for (int i = 0; i < num_args; ++i) {
        if (!std::getline(ss, line)) break;
        
        if (line.empty() || line[0] != '$') break;
        
        int len;
        try {
            len = std::stoi(line.substr(1));
        } catch (...) {
            break;
        }
        
        // Read content of arg
        if (!std::getline(ss, line)) break;
       
        // Remove \r 
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        result.push_back(line);
    }
    
    return result;
}

void TCPServer::handleClient(int client_socket) {
    char buffer[1024] = {0};
    
    // Send Redis welcome message (similar to real Redis)
    std::string welcome = "+OK Redis mock server ready\r\n";
    send(client_socket, welcome.c_str(), welcome.length(), 0);
    
    while (running) {
        // Read data from client
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::cout << "Redis client disconnected" << std::endl;
            } else {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
            }
            break;
        }

        buffer[bytes_read] = '\0';
        std::string received_data(buffer);
        
        std::cout << "Received Redis command: " << received_data;
        
        // Process the Redis command
        std::vector<std::string> args = parseRESP(received_data);
        std::string response = processRedisCommand(args);
        
        if (!response.empty()) {
            if (send(client_socket, response.c_str(), response.length(), 0) < 0) {
                std::cerr << "Send failed: " << strerror(errno) << std::endl;
                break;
            }
            
            std::cout << "Sent response: " << response;
            
            // Check for QUIT command to close connection
            if (!args.empty() && args[0] == "QUIT") {
                break;
            }
        }
    }

    close(client_socket);
}