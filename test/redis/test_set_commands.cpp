// test_set_commands.cpp
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <set>
#include "redis/commands/set_commands.h"
#include "redis/database/redis_database.h"

// Test fixture for SetCommands tests
class SetCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple in-memory database for testing
        database = new RedisDatabase();
        setCommands = new SetCommands(*database);
        
        // Add some test data
        RedisValue set1(RedisType::SET);
        set1.set_value = {"member1", "member2", "member3"};
        database->setValue("existing_set", set1);
        
        RedisValue set2(RedisType::SET);
        set2.set_value = {"apple", "banana", "cherry"};
        database->setValue("fruits_set", set2);
        
        RedisValue empty_set(RedisType::SET);
        database->setValue("empty_set", empty_set);
        
        // Add a non-set value for type checking
        database->setValue("string_key", RedisValue("not_a_set"));
    }
    
    void TearDown() override {
        delete setCommands;
        delete database;
    }
    
    RedisDatabase* database;
    SetCommands* setCommands;
};

// Test SADD command with new set
TEST_F(SetCommandsTest, Sadd_NewSet_ReturnsAddedCount) {
    std::vector<std::string> args = {"SADD", "new_set", "member1", "member2", "member3"};
    std::string result = setCommands->cmdSadd(args);
    
    EXPECT_EQ(":3\r\n", result);
    
    // Verify set was created with members
    RedisValue* value = database->getValue("new_set");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::SET) {
        EXPECT_EQ(3, value->set_value.size());
        EXPECT_TRUE(value->set_value.count("member1") > 0);
        EXPECT_TRUE(value->set_value.count("member2") > 0);
        EXPECT_TRUE(value->set_value.count("member3") > 0);
    }
}

// Test SADD command with existing set
TEST_F(SetCommandsTest, Sadd_ExistingSet_AddsOnlyNewMembers) {
    std::vector<std::string> args = {"SADD", "existing_set", "member2", "member4", "member5"};
    std::string result = setCommands->cmdSadd(args);
    
    // member2 already exists, so only 2 new members added
    EXPECT_EQ(":2\r\n", result);
    
    // Verify members were added
    RedisValue* value = database->getValue("existing_set");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::SET) {
        EXPECT_EQ(5, value->set_value.size()); // 3 original + 2 new
        EXPECT_TRUE(value->set_value.count("member1") > 0);
        EXPECT_TRUE(value->set_value.count("member2") > 0);
        EXPECT_TRUE(value->set_value.count("member3") > 0);
        EXPECT_TRUE(value->set_value.count("member4") > 0);
        EXPECT_TRUE(value->set_value.count("member5") > 0);
    }
}

// Test SADD command with wrong number of arguments
TEST_F(SetCommandsTest, Sadd_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"SADD", "key"}; // Missing members
    std::string result = setCommands->cmdSadd(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test SADD command with wrong type
TEST_F(SetCommandsTest, Sadd_WrongType_ReturnsError) {
    std::vector<std::string> args = {"SADD", "string_key", "member1"};
    std::string result = setCommands->cmdSadd(args);
    
    EXPECT_TRUE(result.find("ERR Operation against a key holding the wrong kind of value") != std::string::npos);
}

// Test SREM command with existing members
TEST_F(SetCommandsTest, Srem_ExistingMembers_ReturnsRemovedCount) {
    std::vector<std::string> args = {"SREM", "existing_set", "member1", "member3", "non_existent_member"};
    std::string result = setCommands->cmdSrem(args);
    
    // Only member1 and member3 exist, so 2 removed
    EXPECT_EQ(":2\r\n", result);
    
    // Verify members were removed
    RedisValue* value = database->getValue("existing_set");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::SET) {
        EXPECT_EQ(1, value->set_value.size()); // Only member2 remains
        EXPECT_TRUE(value->set_value.count("member2") > 0);
        EXPECT_FALSE(value->set_value.count("member1") > 0);
        EXPECT_FALSE(value->set_value.count("member3") > 0);
    }
}

// Test SREM command removes empty set
TEST_F(SetCommandsTest, Srem_RemovesEmptySet_DeletesKey) {
    std::vector<std::string> args = {"SREM", "existing_set", "member1", "member2", "member3"};
    std::string result = setCommands->cmdSrem(args);
    
    EXPECT_EQ(":3\r\n", result);
    
    // Verify set was deleted since it's empty
    RedisValue* value = database->getValue("existing_set");
    EXPECT_EQ(nullptr, value);
}

