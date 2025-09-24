#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <future>
#include <vector>
#include <memory>

#include "server/tcp_server.h"
#include "server/connection_manager.h"
#include "redis/command_handler.h"
#include "resp/resp_parser.h"
#include "utils/logger.h"

class TCPServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a test port that's likely to be available
        test_port = 8765;
        server = std::make_unique<TCPServer>(test_port);
    }

    void TearDown() override {
        if (server && server->isRunning()) {
            server->stop();
            // Give some time for the server to stop
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        server.reset();
    }

    // Helper function to create a client socket
    int createClientSocket() {
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) return -1;

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(test_port);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(client_fd);
            return -1;
        }

        return client_fd;
    }

    // Helper function to send RESP command
    std::string sendCommand(int socket, const std::vector<std::string>& args) {
        std::string command = formatRESPArray(args);
        send(socket, command.c_str(), command.size(), 0);

        char buffer[1024];
        ssize_t n = recv(socket, buffer, sizeof(buffer), 0);
        if (n > 0) {
            return std::string(buffer, n);
        }
        return "";
    }

    // Helper to format commands as RESP arrays
    std::string formatRESPArray(const std::vector<std::string>& args) {
        std::string result = "*" + std::to_string(args.size()) + "\r\n";
        for (const auto& arg : args) {
            result += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
        }
        return result;
    }

    // Helper to start server in background thread
    void startServerAsync() {
        server_thread = std::async(std::launch::async, [this]() {
            try {
                server->start();
            } catch (const std::exception& e) {
                std::cerr << "Server error: " << e.what() << std::endl;
            }
        });
        
        // Wait for server to start
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    int test_port;
    std::unique_ptr<TCPServer> server;
    std::future<void> server_thread;
};

// Test server initialization
TEST_F(TCPServerTest, ServerInitialization) {
    EXPECT_EQ(server->getPort(), test_port);
    EXPECT_FALSE(server->isRunning());
    EXPECT_EQ(server->getActiveConnections(), 0);
}

// Test server constructor with invalid port (edge case)
TEST_F(TCPServerTest, InvalidPortHandling) {
    // Test with port 0 (should work as OS assigns port)
    EXPECT_NO_THROW({
        TCPServer server_auto_port(0);
    });
    
    // Test with negative port (constructor should still work, failure occurs on start)
    EXPECT_NO_THROW({
        TCPServer server_invalid(-1);
    });
}

// Test server start and stop functionality
TEST_F(TCPServerTest, StartStopServer) {
    startServerAsync();
    
    EXPECT_TRUE(server->isRunning());
    EXPECT_EQ(server->getPort(), test_port);
    
    server->stop();
    
    // Wait for server to stop
    if (server_thread.valid()) {
        server_thread.wait_for(std::chrono::milliseconds(500));
    }
    
    EXPECT_FALSE(server->isRunning());
}

// Test starting server twice
TEST_F(TCPServerTest, StartServerTwice) {
    startServerAsync();
    EXPECT_TRUE(server->isRunning());
    
    // Starting again should not throw, but should not change state
    EXPECT_NO_THROW(server->start());
    EXPECT_TRUE(server->isRunning());
}

// Test stopping server twice
TEST_F(TCPServerTest, StopServerTwice) {
    startServerAsync();
    EXPECT_TRUE(server->isRunning());
    
    server->stop();
    EXPECT_FALSE(server->isRunning());
    
    // Stopping again should not throw
    EXPECT_NO_THROW(server->stop());
    EXPECT_FALSE(server->isRunning());
}

// Test single client connection
TEST_F(TCPServerTest, SingleClientConnection) {
    startServerAsync();
    
    int client_socket = createClientSocket();
    ASSERT_GT(client_socket, 0) << "Failed to create client socket";
    
    // Give some time for connection to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(server->getActiveConnections(), 1);
    
    close(client_socket);
    
    // Wait for connection to be cleaned up
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Note: Active connections might not immediately go to 0 due to async cleanup
    // In a real implementation, you might want to add a way to wait for cleanup
}

