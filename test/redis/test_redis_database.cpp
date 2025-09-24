#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "redis/database/redis_database.h"

class RedisDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db.clearDatabase();
    }

    void TearDown() override {
        db.clearDatabase();
    }

    RedisDatabase db;
};

// Test basic key existence
TEST_F(RedisDatabaseTest, KeyExistsBasic) {
    RedisValue value("test_value");
    db.setValue("test_key", value);
    
    EXPECT_TRUE(db.keyExists("test_key"));
    EXPECT_FALSE(db.keyExists("nonexistent_key"));
}

// Test expired key handling
TEST_F(RedisDatabaseTest, KeyExistsExpired) {
    RedisValue value("test_value");
    value.setExpiry(std::chrono::milliseconds(1)); // Very short TTL
    db.setValue("test_key", value);
    
    // Wait for expiry
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_FALSE(db.keyExists("test_key"));
}

// Test getValue functionality
TEST_F(RedisDatabaseTest, GetValue) {
    RedisValue value("test_value");
    db.setValue("test_key", value);
    
    RedisValue* retrieved = db.getValue("test_key");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->string_value, "test_value");
    
    EXPECT_EQ(db.getValue("nonexistent_key"), nullptr);
}

// Test getValue with expired key
TEST_F(RedisDatabaseTest, GetValueExpired) {
    RedisValue value("test_value");
    value.setExpiry(std::chrono::milliseconds(1));
    db.setValue("test_key", value);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    EXPECT_EQ(db.getValue("test_key"), nullptr);
}

// Test setValue with different types
TEST_F(RedisDatabaseTest, SetValueDifferentTypes) {
    // String type
    RedisValue string_val("string_value");
    db.setValue("string_key", string_val);
    
    // List type
    RedisValue list_val(RedisType::LIST);
    list_val.list_value = {"item1", "item2", "item3"};
    db.setValue("list_key", list_val);
    
    // Set type
    RedisValue set_val(RedisType::SET);
    set_val.set_value = {"member1", "member2", "member3"};
    db.setValue("set_key", set_val);
    
    EXPECT_TRUE(db.keyExists("string_key"));
    EXPECT_TRUE(db.keyExists("list_key"));
    EXPECT_TRUE(db.keyExists("set_key"));
    
    RedisValue* retrieved_list = db.getValue("list_key");
    ASSERT_NE(retrieved_list, nullptr);
    EXPECT_EQ(retrieved_list->type, RedisType::LIST);
    EXPECT_EQ(retrieved_list->list_value.size(), 3);
}

// Test deleteKey functionality
TEST_F(RedisDatabaseTest, DeleteKey) {
    RedisValue value("test_value");
    db.setValue("test_key", value);
    
    EXPECT_TRUE(db.keyExists("test_key"));
    EXPECT_TRUE(db.deleteKey("test_key"));
    EXPECT_FALSE(db.keyExists("test_key"));
    EXPECT_FALSE(db.deleteKey("nonexistent_key"));
}

// Test clearDatabase functionality
TEST_F(RedisDatabaseTest, ClearDatabase) {
    RedisValue value1("value1");
    RedisValue value2("value2");
    
    db.setValue("key1", value1);
    db.setValue("key2", value2);
    
    EXPECT_EQ(db.getDatabaseSize(), 2);
    db.clearDatabase();
    EXPECT_EQ(db.getDatabaseSize(), 0);
}

// Test getDatabaseSize
TEST_F(RedisDatabaseTest, GetDatabaseSize) {
    EXPECT_EQ(db.getDatabaseSize(), 0);
    
    RedisValue value("test_value");
    db.setValue("key1", value);
    EXPECT_EQ(db.getDatabaseSize(), 1);
    
    db.setValue("key2", value);
    EXPECT_EQ(db.getDatabaseSize(), 2);
    
    db.deleteKey("key1");
    EXPECT_EQ(db.getDatabaseSize(), 1);
}

// Test cleanupExpiredKeys
TEST_F(RedisDatabaseTest, CleanupExpiredKeys) {
    // Create keys with different expiry times
    RedisValue expired_val("expired");
    expired_val.setExpiry(std::chrono::milliseconds(1));
    db.setValue("expired_key", expired_val);
    
    RedisValue valid_val("valid");
    valid_val.setExpiry(std::chrono::hours(1)); // Long expiry
    db.setValue("valid_key", valid_val);
    
    RedisValue no_expiry_val("no_expiry");
    db.setValue("no_expiry_key", no_expiry_val);
    
    // Wait for the expired key to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Clean up expired keys
    db.cleanupExpiredKeys();
    
    EXPECT_FALSE(db.keyExists("expired_key"));
    EXPECT_TRUE(db.keyExists("valid_key"));
    EXPECT_TRUE(db.keyExists("no_expiry_key"));
    EXPECT_EQ(db.getDatabaseSize(), 2);
}

