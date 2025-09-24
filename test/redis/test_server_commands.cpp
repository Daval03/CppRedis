// test_server_commands.cpp
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include "redis/commands/server_commands.h"
#include "redis/database/redis_database.h"

// Test fixture for ServerCommands tests
class ServerCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple in-memory database for testing
        database = new RedisDatabase();
        commands_processed = 0;
        server_start_time = std::chrono::system_clock::now();
        
        // Create ServerCommands instance
        serverCommands = new ServerCommands(*database, server_start_time, commands_processed);
        
        // Add some test data to the database
        database->setValue("key1", RedisValue("value1"));
        database->setValue("key2", RedisValue("value2"));
        database->setValue("key3", RedisValue("value3"));
        
    }
    
    void TearDown() override {
        delete serverCommands;
        delete database;
    }
    
    RedisDatabase* database;
    ServerCommands* serverCommands;
    std::chrono::system_clock::time_point server_start_time;
    size_t commands_processed;
};

// Test PING command with no arguments
TEST_F(ServerCommandsTest, Ping_NoArguments_ReturnsPong) {
    std::vector<std::string> args = {"PING"};
    std::string result = serverCommands->cmdPing(args);
    
    EXPECT_EQ("+PONG\r\n", result);
    commands_processed++; // Simulate command processing
}

// Test PING command with message argument
TEST_F(ServerCommandsTest, Ping_WithMessage_ReturnsMessage) {
    std::vector<std::string> args = {"PING", "Hello World"};
    std::string result = serverCommands->cmdPing(args);
    
    EXPECT_EQ("$11\r\nHello World\r\n", result);
    commands_processed++; // Simulate command processing
}

