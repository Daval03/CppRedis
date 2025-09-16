#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>

class TCPServer {
private:
    int server_fd;
    int port;
    std::atomic<bool> running;
    std::vector<std::thread> client_threads;

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
};

#endif // TCP_SERVER_H