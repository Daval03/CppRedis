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
    
    std::cout << "TCP Server initialized on port " << port << std::endl;
}

TCPServer::~TCPServer() {
    stop();
    if (server_fd != -1) {
        close(server_fd);
    }
    std::cout << "TCP Server destroyed" << std::endl;
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

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
    }

    running = true;
    std::cout << "Redis Mock Server started on port " << port << std::endl;
    std::cout << "Max connections: " << MAX_CLIENTS << std::endl;
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
    
    // Join all client threads with proper synchronization
    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        client_threads.clear();
    }
    
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

        // Check connection limit
        {
            std::lock_guard<std::mutex> lock(threads_mutex);
            if (client_threads.size() >= MAX_CLIENTS) {
                std::cerr << "Maximum connections reached, rejecting client" << std::endl;
                close(client_socket);
                continue;
            }
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New Redis connection from: " << client_ip << ":" << ntohs(client_addr.sin_port) 
                  << " (Active: " << getActiveConnections() + 1 << ")" << std::endl;

        // Clean up finished threads before adding new one
        cleanupFinishedThreads();

        // Handle client in a separate thread
        {
            std::lock_guard<std::mutex> lock(threads_mutex);
            client_threads.emplace_back(&TCPServer::handleClient, this, client_socket);
        }
    }
}

void TCPServer::cleanupFinishedThreads() {
    std::lock_guard<std::mutex> lock(threads_mutex);
    auto it = std::remove_if(client_threads.begin(), client_threads.end(),
        [](std::thread& t) {
            if (t.joinable()) {
                return false; // Keep joinable threads
            }
            return true; // Remove non-joinable threads
        });
    client_threads.erase(it, client_threads.end());
}

size_t TCPServer::getActiveConnections() {
    std::lock_guard<std::mutex> lock(threads_mutex);
    return client_threads.size();
}

void TCPServer::handleClient(int client_socket) {
    std::vector<char> buffer(BUFFER_SIZE);
    
    // Send Redis welcome message
    std::string welcome = "+OK Redis Mock Server Ready v1.0\r\n";
    if (send(client_socket, welcome.c_str(), welcome.length(), 0) < 0) {
        std::cerr << "Failed to send welcome message" << std::endl;
        close(client_socket);
        return;
    }
    
    while (running) {
        // Read data from client
        ssize_t bytes_read = read(client_socket, buffer.data(), buffer.size() - 1);
        
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::cout << "Redis client disconnected gracefully" << std::endl;
            } else if (running) {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
            }
            break;
        }

        buffer[bytes_read] = '\0';
        std::string received_data(buffer.data(), bytes_read);
        
        std::cout << "Received Redis command (" << bytes_read << " bytes): " 
                  << received_data.substr(0, std::min(size_t(50), received_data.length())) 
                  << (received_data.length() > 50 ? "..." : "") << std::endl;
        
        try {
            // Process the Redis command
            std::vector<std::string> args = parseRESP(received_data);
            
            if (args.empty()) {
                std::string error = "-ERR Protocol error: empty command\r\n";
                send(client_socket, error.c_str(), error.length(), 0);
                continue;
            }
            
            std::string response = processRedisCommand(args);
            
            if (!response.empty()) {
                if (send(client_socket, response.c_str(), response.length(), 0) < 0) {
                    std::cerr << "Send failed: " << strerror(errno) << std::endl;
                    break;
                }
                
                std::cout << "Sent response (" << response.length() << " bytes): " 
                          << response.substr(0, std::min(size_t(50), response.length()))
                          << (response.length() > 50 ? "..." : "") << std::endl;
                
                // Check for QUIT command to close connection
                if (!args.empty()) {
                    std::string cmd = args[0];
                    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
                    if (cmd == "QUIT") {
                        break;
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing command: " << e.what() << std::endl;
            std::string error = "-ERR " + std::string(e.what()) + "\r\n";
            send(client_socket, error.c_str(), error.length(), 0);
        }
    }

    close(client_socket);
    std::cout << "Client connection closed. Active connections: " << getActiveConnections() - 1 << std::endl;
}

std::string TCPServer::processRedisCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "-ERR Protocol error: empty command\r\n";
    }
    
    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    // Command routing
    if (cmd == "PING") return handlePing(args);
    else if (cmd == "ECHO") return handleEcho(args);
    else if (cmd == "SET") return handleSet(args);
    else if (cmd == "GET") return handleGet(args);
    else if (cmd == "DEL") return handleDel(args);
    else if (cmd == "EXISTS") return handleExists(args);
    else if (cmd == "KEYS") return handleKeys(args);
    else if (cmd == "INFO") return handleInfo(args);
    else if (cmd == "COMMAND") return "*0\r\n"; // Empty array for COMMAND
    else if (cmd == "QUIT") return "+OK\r\n";
    else {
        return "-ERR unknown command '" + args[0] + "', with args beginning with:\r\n";
    }
}

std::string TCPServer::handlePing(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        return "+PONG\r\n";
    } else if (args.size() == 2) {
        const std::string& message = args[1];
        return "$" + std::to_string(message.length()) + "\r\n" + message + "\r\n";
    } else {
        return "-ERR wrong number of arguments for 'ping' command\r\n";
    }
}

std::string TCPServer::handleEcho(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for 'echo' command\r\n";
    }
    
    // Concatenate all arguments except the command
    std::string message;
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) message += " ";
        message += args[i];
    }
    
    return "$" + std::to_string(message.length()) + "\r\n" + message + "\r\n";
}

