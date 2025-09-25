// test_hash_commands.cpp
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "redis/commands/hash_commands.h"
#include "redis/database/redis_database.h"

// Test fixture for HashCommands tests
class HashCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple in-memory database for testing
        database = new RedisDatabase();
        hashCommands = new HashCommands(*database);
        
        // Add some test data
        RedisValue hash1(RedisType::HASH);
        hash1.hash_value = {{"field1", "value1"}, {"field2", "value2"}, {"field3", "value3"}};
        database->setValue("existing_hash", hash1);
        
        RedisValue hash2(RedisType::HASH);
        hash2.hash_value = {{"name", "Alice"}, {"age", "30"}, {"city", "New York"}};
        database->setValue("user_hash", hash2);
        
        RedisValue empty_hash(RedisType::HASH);
        database->setValue("empty_hash", empty_hash);
        
        // Add a non-hash value for type checking
        database->setValue("string_key", RedisValue("not_a_hash"));
    }
    
    void TearDown() override {
        delete hashCommands;
        delete database;
    }
    
    RedisDatabase* database;
    HashCommands* hashCommands;
};

// Test HSET command with new hash
TEST_F(HashCommandsTest, Hset_NewHash_ReturnsAddedCount) {
    std::vector<std::string> args = {"HSET", "new_hash", "field1", "value1", "field2", "value2", "field3", "value3"};
    std::string result = hashCommands->cmdHset(args);
    
    EXPECT_EQ(":3\r\n", result); // 3 fields added
    
    // Verify hash was created with fields
    RedisValue* value = database->getValue("new_hash");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::HASH) {
        EXPECT_EQ(3, value->hash_value.size());
        EXPECT_EQ("value1", value->hash_value["field1"]);
        EXPECT_EQ("value2", value->hash_value["field2"]);
        EXPECT_EQ("value3", value->hash_value["field3"]);
    }
}

// Test HSET command with existing hash
TEST_F(HashCommandsTest, Hset_ExistingHash_UpdatesExistingFields) {
    std::vector<std::string> args = {"HSET", "existing_hash", "field2", "new_value", "field4", "value4"};
    std::string result = hashCommands->cmdHset(args);
    
    // field2 exists (not counted as added), field4 is new
    EXPECT_EQ(":1\r\n", result);
    
    // Verify fields were updated/added
    RedisValue* value = database->getValue("existing_hash");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::HASH) {
        EXPECT_EQ(4, value->hash_value.size());
        EXPECT_EQ("value1", value->hash_value["field1"]); // unchanged
        EXPECT_EQ("new_value", value->hash_value["field2"]); // updated
        EXPECT_EQ("value3", value->hash_value["field3"]); // unchanged
        EXPECT_EQ("value4", value->hash_value["field4"]); // new
    }
}

