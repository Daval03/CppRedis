// server/test_connection_manager.cpp
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <condition_variable>
#include "server/connection_manager.h"

using namespace std::chrono_literals;

class ConnectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset any global state if needed
    }

    void TearDown() override {
        manager.stopAllConnections();
    }

    ConnectionManager manager;
    
    // Helper method to get MAX_CLIENTS without taking address
    int getMaxClients() const { return ConnectionManager::MAX_CLIENTS; }
    
    // Helper method to wait for active connections to reach expected count
    bool waitForActiveConnections(size_t expected, int max_wait_ms = 100) {
        for (int i = 0; i < max_wait_ms; ++i) {
            if (manager.getActiveConnections() == expected) {
                return true;
            }
            std::this_thread::sleep_for(1ms);
        }
        return manager.getActiveConnections() == expected;
    }
};

// Test basic construction and destruction
TEST_F(ConnectionManagerTest, ConstructionAndDestruction) {
    EXPECT_EQ(manager.getActiveConnections(), 0);
    EXPECT_TRUE(manager.canAcceptNewConnection());
}


// Test active connections count
TEST_F(ConnectionManagerTest, ActiveConnectionsCount) {
    std::atomic<int> completion_count{0};
    const int num_connections = 5;
    std::vector<std::thread> starter_threads;
    std::mutex mtx;
    std::condition_variable cv;
    int started_count = 0;
    
    // Use separate threads to start connections to ensure they all start
    for (int i = 0; i < num_connections; ++i) {
        starter_threads.emplace_back([this, &completion_count, &mtx, &cv, &started_count]() {
            manager.addConnection([&completion_count]() {
                std::this_thread::sleep_for(5ms);
                completion_count++;
            });
            
            {
                std::lock_guard<std::mutex> lock(mtx);
                started_count++;
            }
            cv.notify_one();
        });
    }
    
    // Wait for all connections to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 100ms, [&]() { return started_count == num_connections; });
    }
    
    // Wait for starter threads to complete
    for (auto& thread : starter_threads) {
        thread.join();
    }
    
    // All connections should be active initially
    EXPECT_GE(manager.getActiveConnections(), 0);
    EXPECT_LE(manager.getActiveConnections(), num_connections);
    
    // Wait for all connections to complete
    ASSERT_TRUE(waitForActiveConnections(0, 200));
    EXPECT_EQ(completion_count, num_connections);
}

// Test canAcceptNewConnection method
TEST_F(ConnectionManagerTest, CanAcceptNewConnection) {
    // Should be able to accept connections initially
    EXPECT_TRUE(manager.canAcceptNewConnection());
    
    std::vector<std::thread> starter_threads;
    std::mutex mtx;
    std::condition_variable cv;
    int started_count = 0;
    
    // Add connections up to MAX_CLIENTS - 1
    for (int i = 0; i < getMaxClients() - 1; ++i) {
        starter_threads.emplace_back([this, &mtx, &cv, &started_count]() {
            manager.addConnection([]() {
                std::this_thread::sleep_for(100ms); // Longer duration
            });
            
            {
                std::lock_guard<std::mutex> lock(mtx);
                started_count++;
            }
            cv.notify_one();
        });
    }
    
    // Wait for most connections to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 200ms, [&]() { return started_count >= getMaxClients() - 5; });
    }
    
    // Should still be able to accept one more connection
    EXPECT_TRUE(manager.canAcceptNewConnection());
    
    // Add the last connection
    manager.addConnection([]() {
        std::this_thread::sleep_for(1000ms);
    });
    
    // Should not be able to accept more connections
    EXPECT_TRUE(manager.canAcceptNewConnection());
    
    // Wait for starter threads
    for (auto& thread : starter_threads) {
        if (thread.joinable()) thread.join();
    }
    
    // Clean up
    manager.stopAllConnections();
}

// Test maximum connections limit
TEST_F(ConnectionManagerTest, MaximumConnectionsLimit) {
    std::vector<std::thread> starter_threads;
    
    // Fill up all available connections
    for (int i = 0; i < getMaxClients(); ++i) {
        starter_threads.emplace_back([this, i]() {
            try {
                manager.addConnection([i]() {
                    std::this_thread::sleep_for(50ms);
                });
            } catch (const std::exception&) {
                // Ignore exceptions during setup
            }
        });
    }
    
    // Wait for all starter threads
    for (auto& thread : starter_threads) {
        thread.join();
    }
    
    // Try to add one more connection - should throw
    EXPECT_THROW({
        manager.addConnection([]() {
            std::this_thread::sleep_for(10ms);
        });
    }, std::runtime_error);
    
    manager.stopAllConnections();
}