// Test getMatchingKeys with wildcard patterns
TEST_F(RedisDatabaseTest, GetMatchingKeys) {
    RedisValue value("test_value");
    
    db.setValue("user:1:name", value);
    db.setValue("user:2:name", value);
    db.setValue("user:1:email", value);
    db.setValue("product:1:name", value);
    db.setValue("config:server", value);
    
    // Test exact match
    auto exact_match = db.getMatchingKeys("user:1:name");
    EXPECT_EQ(exact_match.size(), 1);
    EXPECT_EQ(exact_match[0], "user:1:name");
    
    // Test wildcard pattern
    auto user_keys = db.getMatchingKeys("user:*");
    EXPECT_EQ(user_keys.size(), 3);
    
    auto user_1_keys = db.getMatchingKeys("user:1:*");
    EXPECT_EQ(user_1_keys.size(), 2);
    
    auto all_keys = db.getMatchingKeys("*");
    EXPECT_EQ(all_keys.size(), 5);
}

// Test getMatchingKeys with question mark pattern
TEST_F(RedisDatabaseTest, GetMatchingKeysQuestionMark) {
    RedisValue value("test_value");
    
    db.setValue("cat", value);
    db.setValue("bat", value);
    db.setValue("rat", value);
    db.setValue("cats", value);
    
    auto three_letter = db.getMatchingKeys("?at");
    EXPECT_EQ(three_letter.size(), 3); // cat, bat, rat
    
    auto four_letter = db.getMatchingKeys("???s");
    EXPECT_EQ(four_letter.size(), 1); // cats
}

// Test getMatchingKeys with expired keys
TEST_F(RedisDatabaseTest, GetMatchingKeysWithExpired) {
    RedisValue valid_val("valid");
    RedisValue expired_val("expired");
    expired_val.setExpiry(std::chrono::milliseconds(1));
    
    db.setValue("valid_key", valid_val);
    db.setValue("expired_key", expired_val);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto keys = db.getMatchingKeys("*");
    EXPECT_EQ(keys.size(), 1);
    EXPECT_EQ(keys[0], "valid_key");
}

// Test thread safety
TEST_F(RedisDatabaseTest, ThreadSafety) {
    const int num_threads = 10;
    const int operations_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                std::string key = "key_" + std::to_string(i) + "_" + std::to_string(j);
                RedisValue value("value_" + std::to_string(i) + "_" + std::to_string(j));
                
                db.setValue(key, value);
                
                // Verify the value was set correctly
                RedisValue* retrieved = db.getValue(key);
                EXPECT_NE(retrieved, nullptr);
                if (retrieved) {
                    EXPECT_EQ(retrieved->string_value, value.string_value);
                }
                
                // Verify key exists
                EXPECT_TRUE(db.keyExists(key));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all keys are present
    EXPECT_EQ(db.getDatabaseSize(), num_threads * operations_per_thread);
}

// Test move semantics in setValue
TEST_F(RedisDatabaseTest, SetValueMoveSemantics) {
    RedisValue original_value("original");
    std::string original_content = original_value.string_value;
    
    db.setValue("test_key", std::move(original_value));
    
    // The original value might be in a valid but unspecified state
    RedisValue* retrieved = db.getValue("test_key");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->string_value, original_content);
}

// Test special characters in keys for pattern matching
TEST_F(RedisDatabaseTest, GetMatchingKeysSpecialCharacters) {
    RedisValue value("test_value");
    
    db.setValue("user.name", value);
    db.setValue("user-name", value);
    db.setValue("user+name", value);
    db.setValue("user(name)", value);
    
    auto dot_keys = db.getMatchingKeys("user.name");
    EXPECT_EQ(dot_keys.size(), 1);
    
    auto wildcard_keys = db.getMatchingKeys("user*");
    EXPECT_EQ(wildcard_keys.size(), 4);
}

// Test empty pattern
TEST_F(RedisDatabaseTest, GetMatchingKeysEmptyPattern) {
    RedisValue value("test_value");
    db.setValue("test_key", value);
    
    auto empty_pattern = db.getMatchingKeys("");
    EXPECT_TRUE(empty_pattern.empty());
    
    auto only_wildcard = db.getMatchingKeys("*");
    EXPECT_EQ(only_wildcard.size(), 1);
}
