// test_ttl_commands.cpp
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include "redis/commands/ttl_commands.h"
#include "redis/database/redis_database.h"

// Test fixture for TTLCommands tests
class TTLCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple in-memory database for testing
        database = new RedisDatabase();
        ttlCommands = new TTLCommands(*database);
        
        // Add some test keys with different types
        database->setValue("string_key", RedisValue("test_value"));
        database->setValue("list_key", RedisValue(RedisType::LIST));
        database->setValue("no_expiry_key", RedisValue("no_expiry_value"));
        database->setValue("expiring_key", RedisValue("expiring_value"));
    }
    
    void TearDown() override {
        delete ttlCommands;
        delete database;
    }
    
    RedisDatabase* database;
    TTLCommands* ttlCommands;
};

// Test expire command with valid arguments
TEST_F(TTLCommandsTest, Expire_ValidArguments_ReturnsSuccess) {
    std::vector<std::string> args = {"EXPIRE", "string_key", "60"};
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_EQ(":1\r\n", result); // Should return 1 for success
    
    // Verify expiry was set
    RedisValue* value = database->getValue("string_key");
    EXPECT_TRUE(value != nullptr);
    if (value) {
        EXPECT_TRUE(value->has_expiry);
    }
}

// Test expire command with wrong number of arguments
TEST_F(TTLCommandsTest, Expire_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"EXPIRE", "string_key"}; // Missing seconds
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test expire command with non-existent key
TEST_F(TTLCommandsTest, Expire_NonExistentKey_ReturnsZero) {
    std::vector<std::string> args = {"EXPIRE", "non_existent_key", "60"};
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_EQ(":0\r\n", result); // Should return 0 for non-existent key
}

// Test expire command with invalid integer
TEST_F(TTLCommandsTest, Expire_InvalidInteger_ReturnsError) {
    std::vector<std::string> args = {"EXPIRE", "string_key", "not_a_number"};
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_TRUE(result.find("ERR value is not an integer") != std::string::npos);
}

// Test expire command with zero or negative seconds (should delete key)
TEST_F(TTLCommandsTest, Expire_ZeroSeconds_DeletesKeyAndReturnsOne) {
    std::vector<std::string> args = {"EXPIRE", "string_key", "0"};
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_EQ(":1\r\n", result);
    
    // Verify key was deleted
    RedisValue* value = database->getValue("string_key");
    EXPECT_EQ(nullptr, value);
}

// Test expire command with negative seconds (should delete key)
TEST_F(TTLCommandsTest, Expire_NegativeSeconds_DeletesKeyAndReturnsOne) {
    std::vector<std::string> args = {"EXPIRE", "list_key", "-1"};
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_EQ(":1\r\n", result);
    
    // Verify key was deleted
    RedisValue* value = database->getValue("list_key");
    EXPECT_EQ(nullptr, value);
}

// Test expireat command with valid arguments
TEST_F(TTLCommandsTest, Expireat_ValidArguments_ReturnsSuccess) {
    long long future_timestamp = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + std::chrono::hours(1));
    
    std::vector<std::string> args = {"EXPIREAT", "string_key", std::to_string(future_timestamp)};
    std::string result = ttlCommands->cmdExpireat(args);
    
    EXPECT_EQ(":1\r\n", result);
    
    // Verify expiry was set
    RedisValue* value = database->getValue("string_key");
    EXPECT_TRUE(value != nullptr);
    if (value) {
        EXPECT_TRUE(value->has_expiry);
    }
}

// Test expireat command with past timestamp (should delete key)
TEST_F(TTLCommandsTest, Expireat_PastTimestamp_DeletesKeyAndReturnsOne) {
    long long past_timestamp = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() - std::chrono::hours(1));
    
    std::vector<std::string> args = {"EXPIREAT", "string_key", std::to_string(past_timestamp)};
    std::string result = ttlCommands->cmdExpireat(args);
    
    EXPECT_EQ(":1\r\n", result);
    
    // Verify key was deleted
    RedisValue* value = database->getValue("string_key");
    EXPECT_EQ(nullptr, value);
}