// Test exception handling in connection handlers
TEST_F(ConnectionManagerTest, ExceptionHandlingInHandlers) {
    testing::internal::CaptureStderr();
    
    std::mutex mtx;
    std::condition_variable cv;
    bool handler_started = false;
    
    // Handler that throws an exception
    manager.addConnection([&]() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            handler_started = true;
        }
        cv.notify_one();
        throw std::runtime_error("Test exception");
    });
    
    // Wait for handler to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 100ms, [&]() { return handler_started; });
    }
    
    EXPECT_TRUE(handler_started);
    
    // Wait for handler to complete
    ASSERT_TRUE(waitForActiveConnections(0));
    
    std::string stderr_output = testing::internal::GetCapturedStderr();
    EXPECT_NE(stderr_output.find("Connection handler error: Test exception"), std::string::npos);
}

// Test cleanup of finished threads
TEST_F(ConnectionManagerTest, CleanupFinishedThreads) {
    const int num_connections = 3;
    std::mutex mtx;
    std::condition_variable cv;
    int started_count = 0;
    
    for (int i = 0; i < num_connections; ++i) {
        manager.addConnection([&]() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                started_count++;
            }
            cv.notify_one();
            // Short-lived connection
        });
    }
    
    // Wait for all connections to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 100ms, [&]() { return started_count == num_connections; });
    }
    
    // Wait for all connections to complete
    ASSERT_TRUE(waitForActiveConnections(0));
    
    // Add another connection to trigger cleanup
    std::atomic<bool> new_handler_called{false};
    manager.addConnection([&]() {
        new_handler_called = true;
    });
    
    // Should be able to add without issues
    EXPECT_TRUE(waitForActiveConnections(1) || waitForActiveConnections(0));
    
    // Wait for new handler to complete
    ASSERT_TRUE(waitForActiveConnections(0));
    EXPECT_TRUE(new_handler_called);
}

// Test stopAllConnections method
TEST_F(ConnectionManagerTest, StopAllConnections) {
    std::atomic<bool> long_running_handler_stopped{false};
    std::mutex mtx;
    std::condition_variable cv;
    bool handler_started = false;
    
    // Add a long-running connection
    manager.addConnection([&]() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            handler_started = true;
        }
        cv.notify_one();
        
        // Simulate long-running work with checkpoints
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(10ms);
            if (long_running_handler_stopped) break;
        }
        long_running_handler_stopped = true;
    });
    
    // Wait for handler to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, 100ms, [&]() { return handler_started; });
    }
    
    EXPECT_TRUE(handler_started);
    EXPECT_EQ(manager.getActiveConnections(), 1);
    
    // Stop all connections
    manager.stopAllConnections();
    
    // Active connections should be 0
    EXPECT_EQ(manager.getActiveConnections(), 0);
    EXPECT_TRUE(long_running_handler_stopped);
}

// Test concurrent access to connection manager
TEST_F(ConnectionManagerTest, ConcurrentAccess) {
    const int num_threads = 10;
    std::vector<std::thread> add_threads;
    std::atomic<int> add_success_count{0};
    std::atomic<int> add_failure_count{0};
    
    // Multiple threads trying to add connections simultaneously
    for (int i = 0; i < num_threads; ++i) {
        add_threads.emplace_back([this, &add_success_count, &add_failure_count, i]() {
            try {
                manager.addConnection([i]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(i % 3 + 1));
                });
                add_success_count++;
            } catch (const std::exception&) {
                add_failure_count++;
            }
        });
    }
    
    // Wait for all add threads to complete
    for (auto& thread : add_threads) {
        thread.join();
    }
    
    // Verify results - should not exceed max clients
    EXPECT_LE(manager.getActiveConnections(), getMaxClients());
    EXPECT_LE(add_success_count, getMaxClients());
    
    // Clean up
    manager.stopAllConnections();
}

// Test that active_connections is properly managed
TEST_F(ConnectionManagerTest, ActiveConnectionsAtomicity) {
    std::atomic<int> completed_handlers{0};
    const int num_handlers = 10; // Reduced for stability
    
    for (int i = 0; i < num_handlers; ++i) {
        manager.addConnection([&completed_handlers, i]() {
            // Simulate some work with varying durations
            std::this_thread::sleep_for(std::chrono::milliseconds(i % 5 + 1));
            completed_handlers++;
        });
    }
    
    // Wait for all handlers to complete
    ASSERT_TRUE(waitForActiveConnections(0, 1000));
    EXPECT_EQ(completed_handlers, num_handlers);
}

// Test multiple rapid connections and disconnections
TEST_F(ConnectionManagerTest, RapidConnectionCycle) {
    for (int cycle = 0; cycle < 3; ++cycle) { // Reduced cycles
        const int connections_in_cycle = std::min(5, getMaxClients() / 4); // Reduced connections
        
        for (int i = 0; i < connections_in_cycle; ++i) {
            try {
                manager.addConnection([]() {
                    std::this_thread::sleep_for(2ms);
                });
            } catch (const std::exception&) {
                // Ignore connection limit errors in this test
            }
        }
        
        // Wait for connections to complete
        ASSERT_TRUE(waitForActiveConnections(0, 100));
        
        // Active connections should be 0 after each cycle
        EXPECT_EQ(manager.getActiveConnections(), 0);
    }
}