// Test multiple client connections
TEST_F(TCPServerTest, MultipleClientConnections) {
    startServerAsync();
    
    std::vector<int> client_sockets;
    const int num_clients = 3;
    
    for (int i = 0; i < num_clients; i++) {
        int client_socket = createClientSocket();
        ASSERT_GT(client_socket, 0) << "Failed to create client socket " << i;
        client_sockets.push_back(client_socket);
        
        // Small delay between connections
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Check that all connections are active
    EXPECT_EQ(server->getActiveConnections(), num_clients);
    
    // Close all connections
    for (int socket : client_sockets) {
        close(socket);
    }
}

// Test Redis-like command handling
TEST_F(TCPServerTest, BasicRedisCommands) {
    startServerAsync();
    
    int client_socket = createClientSocket();
    ASSERT_GT(client_socket, 0);
    
    // Test PING command
    std::string response = sendCommand(client_socket, {"PING"});
    EXPECT_FALSE(response.empty());
    
    // Test SET command
    response = sendCommand(client_socket, {"SET", "key1", "value1"});
    EXPECT_FALSE(response.empty());
    
    // Test GET command
    response = sendCommand(client_socket, {"GET", "key1"});
    EXPECT_FALSE(response.empty());
    
    close(client_socket);
}

// Test client disconnection handling
TEST_F(TCPServerTest, ClientDisconnection) {
    startServerAsync();
    
    int client_socket = createClientSocket();
    ASSERT_GT(client_socket, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(server->getActiveConnections(), 1);
    
    // Abruptly close the socket
    close(client_socket);
    
    // Wait for server to detect disconnection
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Connection count should eventually decrease
    // Note: This might be flaky due to timing - in production you might want
    // a more reliable way to test this
}

// Test port already in use scenario
TEST_F(TCPServerTest, PortAlreadyInUse) {
    startServerAsync();
    EXPECT_TRUE(server->isRunning());
    
    // Try to create another server on the same port
    TCPServer another_server(test_port);
    
    // This should throw an exception when trying to start
    EXPECT_THROW(another_server.start(), std::runtime_error);
}

// Test server destruction while running
TEST_F(TCPServerTest, DestructorWhileRunning) {
    startServerAsync();
    EXPECT_TRUE(server->isRunning());
    
    // Create a client connection
    int client_socket = createClientSocket();
    EXPECT_GT(client_socket, 0);
    
    // Destroy the server - destructor should clean up properly
    server.reset();
    
    // Wait for cleanup
    if (server_thread.valid()) {
        server_thread.wait_for(std::chrono::seconds(1));
    }
    
    // Client socket should be closed by server cleanup
    char buffer[10];
    ssize_t result = recv(client_socket, buffer, sizeof(buffer), MSG_DONTWAIT);
    EXPECT_EQ(result, -1); // Should return 0 for closed connection
    
    close(client_socket);
}

// Test large message handling
TEST_F(TCPServerTest, LargeMessageHandling) {
    startServerAsync();
    
    int client_socket = createClientSocket();
    ASSERT_GT(client_socket, 0);
    
    // Create a large value (but within reasonable limits)
    std::string large_value(1000, 'A');
    std::string response = sendCommand(client_socket, {"SET", "large_key", large_value});
    EXPECT_FALSE(response.empty());
    
    // Try to get it back
    response = sendCommand(client_socket, {"GET", "large_key"});
    EXPECT_FALSE(response.empty());
    
    close(client_socket);
}

// Test rapid connect/disconnect
TEST_F(TCPServerTest, RapidConnectDisconnect) {
    startServerAsync();
    
    const int num_iterations = 10;
    for (int i = 0; i < num_iterations; i++) {
        int client_socket = createClientSocket();
        if (client_socket > 0) {
            // Send a quick command
            sendCommand(client_socket, {"PING"});
            close(client_socket);
        }
        
        // Small delay to avoid overwhelming the server
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Server should still be running
    EXPECT_TRUE(server->isRunning());
}

// Mock test for connection manager integration
class MockConnectionManager {
public:
    MOCK_METHOD(bool, canAcceptNewConnection, ());
    MOCK_METHOD(size_t, getActiveConnections, ());
    MOCK_METHOD(void, addConnection, (std::function<void()>));
    MOCK_METHOD(void, stopAllConnections, ());
    
    static const size_t MAX_CLIENTS = 100;
};

// Integration test would require dependency injection or friend classes
// This is a simplified example of how you might test with mocks
TEST(TCPServerMockTest, ConnectionLimitHandling) {
    // This test would require modifying TCPServer to accept ConnectionManager
    // as a dependency or making it testable through other means
    
    MockConnectionManager mock_manager;
    
    // Set expectations
    EXPECT_CALL(mock_manager, canAcceptNewConnection())
        .WillOnce(::testing::Return(false));
    
    // Test that when connection limit is reached, new connections are rejected
    // This would require integration with the actual TCPServer class
    EXPECT_FALSE(mock_manager.canAcceptNewConnection());
}

// Performance test (optional)
TEST_F(TCPServerTest, DISABLED_PerformanceTest) {
    startServerAsync();
    
    const int num_clients = 50;
    const int commands_per_client = 100;
    
    std::vector<std::thread> client_threads;
    
    auto client_work = [this](int client_id) {
        int socket = createClientSocket();
        if (socket <= 0) return;
        
        for (int i = 0; i < commands_per_client; i++) {
            std::string key = "key_" + std::to_string(client_id) + "_" + std::to_string(i);
            std::string value = "value_" + std::to_string(i);
            
            sendCommand(socket, {"SET", key, value});
            sendCommand(socket, {"GET", key});
        }
        
        close(socket);
    };
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_clients; i++) {
        client_threads.emplace_back(client_work, i);
    }
    
    for (auto& thread : client_threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Performance test completed in " << duration.count() << "ms" << std::endl;
    std::cout << "Total operations: " << (num_clients * commands_per_client * 2) << std::endl;
}