// Test HSET command with wrong number of arguments
TEST_F(HashCommandsTest, Hset_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HSET", "key", "field1"}; // Missing value
    std::string result = hashCommands->cmdHset(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HSET command with odd number of field-value pairs
TEST_F(HashCommandsTest, Hset_OddNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HSET", "key", "field1", "value1", "field2"}; // Missing value for field2
    std::string result = hashCommands->cmdHset(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HSET command with wrong type
TEST_F(HashCommandsTest, Hset_WrongType_ReturnsError) {
    std::vector<std::string> args = {"HSET", "string_key", "field1", "value1"};
    std::string result = hashCommands->cmdHset(args);
    
    EXPECT_TRUE(result.find("ERR Operation against a key holding the wrong kind of value") != std::string::npos);
}

// Test HGET command with existing field
TEST_F(HashCommandsTest, Hget_ExistingField_ReturnsValue) {
    std::vector<std::string> args = {"HGET", "existing_hash", "field1"};
    std::string result = hashCommands->cmdHget(args);
    
    EXPECT_EQ("$6\r\nvalue1\r\n", result);
}

// Test HGET command with non-existent field
TEST_F(HashCommandsTest, Hget_NonExistentField_ReturnsNull) {
    std::vector<std::string> args = {"HGET", "existing_hash", "non_existent_field"};
    std::string result = hashCommands->cmdHget(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test HGET command with non-existent hash
TEST_F(HashCommandsTest, Hget_NonExistentHash_ReturnsNull) {
    std::vector<std::string> args = {"HGET", "non_existent_hash", "field1"};
    std::string result = hashCommands->cmdHget(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test HGET command with wrong type
TEST_F(HashCommandsTest, Hget_WrongType_ReturnsNull) {
    std::vector<std::string> args = {"HGET", "string_key", "field1"};
    std::string result = hashCommands->cmdHget(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test HGET command with wrong number of arguments
TEST_F(HashCommandsTest, Hget_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HGET", "existing_hash"}; // Missing field
    std::string result = hashCommands->cmdHget(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HDEL command with existing fields
TEST_F(HashCommandsTest, Hdel_ExistingFields_ReturnsDeletedCount) {
    std::vector<std::string> args = {"HDEL", "existing_hash", "field1", "field3", "non_existent_field"};
    std::string result = hashCommands->cmdHdel(args);
    
    // Only field1 and field3 exist, so 2 deleted
    EXPECT_EQ(":2\r\n", result);
    
    // Verify fields were deleted
    RedisValue* value = database->getValue("existing_hash");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::HASH) {
        EXPECT_EQ(1, value->hash_value.size()); // Only field2 remains
        EXPECT_TRUE(value->hash_value.find("field1") == value->hash_value.end());
        EXPECT_TRUE(value->hash_value.find("field3") == value->hash_value.end());
        EXPECT_TRUE(value->hash_value.find("field2") != value->hash_value.end());
    }
}

// Test HDEL command removes empty hash
TEST_F(HashCommandsTest, Hdel_RemovesEmptyHash_DeletesKey) {
    std::vector<std::string> args = {"HDEL", "existing_hash", "field1", "field2", "field3"};
    std::string result = hashCommands->cmdHdel(args);
    
    EXPECT_EQ(":3\r\n", result);
    
    // Verify hash was deleted since it's empty
    RedisValue* value = database->getValue("existing_hash");
    EXPECT_EQ(nullptr, value);
}

// Test HDEL command with non-existent hash
TEST_F(HashCommandsTest, Hdel_NonExistentHash_ReturnsZero) {
    std::vector<std::string> args = {"HDEL", "non_existent_hash", "field1"};
    std::string result = hashCommands->cmdHdel(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HDEL command with wrong type
TEST_F(HashCommandsTest, Hdel_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"HDEL", "string_key", "field1"};
    std::string result = hashCommands->cmdHdel(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HDEL command with wrong number of arguments
TEST_F(HashCommandsTest, Hdel_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HDEL", "existing_hash"}; // Missing fields
    std::string result = hashCommands->cmdHdel(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HEXISTS command with existing field
TEST_F(HashCommandsTest, Hexists_ExistingField_ReturnsOne) {
    std::vector<std::string> args = {"HEXISTS", "existing_hash", "field1"};
    std::string result = hashCommands->cmdHexists(args);
    
    EXPECT_EQ(":1\r\n", result);
}

// Test HEXISTS command with non-existent field
TEST_F(HashCommandsTest, Hexists_NonExistentField_ReturnsZero) {
    std::vector<std::string> args = {"HEXISTS", "existing_hash", "non_existent_field"};
    std::string result = hashCommands->cmdHexists(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HEXISTS command with non-existent hash
TEST_F(HashCommandsTest, Hexists_NonExistentHash_ReturnsZero) {
    std::vector<std::string> args = {"HEXISTS", "non_existent_hash", "field1"};
    std::string result = hashCommands->cmdHexists(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HEXISTS command with wrong type
TEST_F(HashCommandsTest, Hexists_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"HEXISTS", "string_key", "field1"};
    std::string result = hashCommands->cmdHexists(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HEXISTS command with wrong number of arguments
TEST_F(HashCommandsTest, Hexists_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HEXISTS", "existing_hash"}; // Missing field
    std::string result = hashCommands->cmdHexists(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HLEN command with non-empty hash
TEST_F(HashCommandsTest, Hlen_NonEmptyHash_ReturnsFieldCount) {
    std::vector<std::string> args = {"HLEN", "existing_hash"};
    std::string result = hashCommands->cmdHlen(args);
    
    EXPECT_EQ(":3\r\n", result); // 3 fields
}

// Test HLEN command with empty hash
TEST_F(HashCommandsTest, Hlen_EmptyHash_ReturnsZero) {
    std::vector<std::string> args = {"HLEN", "empty_hash"};
    std::string result = hashCommands->cmdHlen(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HLEN command with non-existent hash
TEST_F(HashCommandsTest, Hlen_NonExistentHash_ReturnsZero) {
    std::vector<std::string> args = {"HLEN", "non_existent_hash"};
    std::string result = hashCommands->cmdHlen(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HLEN command with wrong type
TEST_F(HashCommandsTest, Hlen_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"HLEN", "string_key"};
    std::string result = hashCommands->cmdHlen(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test HLEN command with wrong number of arguments
TEST_F(HashCommandsTest, Hlen_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HLEN"}; // Missing key
    std::string result = hashCommands->cmdHlen(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HKEYS command with non-empty hash
TEST_F(HashCommandsTest, Hkeys_NonEmptyHash_ReturnsAllKeys) {
    std::vector<std::string> args = {"HKEYS", "user_hash"};
    std::string result = hashCommands->cmdHkeys(args);
    
    // Should return all field names in an array
    EXPECT_TRUE(result.find("name") != std::string::npos);
    EXPECT_TRUE(result.find("age") != std::string::npos);
    EXPECT_TRUE(result.find("city") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
}

// Test HKEYS command with empty hash
TEST_F(HashCommandsTest, Hkeys_EmptyHash_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HKEYS", "empty_hash"};
    std::string result = hashCommands->cmdHkeys(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HKEYS command with non-existent hash
TEST_F(HashCommandsTest, Hkeys_NonExistentHash_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HKEYS", "non_existent_hash"};
    std::string result = hashCommands->cmdHkeys(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HKEYS command with wrong type
TEST_F(HashCommandsTest, Hkeys_WrongType_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HKEYS", "string_key"};
    std::string result = hashCommands->cmdHkeys(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HKEYS command with wrong number of arguments
TEST_F(HashCommandsTest, Hkeys_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HKEYS"}; // Missing key
    std::string result = hashCommands->cmdHkeys(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HVALS command with non-empty hash
TEST_F(HashCommandsTest, Hvals_NonEmptyHash_ReturnsAllValues) {
    std::vector<std::string> args = {"HVALS", "user_hash"};
    std::string result = hashCommands->cmdHvals(args);
    
    // Should return all field values in an array
    EXPECT_TRUE(result.find("Alice") != std::string::npos);
    EXPECT_TRUE(result.find("30") != std::string::npos);
    EXPECT_TRUE(result.find("New York") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
}

// Test HVALS command with empty hash
TEST_F(HashCommandsTest, Hvals_EmptyHash_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HVALS", "empty_hash"};
    std::string result = hashCommands->cmdHvals(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HVALS command with non-existent hash
TEST_F(HashCommandsTest, Hvals_NonExistentHash_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HVALS", "non_existent_hash"};
    std::string result = hashCommands->cmdHvals(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HVALS command with wrong type
TEST_F(HashCommandsTest, Hvals_WrongType_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HVALS", "string_key"};
    std::string result = hashCommands->cmdHvals(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HVALS command with wrong number of arguments
TEST_F(HashCommandsTest, Hvals_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HVALS"}; // Missing key
    std::string result = hashCommands->cmdHvals(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test HGETALL command with non-empty hash
TEST_F(HashCommandsTest, Hgetall_NonEmptyHash_ReturnsAllFieldsAndValues) {
    std::vector<std::string> args = {"HGETALL", "user_hash"};
    std::string result = hashCommands->cmdHgetall(args);
    
    // Should return field-value pairs in an array
    EXPECT_TRUE(result.find("name") != std::string::npos);
    EXPECT_TRUE(result.find("Alice") != std::string::npos);
    EXPECT_TRUE(result.find("age") != std::string::npos);
    EXPECT_TRUE(result.find("30") != std::string::npos);
    EXPECT_TRUE(result.find("city") != std::string::npos);
    EXPECT_TRUE(result.find("New York") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
    
    // Should have 6 elements (3 fields × 2 each)
    EXPECT_EQ('*', result[0]);
    EXPECT_TRUE(result.find("*6\r\n") == 0 || result.find("*6\r\n") != std::string::npos);
}

// Test HGETALL command with empty hash
TEST_F(HashCommandsTest, Hgetall_EmptyHash_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HGETALL", "empty_hash"};
    std::string result = hashCommands->cmdHgetall(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HGETALL command with non-existent hash
TEST_F(HashCommandsTest, Hgetall_NonExistentHash_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HGETALL", "non_existent_hash"};
    std::string result = hashCommands->cmdHgetall(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HGETALL command with wrong type
TEST_F(HashCommandsTest, Hgetall_WrongType_ReturnsEmptyArray) {
    std::vector<std::string> args = {"HGETALL", "string_key"};
    std::string result = hashCommands->cmdHgetall(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test HGETALL command with wrong number of arguments
TEST_F(HashCommandsTest, Hgetall_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"HGETALL"}; // Missing key
    std::string result = hashCommands->cmdHgetall(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Integration test: Multiple hash operations
TEST_F(HashCommandsTest, Integration_MultipleHashOperations) {
    // Create a hash with HSET
    std::vector<std::string> hset_args = {"HSET", "integration_hash", "name", "John", "age", "25", "city", "London"};
    std::string hset_result = hashCommands->cmdHset(hset_args);
    EXPECT_EQ(":3\r\n", hset_result);
    
    // Check field existence with HEXISTS
    std::vector<std::string> hexists_args = {"HEXISTS", "integration_hash", "name"};
    std::string hexists_result = hashCommands->cmdHexists(hexists_args);
    EXPECT_EQ(":1\r\n", hexists_result);
    
    // Get a value with HGET
    std::vector<std::string> hget_args = {"HGET", "integration_hash", "age"};
    std::string hget_result = hashCommands->cmdHget(hget_args);
    EXPECT_EQ("$2\r\n25\r\n", hget_result);
    
    // Check field count with HLEN
    std::vector<std::string> hlen_args = {"HLEN", "integration_hash"};
    std::string hlen_result = hashCommands->cmdHlen(hlen_args);
    EXPECT_EQ(":3\r\n", hlen_result);
    
    // Update a field with HSET
    std::vector<std::string> hset_update_args = {"HSET", "integration_hash", "age", "26", "country", "UK"};
    std::string hset_update_result = hashCommands->cmdHset(hset_update_args);
    EXPECT_EQ(":1\r\n", hset_update_result); // Only country is new
    
    // Check updated field count
    hlen_result = hashCommands->cmdHlen(hlen_args);
    EXPECT_EQ(":4\r\n", hlen_result);
    
    // Get all keys with HKEYS
    std::vector<std::string> hkeys_args = {"HKEYS", "integration_hash"};
    std::string hkeys_result = hashCommands->cmdHkeys(hkeys_args);
    EXPECT_TRUE(hkeys_result.find("name") != std::string::npos);
    EXPECT_TRUE(hkeys_result.find("age") != std::string::npos);
    EXPECT_TRUE(hkeys_result.find("city") != std::string::npos);
    EXPECT_TRUE(hkeys_result.find("country") != std::string::npos);
    
    // Get all values with HVALS
    std::vector<std::string> hvals_args = {"HVALS", "integration_hash"};
    std::string hvals_result = hashCommands->cmdHvals(hvals_args);
    EXPECT_TRUE(hvals_result.find("John") != std::string::npos);
    EXPECT_TRUE(hvals_result.find("26") != std::string::npos); // Updated age
    EXPECT_TRUE(hvals_result.find("London") != std::string::npos);
    EXPECT_TRUE(hvals_result.find("UK") != std::string::npos);
    
    // Get everything with HGETALL
    std::vector<std::string> hgetall_args = {"HGETALL", "integration_hash"};
    std::string hgetall_result = hashCommands->cmdHgetall(hgetall_args);
    EXPECT_TRUE(hgetall_result.find("name") != std::string::npos);
    EXPECT_TRUE(hgetall_result.find("John") != std::string::npos);
    EXPECT_TRUE(hgetall_result.find("age") != std::string::npos);
    EXPECT_TRUE(hgetall_result.find("26") != std::string::npos);
    
    // Delete some fields with HDEL
    std::vector<std::string> hdel_args = {"HDEL", "integration_hash", "city", "country", "non_existent"};
    std::string hdel_result = hashCommands->cmdHdel(hdel_args);
    EXPECT_EQ(":2\r\n", hdel_result);
    
    // Final field count
    hlen_result = hashCommands->cmdHlen(hlen_args);
    EXPECT_EQ(":2\r\n", hlen_result);
}

// Test edge case: Empty field names and values
TEST_F(HashCommandsTest, EdgeCase_EmptyFieldNamesAndValues) {
    std::vector<std::string> args = {"HSET", "edge_hash", "", "empty_value", "empty_field", ""};
    std::string result = hashCommands->cmdHset(args);
    
    EXPECT_EQ(":2\r\n", result);
    
    RedisValue* value = database->getValue("edge_hash");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::HASH) {
        EXPECT_EQ(2, value->hash_value.size());
        EXPECT_EQ("empty_value", value->hash_value[""]);
        EXPECT_EQ("", value->hash_value["empty_field"]);
    }
}

// Test edge case: Large hash operations
TEST_F(HashCommandsTest, EdgeCase_LargeHashOperations) {
    // Add many fields
    std::vector<std::string> args = {"HSET", "large_hash"};
    for (int i = 0; i < 100; i++) {
        args.push_back("field_" + std::to_string(i));
        args.push_back("value_" + std::to_string(i));
    }
    
    std::string result = hashCommands->cmdHset(args);
    EXPECT_EQ(":100\r\n", result);
    
    // Check field count
    std::vector<std::string> hlen_args = {"HLEN", "large_hash"};
    std::string hlen_result = hashCommands->cmdHlen(hlen_args);
    EXPECT_EQ(":100\r\n", hlen_result);
    
    // Get a specific field
    std::vector<std::string> hget_args = {"HGET", "large_hash", "field_50"};
    std::string hget_result = hashCommands->cmdHget(hget_args);
    
    EXPECT_EQ("$8\r\nvalue_50\r\n", hget_result);
    
    // Delete half the fields
    std::vector<std::string> hdel_args = {"HDEL", "large_hash"};
    for (int i = 0; i < 50; i++) {
        hdel_args.push_back("field_" + std::to_string(i));
    }
    
    std::string hdel_result = hashCommands->cmdHdel(hdel_args);
    EXPECT_EQ(":50\r\n", hdel_result);
    
    // Final field count
    hlen_result = hashCommands->cmdHlen(hlen_args);
    EXPECT_EQ(":50\r\n", hlen_result);
}

// Test edge case: Special characters in field names and values
TEST_F(HashCommandsTest, EdgeCase_SpecialCharacters) {
    std::vector<std::string> args = {"HSET", "special_hash", "field with spaces", "value with spaces", "field\nwith\nnewlines", "value\nwith\nnewlines"};
    std::string result = hashCommands->cmdHset(args);
    
    EXPECT_EQ(":2\r\n", result);
    
    RedisValue* value = database->getValue("special_hash");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::HASH) {
        EXPECT_EQ(2, value->hash_value.size());
        EXPECT_EQ("value with spaces", value->hash_value["field with spaces"]);
        EXPECT_EQ("value\nwith\nnewlines", value->hash_value["field\nwith\nnewlines"]);
    }
}