// Test SREM command with non-existent set
TEST_F(SetCommandsTest, Srem_NonExistentSet_ReturnsZero) {
    std::vector<std::string> args = {"SREM", "non_existent_set", "member1"};
    std::string result = setCommands->cmdSrem(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SREM command with wrong type
TEST_F(SetCommandsTest, Srem_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"SREM", "string_key", "member1"};
    std::string result = setCommands->cmdSrem(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SISMEMBER command with existing member
TEST_F(SetCommandsTest, Sismember_ExistingMember_ReturnsOne) {
    std::vector<std::string> args = {"SISMEMBER", "existing_set", "member1"};
    std::string result = setCommands->cmdSismember(args);
    
    EXPECT_EQ(":1\r\n", result);
}

// Test SISMEMBER command with non-existent member
TEST_F(SetCommandsTest, Sismember_NonExistentMember_ReturnsZero) {
    std::vector<std::string> args = {"SISMEMBER", "existing_set", "non_existent_member"};
    std::string result = setCommands->cmdSismember(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SISMEMBER command with non-existent set
TEST_F(SetCommandsTest, Sismember_NonExistentSet_ReturnsZero) {
    std::vector<std::string> args = {"SISMEMBER", "non_existent_set", "member1"};
    std::string result = setCommands->cmdSismember(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SISMEMBER command with wrong type
TEST_F(SetCommandsTest, Sismember_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"SISMEMBER", "string_key", "member1"};
    std::string result = setCommands->cmdSismember(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SISMEMBER command with wrong number of arguments
TEST_F(SetCommandsTest, Sismember_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"SISMEMBER", "existing_set"}; // Missing member
    std::string result = setCommands->cmdSismember(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test SCARD command with non-empty set
TEST_F(SetCommandsTest, Scard_NonEmptySet_ReturnsCardinality) {
    std::vector<std::string> args = {"SCARD", "existing_set"};
    std::string result = setCommands->cmdScard(args);
    
    EXPECT_EQ(":3\r\n", result); // 3 members
}

// Test SCARD command with empty set
TEST_F(SetCommandsTest, Scard_EmptySet_ReturnsZero) {
    std::vector<std::string> args = {"SCARD", "empty_set"};
    std::string result = setCommands->cmdScard(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SCARD command with non-existent set
TEST_F(SetCommandsTest, Scard_NonExistentSet_ReturnsZero) {
    std::vector<std::string> args = {"SCARD", "non_existent_set"};
    std::string result = setCommands->cmdScard(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SCARD command with wrong type
TEST_F(SetCommandsTest, Scard_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"SCARD", "string_key"};
    std::string result = setCommands->cmdScard(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test SCARD command with wrong number of arguments
TEST_F(SetCommandsTest, Scard_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"SCARD"}; // Missing key
    std::string result = setCommands->cmdScard(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test SMEMBERS command with non-empty set
TEST_F(SetCommandsTest, Smembers_NonEmptySet_ReturnsAllMembers) {
    std::vector<std::string> args = {"SMEMBERS", "fruits_set"};
    std::string result = setCommands->cmdSmembers(args);
    
    // Should return all members in an array (order may vary)
    EXPECT_TRUE(result.find("apple") != std::string::npos);
    EXPECT_TRUE(result.find("banana") != std::string::npos);
    EXPECT_TRUE(result.find("cherry") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
}

// Test SMEMBERS command with empty set
TEST_F(SetCommandsTest, Smembers_EmptySet_ReturnsEmptyArray) {
    std::vector<std::string> args = {"SMEMBERS", "empty_set"};
    std::string result = setCommands->cmdSmembers(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test SMEMBERS command with non-existent set
TEST_F(SetCommandsTest, Smembers_NonExistentSet_ReturnsEmptyArray) {
    std::vector<std::string> args = {"SMEMBERS", "non_existent_set"};
    std::string result = setCommands->cmdSmembers(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test SMEMBERS command with wrong type
TEST_F(SetCommandsTest, Smembers_WrongType_ReturnsEmptyArray) {
    std::vector<std::string> args = {"SMEMBERS", "string_key"};
    std::string result = setCommands->cmdSmembers(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test SMEMBERS command with wrong number of arguments
TEST_F(SetCommandsTest, Smembers_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"SMEMBERS"}; // Missing key
    std::string result = setCommands->cmdSmembers(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test SPOP command with non-empty set
TEST_F(SetCommandsTest, Spop_NonEmptySet_ReturnsRandomMember) {
    std::vector<std::string> args = {"SPOP", "fruits_set"};
    std::string result = setCommands->cmdSpop(args);
    
    // Should return one of the fruits
    EXPECT_TRUE(result.find("apple") != std::string::npos || 
                result.find("banana") != std::string::npos || 
                result.find("cherry") != std::string::npos);
    
    // Verify set size decreased by 1
    RedisValue* value = database->getValue("fruits_set");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::SET) {
        EXPECT_EQ(2, value->set_value.size());
    }
}

// Test SPOP command with empty set
TEST_F(SetCommandsTest, Spop_EmptySet_ReturnsNull) {
    std::vector<std::string> args = {"SPOP", "empty_set"};
    std::string result = setCommands->cmdSpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test SPOP command with non-existent set
TEST_F(SetCommandsTest, Spop_NonExistentSet_ReturnsNull) {
    std::vector<std::string> args = {"SPOP", "non_existent_set"};
    std::string result = setCommands->cmdSpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test SPOP command with wrong type
TEST_F(SetCommandsTest, Spop_WrongType_ReturnsNull) {
    std::vector<std::string> args = {"SPOP", "string_key"};
    std::string result = setCommands->cmdSpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test SPOP command with wrong number of arguments
TEST_F(SetCommandsTest, Spop_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"SPOP"}; // Missing key
    std::string result = setCommands->cmdSpop(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Integration test: Multiple set operations
TEST_F(SetCommandsTest, Integration_MultipleSetOperations) {
    // Create a set with SADD
    std::vector<std::string> sadd_args = {"SADD", "integration_set", "a", "b", "c", "d"};
    std::string sadd_result = setCommands->cmdSadd(sadd_args);
    EXPECT_EQ(":4\r\n", sadd_result);
    
    // Check cardinality with SCARD
    std::vector<std::string> scard_args = {"SCARD", "integration_set"};
    std::string scard_result = setCommands->cmdScard(scard_args);
    EXPECT_EQ(":4\r\n", scard_result);
    
    // Check membership with SISMEMBER
    std::vector<std::string> sismember_args = {"SISMEMBER", "integration_set", "b"};
    std::string sismember_result = setCommands->cmdSismember(sismember_args);
    EXPECT_EQ(":1\r\n", sismember_result);
    
    // Remove some members with SREM
    std::vector<std::string> srem_args = {"SREM", "integration_set", "a", "c", "x"}; // x doesn't exist
    std::string srem_result = setCommands->cmdSrem(srem_args);
    EXPECT_EQ(":2\r\n", srem_result); // Only a and c removed
    
    // Check cardinality again
    scard_result = setCommands->cmdScard(scard_args);
    EXPECT_EQ(":2\r\n", scard_result);
    
    // Get all members with SMEMBERS
    std::vector<std::string> smembers_args = {"SMEMBERS", "integration_set"};
    std::string smembers_result = setCommands->cmdSmembers(smembers_args);
    EXPECT_TRUE(smembers_result.find("b") != std::string::npos);
    EXPECT_TRUE(smembers_result.find("d") != std::string::npos);
    EXPECT_EQ('*', smembers_result[0]); // Array response
    
    // Pop a member with SPOP
    std::vector<std::string> spop_args = {"SPOP", "integration_set"};
    std::string spop_result = setCommands->cmdSpop(spop_args);
    EXPECT_TRUE(spop_result.find("b") != std::string::npos || spop_result.find("d") != std::string::npos);
    
    // Final cardinality
    scard_result = setCommands->cmdScard(scard_args);
    EXPECT_EQ(":1\r\n", scard_result);
}

// Test edge case: Duplicate members in SADD
TEST_F(SetCommandsTest, EdgeCase_DuplicateMembersInSadd) {
    std::vector<std::string> args = {"SADD", "duplicate_set", "member1", "member1", "member1"};
    std::string result = setCommands->cmdSadd(args);
    
    // Only one unique member should be added
    EXPECT_EQ(":1\r\n", result);
    
    RedisValue* value = database->getValue("duplicate_set");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::SET) {
        EXPECT_EQ(1, value->set_value.size());
        EXPECT_TRUE(value->set_value.count("member1") > 0);
    }
}

// Test edge case: Large set operations
TEST_F(SetCommandsTest, EdgeCase_LargeSetOperations) {
    // Add many members
    std::vector<std::string> args = {"SADD", "large_set"};
    for (int i = 0; i < 100; i++) {
        args.push_back("member_" + std::to_string(i));
    }
    
    std::string result = setCommands->cmdSadd(args);
    EXPECT_EQ(":100\r\n", result);
    
    // Check cardinality
    std::vector<std::string> scard_args = {"SCARD", "large_set"};
    std::string scard_result = setCommands->cmdScard(scard_args);
    EXPECT_EQ(":100\r\n", scard_result);
    
    // Remove half the members
    std::vector<std::string> srem_args = {"SREM", "large_set"};
    for (int i = 0; i < 50; i++) {
        srem_args.push_back("member_" + std::to_string(i));
    }
    
    std::string srem_result = setCommands->cmdSrem(srem_args);
    EXPECT_EQ(":50\r\n", srem_result);
    
    // Final cardinality
    scard_result = setCommands->cmdScard(scard_args);
    EXPECT_EQ(":50\r\n", scard_result);
}