// Test expireat command with non-existent key
TEST_F(TTLCommandsTest, Expireat_NonExistentKey_ReturnsZero) {
    long long future_timestamp = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + std::chrono::hours(1));
    
    std::vector<std::string> args = {"EXPIREAT", "non_existent_key", std::to_string(future_timestamp)};
    std::string result = ttlCommands->cmdExpireat(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test ttl command with non-existent key
TEST_F(TTLCommandsTest, Ttl_NonExistentKey_ReturnsMinusTwo) {
    std::vector<std::string> args = {"TTL", "non_existent_key"};
    std::string result = ttlCommands->cmdTtl(args);
    
    EXPECT_EQ(":-2\r\n", result);
}

// Test ttl command with key having no expiry
TEST_F(TTLCommandsTest, Ttl_KeyWithNoExpiry_ReturnsMinusOne) {
    std::vector<std::string> args = {"TTL", "no_expiry_key"};
    std::string result = ttlCommands->cmdTtl(args);
    
    EXPECT_EQ(":-1\r\n", result);
}

// Test ttl command with expired key
TEST_F(TTLCommandsTest, Ttl_ExpiredKey_ReturnsMinusTwoAndDeletesKey) {
    // Set a past expiry by directly manipulating the value
    RedisValue* value = database->getValue("expiring_key");
    if (value) {
        value->expiry = std::chrono::system_clock::now() - std::chrono::seconds(1);
        value->has_expiry = true;
    }
    
    std::vector<std::string> args = {"TTL", "expiring_key"};
    std::string result = ttlCommands->cmdTtl(args);
    
    EXPECT_EQ(":-2\r\n", result);
    
    // Verify key was deleted
    value = database->getValue("expiring_key");
    EXPECT_EQ(nullptr, value);
}

// Test ttl command with valid expiry
TEST_F(TTLCommandsTest, Ttl_KeyWithValidExpiry_ReturnsPositiveTTL) {
    // Set expiry to 60 seconds in the future
    RedisValue* value = database->getValue("string_key");
    if (value) {
        value->expiry = std::chrono::system_clock::now() + std::chrono::seconds(60);
        value->has_expiry = true;
    }
    
    std::vector<std::string> args = {"TTL", "string_key"};
    std::string result = ttlCommands->cmdTtl(args);
    
    // Should return a positive integer around 60
    // Since we can't exactly predict due to timing, we'll check the format
    EXPECT_EQ(':', result[0]); // Integer response format
    EXPECT_NE(":-2\r\n", result);
    EXPECT_NE(":-1\r\n", result);
    
    // Verify the TTL is approximately 60 seconds (within a reasonable range)
    // Extract integer from ":NN\r\n" format
    if (result.length() > 3) {
        try {
            int ttl = std::stoi(result.substr(1, result.length() - 3));
            EXPECT_GT(ttl, 55);
            EXPECT_LE(ttl, 60);
        } catch (const std::exception& e) {
            EXPECT_EQ(':', result[0]);
            EXPECT_NE(result.find("-1"), 1); // Not at position 1
            EXPECT_NE(result.find("-2"), 1); // Not at position 1
        }
    }
}

// Test persist command with non-existent key
TEST_F(TTLCommandsTest, Persist_NonExistentKey_ReturnsZero) {
    std::vector<std::string> args = {"PERSIST", "non_existent_key"};
    std::string result = ttlCommands->cmdPersist(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test persist command with key having no expiry
TEST_F(TTLCommandsTest, Persist_KeyWithNoExpiry_ReturnsZero) {
    std::vector<std::string> args = {"PERSIST", "no_expiry_key"};
    std::string result = ttlCommands->cmdPersist(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test persist command with key having expiry
TEST_F(TTLCommandsTest, Persist_KeyWithExpiry_ReturnsOneAndClearsExpiry) {
    // Set expiry first
    RedisValue* value = database->getValue("string_key");
    if (value) {
        value->expiry = std::chrono::system_clock::now() + std::chrono::seconds(60);
        value->has_expiry = true;
    }
    
    std::vector<std::string> args = {"PERSIST", "string_key"};
    std::string result = ttlCommands->cmdPersist(args);
    
    EXPECT_EQ(":1\r\n", result);
    
    // Verify expiry was cleared
    value = database->getValue("string_key");
    EXPECT_TRUE(value != nullptr);
    if (value) {
        EXPECT_FALSE(value->has_expiry);
    }
}

// Test persist command wrong number of arguments
TEST_F(TTLCommandsTest, Persist_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"PERSIST", "string_key", "extra_arg"};
    std::string result = ttlCommands->cmdPersist(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test expireat command wrong number of arguments
TEST_F(TTLCommandsTest, Expireat_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"EXPIREAT", "string_key"};
    std::string result = ttlCommands->cmdExpireat(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test ttl command wrong number of arguments
TEST_F(TTLCommandsTest, Ttl_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"TTL"};
    std::string result = ttlCommands->cmdTtl(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test with different data types
TEST_F(TTLCommandsTest, Expire_WithListType_WorksCorrectly) {
    std::vector<std::string> args = {"EXPIRE", "list_key", "30"};
    std::string result = ttlCommands->cmdExpire(args);
    
    EXPECT_EQ(":1\r\n", result);
    
    // Verify expiry was set
    RedisValue* value = database->getValue("list_key");
    EXPECT_TRUE(value != nullptr);
    if (value) {
        EXPECT_TRUE(value->has_expiry);
        EXPECT_EQ(RedisType::LIST, value->type);
    }
}

// Integration test: Set expiry and check TTL
TEST_F(TTLCommandsTest, Integration_SetExpireAndCheckTTL) {
    // Set expiry
    std::vector<std::string> expire_args = {"EXPIRE", "string_key", "120"};
    std::string expire_result = ttlCommands->cmdExpire(expire_args);
    EXPECT_EQ(":1\r\n", expire_result);
    
    // Check TTL
    std::vector<std::string> ttl_args = {"TTL", "string_key"};
    std::string ttl_result = ttlCommands->cmdTtl(ttl_args);
    
    // Should return a positive integer around 120
    EXPECT_EQ(':', ttl_result[0]);
    EXPECT_NE(":-2\r\n", ttl_result);
    EXPECT_NE(":-1\r\n", ttl_result);
    
    // Remove expiry with PERSIST
    std::vector<std::string> persist_args = {"PERSIST", "string_key"};
    std::string persist_result = ttlCommands->cmdPersist(persist_args);
    EXPECT_EQ(":1\r\n", persist_result);
    
    // Check TTL again - should be -1 (no expiry)
    ttl_result = ttlCommands->cmdTtl(ttl_args);
    EXPECT_EQ(":-1\r\n", ttl_result);
}

// Test isExpired method functionality
TEST_F(TTLCommandsTest, RedisValue_IsExpired_WorksCorrectly) {
    RedisValue value("test");
    
    // Test without expiry
    EXPECT_FALSE(value.isExpired());
    
    // Test with future expiry
    value.expiry = std::chrono::system_clock::now() + std::chrono::seconds(10);
    value.has_expiry = true;
    EXPECT_FALSE(value.isExpired());
    
    // Test with past expiry
    value.expiry = std::chrono::system_clock::now() - std::chrono::seconds(10);
    value.has_expiry = true;
    EXPECT_TRUE(value.isExpired());
}

// Test setExpiry method
TEST_F(TTLCommandsTest, RedisValue_SetExpiry_WorksCorrectly) {
    RedisValue value("test");
    
    value.setExpiry(std::chrono::seconds(30));
    
    EXPECT_TRUE(value.has_expiry);
    EXPECT_GT(value.expiry, std::chrono::system_clock::now());
}

// Test clearExpiry method
TEST_F(TTLCommandsTest, RedisValue_ClearExpiry_WorksCorrectly) {
    RedisValue value("test");
    
    value.setExpiry(std::chrono::seconds(30));
    EXPECT_TRUE(value.has_expiry);
    
    value.clearExpiry();
    EXPECT_FALSE(value.has_expiry);
}