std::string TCPServer::handleSet(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return "-ERR wrong number of arguments for 'set' command\r\n";
    }
    
    const std::string& key = args[1];
    const std::string& value = args[2];
    
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        redis_store[key] = value;
    }
    
    std::cout << "SET: " << key << " = " << value << " (Store size: " << redis_store.size() << ")" << std::endl;
    return "+OK\r\n";
}

std::string TCPServer::handleGet(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return "-ERR wrong number of arguments for 'get' command\r\n";
    }
    
    const std::string& key = args[1];
    
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        auto it = redis_store.find(key);
        if (it != redis_store.end()) {
            const std::string& value = it->second;
            return "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
        }
    }
    
    return "$-1\r\n"; // NULL bulk string
}

std::string TCPServer::handleDel(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for 'del' command\r\n";
    }
    
    int deleted_count = 0;
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        for (size_t i = 1; i < args.size(); ++i) {
            if (redis_store.erase(args[i]) > 0) {
                deleted_count++;
            }
        }
    }
    
    std::cout << "DEL: Deleted " << deleted_count << " keys (Store size: " << redis_store.size() << ")" << std::endl;
    return ":" + std::to_string(deleted_count) + "\r\n";
}

std::string TCPServer::handleExists(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return "-ERR wrong number of arguments for 'exists' command\r\n";
    }
    
    int exists_count = 0;
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        for (size_t i = 1; i < args.size(); ++i) {
            if (redis_store.find(args[i]) != redis_store.end()) {
                exists_count++;
            }
        }
    }
    
    return ":" + std::to_string(exists_count) + "\r\n";
}

std::string TCPServer::handleKeys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return "-ERR wrong number of arguments for 'keys' command\r\n";
    }
    
    const std::string& pattern = args[1];
    std::vector<std::string> matching_keys;
    
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        for (const auto& pair : redis_store) {
            // Simple pattern matching (only supports * for now)
            if (pattern == "*" || pair.first.find(pattern) != std::string::npos) {
                matching_keys.push_back(pair.first);
            }
        }
    }
    
    std::string response = "*" + std::to_string(matching_keys.size()) + "\r\n";
    for (const auto& key : matching_keys) {
        response += "$" + std::to_string(key.length()) + "\r\n" + key + "\r\n";
    }
    
    return response;
}

std::string TCPServer::handleInfo(const std::vector<std::string>& args) {
    std::string info = "# Server\r\n";
    info += "redis_version:7.0.0-mock\r\n";
    info += "redis_mode:standalone\r\n";
    info += "process_id:1234\r\n";
    info += "tcp_port:" + std::to_string(port) + "\r\n";
    info += "\r\n";
    info += "# Clients\r\n";
    info += "connected_clients:" + std::to_string(getActiveConnections()) + "\r\n";
    info += "\r\n";
    info += "# Memory\r\n";
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        info += "used_memory_dataset:" + std::to_string(redis_store.size() * 64) + "\r\n"; // Rough estimate
    }
    info += "\r\n";
    info += "# Keyspace\r\n";
    {
        std::lock_guard<std::mutex> lock(store_mutex);
        if (!redis_store.empty()) {
            info += "db0:keys=" + std::to_string(redis_store.size()) + ",expires=0,avg_ttl=0\r\n";
        }
    }
    
    return "$" + std::to_string(info.length()) + "\r\n" + info + "\r\n";
}

std::vector<std::string> TCPServer::parseRESP(const std::string& input) {
    std::vector<std::string> result;
    
    if (input.empty()) {
        throw std::invalid_argument("Empty input");
    }
    
    if (input[0] != '*') {
        // Fallback for plain text commands (telnet compatibility)
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            // Remove \r\n characters
            token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
            token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
            if (!token.empty()) {
                result.push_back(token);
            }
        }
        return result;
    }

    std::istringstream ss(input);
    std::string line;
    
    // Read first line *<num>
    if (!std::getline(ss, line)) {
        throw std::invalid_argument("Invalid RESP format: missing array header");
    }
    
    if (line.empty() || line[0] != '*') {
        throw std::invalid_argument("Invalid RESP format: expected array");
    }
    
    // Remove \r if present
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    
    // Extract number of arguments
    int num_args;
    try {
        num_args = std::stoi(line.substr(1));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid RESP format: invalid array size");
    }
    
    if (num_args < 0) {
        throw std::invalid_argument("Invalid RESP format: negative array size");
    }
    
    if (num_args > 100) { // Reasonable limit
        throw std::invalid_argument("Invalid RESP format: too many arguments");
    }
    
    // Read each argument
    for (int i = 0; i < num_args; ++i) {
        // Read line $<len>
        if (!std::getline(ss, line)) {
            throw std::invalid_argument("Invalid RESP format: missing bulk string header");
        }
        
        if (line.empty() || line[0] != '$') {
            throw std::invalid_argument("Invalid RESP format: expected bulk string");
        }
        
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        int len;
        try {
            len = std::stoi(line.substr(1));
        } catch (const std::exception& e) {
            throw std::invalid_argument("Invalid RESP format: invalid bulk string length");
        }
        
        if (len < 0) {
            result.push_back(""); // NULL string represented as empty
            continue;
        }
        
        if (len > static_cast<int>(MAX_STRING_LENGTH)) {
            throw std::invalid_argument("Invalid RESP format: string too long");
        }
        
        // Read the argument content
        if (!std::getline(ss, line)) {
            throw std::invalid_argument("Invalid RESP format: missing bulk string content");
        }
        
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // Validate length
        if (static_cast<int>(line.length()) != len) {
            std::cerr << "Warning: Expected length " << len << " but got " << line.length() << std::endl;
        }
        
        result.push_back(line);
    }
    
    return result;
}