// Test PING command with too many arguments
TEST_F(ServerCommandsTest, Ping_TooManyArguments_ReturnsError) {
    std::vector<std::string> args = {"PING", "arg1", "arg2", "arg3"};
    std::string result = serverCommands->cmdPing(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("ping") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test ECHO command with valid argument
TEST_F(ServerCommandsTest, Echo_ValidArgument_ReturnsEchoedString) {
    std::vector<std::string> args = {"ECHO", "Hello Redis"};
    std::string result = serverCommands->cmdEcho(args);
    
    EXPECT_EQ("$11\r\nHello Redis\r\n", result);
    commands_processed++; // Simulate command processing
}

// Test ECHO command with wrong number of arguments
TEST_F(ServerCommandsTest, Echo_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"ECHO"}; // Missing message
    std::string result = serverCommands->cmdEcho(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("echo") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test ECHO command with too many arguments
TEST_F(ServerCommandsTest, Echo_TooManyArguments_ReturnsError) {
    std::vector<std::string> args = {"ECHO", "arg1", "arg2"};
    std::string result = serverCommands->cmdEcho(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test INFO command
TEST_F(ServerCommandsTest, Info_ReturnsServerInformation) {
    std::vector<std::string> args = {"INFO"};
    std::string result = serverCommands->cmdInfo(args);
    
    // Check that the response contains expected sections
    EXPECT_TRUE(result.find("# Server") != std::string::npos);
    EXPECT_TRUE(result.find("redis_version:7.0.0") != std::string::npos);
    EXPECT_TRUE(result.find("uptime_in_seconds:") != std::string::npos);
    EXPECT_TRUE(result.find("# Stats") != std::string::npos);
    EXPECT_TRUE(result.find("total_commands_processed:") != std::string::npos);
    EXPECT_TRUE(result.find("# Keyspace") != std::string::npos);
    EXPECT_TRUE(result.find("db0:keys=") != std::string::npos);
    
    // Verify it's a bulk string response
    EXPECT_EQ('$', result[0]);
    commands_processed++; // Simulate command processing
}

// Test INFO command with arguments (should ignore them)
TEST_F(ServerCommandsTest, Info_WithArguments_IgnoresArguments) {
    std::vector<std::string> args = {"INFO", "server", "stats"};
    std::string result = serverCommands->cmdInfo(args);
    
    // Should still return full info response
    EXPECT_TRUE(result.find("# Server") != std::string::npos);
    EXPECT_TRUE(result.find("# Stats") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test FLUSHALL command
TEST_F(ServerCommandsTest, Flushall_ClearsDatabase) {
    // Verify database has data before flush
    EXPECT_GT(database->getDatabaseSize(), 0);
    
    std::vector<std::string> args = {"FLUSHALL"};
    std::string result = serverCommands->cmdFlushall(args);
    
    EXPECT_EQ("+OK\r\n", result);
    
    // Verify database is empty after flush
    database->cleanupExpiredKeys();
    EXPECT_EQ(0, database->getDatabaseSize());
    commands_processed++; // Simulate command processing
}

// Test FLUSHALL command with too many arguments
TEST_F(ServerCommandsTest, Flushall_TooManyArguments_ReturnsError) {
    std::vector<std::string> args = {"FLUSHALL", "arg1", "arg2"};
    std::string result = serverCommands->cmdFlushall(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("flushall") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test KEYS command with pattern matching all keys
TEST_F(ServerCommandsTest, Keys_MatchAll_ReturnsAllKeys) {
    std::vector<std::string> args = {"KEYS", "*"};
    std::string result = serverCommands->cmdKeys(args);
    
    // Should return an array containing all keys
    EXPECT_EQ('*', result[0]); // Array response
    EXPECT_TRUE(result.find("key1") != std::string::npos);
    EXPECT_TRUE(result.find("key2") != std::string::npos);
    EXPECT_TRUE(result.find("key3") != std::string::npos);
    EXPECT_TRUE(result.find("expiring_key") == std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test KEYS command with specific pattern
TEST_F(ServerCommandsTest, Keys_SpecificPattern_ReturnsMatchingKeys) {
    std::vector<std::string> args = {"KEYS", "key*"};
    std::string result = serverCommands->cmdKeys(args);
    
    // Should return keys starting with "key"
    EXPECT_EQ('*', result[0]); // Array response
    EXPECT_TRUE(result.find("key1") != std::string::npos);
    EXPECT_TRUE(result.find("key2") != std::string::npos);
    EXPECT_TRUE(result.find("key3") != std::string::npos);
    EXPECT_TRUE(result.find("expiring_key") == std::string::npos); // Should not match
    commands_processed++; // Simulate command processing
}

// Test KEYS command with no matches
TEST_F(ServerCommandsTest, Keys_NoMatches_ReturnsEmptyArray) {
    std::vector<std::string> args = {"KEYS", "nonexistent*"};
    std::string result = serverCommands->cmdKeys(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
    commands_processed++; // Simulate command processing
}

// Test KEYS command with wrong number of arguments
TEST_F(ServerCommandsTest, Keys_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"KEYS"}; // Missing pattern
    std::string result = serverCommands->cmdKeys(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("keys") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test KEYS command with too many arguments
TEST_F(ServerCommandsTest, Keys_TooManyArguments_ReturnsError) {
    std::vector<std::string> args = {"KEYS", "*", "extra_arg"};
    std::string result = serverCommands->cmdKeys(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test DBSIZE command
TEST_F(ServerCommandsTest, Dbsize_ReturnsCorrectSize) {
    std::vector<std::string> args = {"DBSIZE"};
    std::string result = serverCommands->cmdDbsize(args);
    
    // Should return the number of keys in the database
    size_t expected_size = database->getDatabaseSize();
    std::string expected_response = ":" + std::to_string(expected_size) + "\r\n";
    EXPECT_EQ(expected_response, result);
    commands_processed++; // Simulate command processing
}

// Test DBSIZE command after adding a key
TEST_F(ServerCommandsTest, Dbsize_AfterAddingKey_ReturnsUpdatedSize) {
    // Add a new key
    database->setValue("new_key", RedisValue("new_value"));
    
    std::vector<std::string> args = {"DBSIZE"};
    std::string result = serverCommands->cmdDbsize(args);
    
    size_t expected_size = database->getDatabaseSize();
    std::string expected_response = ":" + std::to_string(expected_size) + "\r\n";
    EXPECT_EQ(expected_response, result);
    commands_processed++; // Simulate command processing
}

// Test DBSIZE command after FLUSHALL
TEST_F(ServerCommandsTest, Dbsize_AfterFlushall_ReturnsZero) {
    // First flush the database
    std::vector<std::string> flush_args = {"FLUSHALL"};
    serverCommands->cmdFlushall(flush_args);
    
    std::vector<std::string> args = {"DBSIZE"};
    std::string result = serverCommands->cmdDbsize(args);
    
    EXPECT_EQ(":0\r\n", result);
    commands_processed += 2; // Simulate both commands being processed
}

// Test DBSIZE command with wrong number of arguments
TEST_F(ServerCommandsTest, Dbsize_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"DBSIZE", "extra_arg"};
    std::string result = serverCommands->cmdDbsize(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("dbsize") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test TIME command
TEST_F(ServerCommandsTest, Time_ReturnsCurrentTime) {
    std::vector<std::string> args = {"TIME"};
    std::string result = serverCommands->cmdTime(args);
    
    // Should return an array with two elements: seconds and microseconds
    EXPECT_EQ('*', result[0]); // Array response
    
    // Extract the time values from the response
    // Format: "*2\r\n$10\r\n1234567890\r\n$6\r\n123456\r\n"
    size_t first_dollar = result.find('$');
    size_t second_dollar = result.find('$', first_dollar + 1);
    
    EXPECT_NE(std::string::npos, first_dollar);
    EXPECT_NE(std::string::npos, second_dollar);
    
    // Verify both elements are present
    EXPECT_TRUE(result.find("\r\n") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test TIME command with wrong number of arguments
TEST_F(ServerCommandsTest, Time_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"TIME", "extra_arg"};
    std::string result = serverCommands->cmdTime(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("time") != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Integration test: Multiple server commands
TEST_F(ServerCommandsTest, Integration_MultipleCommands) {
    // Test sequence of commands
    std::vector<std::string> ping_args = {"PING"};
    std::string ping_result = serverCommands->cmdPing(ping_args);
    EXPECT_EQ("+PONG\r\n", ping_result);
    
    std::vector<std::string> echo_args = {"ECHO", "Test Message"};
    std::string echo_result = serverCommands->cmdEcho(echo_args);
    EXPECT_EQ("$12\r\nTest Message\r\n", echo_result);
    
    std::vector<std::string> dbsize_args = {"DBSIZE"};
    std::string dbsize_result = serverCommands->cmdDbsize(dbsize_args);
    size_t initial_size = database->getDatabaseSize();
    std::string expected_dbsize = ":" + std::to_string(initial_size) + "\r\n";
    EXPECT_EQ(expected_dbsize, dbsize_result);
    
    std::vector<std::string> keys_args = {"KEYS", "key*"};
    std::string keys_result = serverCommands->cmdKeys(keys_args);
    EXPECT_EQ('*', keys_result[0]); // Array response
    
    std::vector<std::string> time_args = {"TIME"};
    std::string time_result = serverCommands->cmdTime(time_args);
    EXPECT_EQ('*', time_result[0]); // Array response
    
    commands_processed += 5; // Simulate all commands being processed
}

// Test INFO command reflects command processing count
TEST_F(ServerCommandsTest, Info_ReflectsCommandsProcessed) {
    // Process some commands first
    serverCommands->cmdPing({"PING"});
    serverCommands->cmdEcho({"ECHO", "test"});
    serverCommands->cmdDbsize({"DBSIZE"});
    commands_processed += 3;
    
    std::vector<std::string> args = {"INFO"};
    std::string result = serverCommands->cmdInfo(args);
    
    // Should show 3 commands processed
    EXPECT_TRUE(result.find("total_commands_processed:3") != std::string::npos);
    commands_processed++; // For the INFO command itself
}

// Test edge case: Empty database operations
TEST_F(ServerCommandsTest, EdgeCase_EmptyDatabase) {
    // Clear the database first
    serverCommands->cmdFlushall({"FLUSHALL"});
    
    // Test DBSIZE on empty database
    std::vector<std::string> dbsize_args = {"DBSIZE"};
    std::string dbsize_result = serverCommands->cmdDbsize(dbsize_args);
    EXPECT_EQ(":0\r\n", dbsize_result);
    
    // Test KEYS on empty database
    std::vector<std::string> keys_args = {"KEYS", "*"};
    std::string keys_result = serverCommands->cmdKeys(keys_args);
    EXPECT_EQ("*0\r\n", keys_result); // Empty array
    
    commands_processed += 3; // Simulate all commands being processed
}

// Test edge case: Very long echo message
TEST_F(ServerCommandsTest, EdgeCase_LongEchoMessage) {
    std::string long_message(1000, 'A'); // 1000 'A's
    std::vector<std::string> args = {"ECHO", long_message};
    std::string result = serverCommands->cmdEcho(args);
    
    // Should handle long messages correctly
    EXPECT_TRUE(result.find(long_message) != std::string::npos);
    commands_processed++; // Simulate command processing
}

// Test command argument case sensitivity (commands should be case-insensitive in real Redis)
// Note: This test assumes your implementation handles case sensitivity as needed
TEST_F(ServerCommandsTest, EdgeCase_CommandCaseVariations) {
    // These would typically be handled by the command dispatcher, not the command implementations
    // The command implementations expect the command name in uppercase as per the protocol
    
    std::vector<std::string> ping_args = {"PING"};
    std::string result = serverCommands->cmdPing(ping_args);
    EXPECT_EQ("+PONG\r\n", result);
    commands_processed++; // Simulate command processing
}