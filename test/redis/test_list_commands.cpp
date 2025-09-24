// test_list_commands.cpp
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <list>
#include "redis/commands/list_commands.h"
#include "redis/database/redis_database.h"

// Test fixture for ListCommands tests
class ListCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple in-memory database for testing
        database = new RedisDatabase();
        listCommands = new ListCommands(*database);
        
        // Add some test data
        RedisValue list1(RedisType::LIST);
        list1.list_value = {"item1", "item2", "item3"};
        database->setValue("existing_list", list1);
        
        RedisValue list2(RedisType::LIST);
        list2.list_value = {"a", "b", "c", "d", "e"};
        database->setValue("long_list", list2);
        
        RedisValue empty_list(RedisType::LIST);
        database->setValue("empty_list", empty_list);
        
        // Add a non-list value for type checking
        database->setValue("string_key", RedisValue("not_a_list"));
    }
    
    void TearDown() override {
        delete listCommands;
        delete database;
    }
    
    RedisDatabase* database;
    ListCommands* listCommands;
};

// Test LPUSH command with new list
TEST_F(ListCommandsTest, Lpush_NewList_ReturnsListSize) {
    std::vector<std::string> args = {"LPUSH", "new_list", "value1", "value2", "value3"};
    std::string result = listCommands->cmdLpush(args);
    
    EXPECT_EQ(":3\r\n", result);
    
    // Verify list was created with elements in correct order (reverse insertion)
    RedisValue* value = database->getValue("new_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        EXPECT_EQ(3, value->list_value.size());
        auto it = value->list_value.begin();
        EXPECT_EQ("value3", *it++); // Last pushed element is first
        EXPECT_EQ("value2", *it++);
        EXPECT_EQ("value1", *it);   // First pushed element is last
    }
}

// Test LPUSH command with existing list
TEST_F(ListCommandsTest, Lpush_ExistingList_AddsToFront) {
    std::vector<std::string> args = {"LPUSH", "existing_list", "new_item1", "new_item2"};
    std::string result = listCommands->cmdLpush(args);
    
    EXPECT_EQ(":5\r\n", result); // 3 original + 2 new
    
    // Verify elements were added to front in reverse order
    RedisValue* value = database->getValue("existing_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        EXPECT_EQ(5, value->list_value.size());
        auto it = value->list_value.begin();
        EXPECT_EQ("new_item2", *it++); // Last pushed element is first
        EXPECT_EQ("new_item1", *it++);
        EXPECT_EQ("item1", *it++);
        EXPECT_EQ("item2", *it++);
        EXPECT_EQ("item3", *it);
    }
}

// Test LPUSH command with wrong number of arguments
TEST_F(ListCommandsTest, Lpush_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"LPUSH", "key"}; // Missing values
    std::string result = listCommands->cmdLpush(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("lpush") != std::string::npos);
}

// Test LPUSH command with wrong type
TEST_F(ListCommandsTest, Lpush_WrongType_ReturnsError) {
    std::vector<std::string> args = {"LPUSH", "string_key", "value1"};
    std::string result = listCommands->cmdLpush(args);
    
    EXPECT_TRUE(result.find("ERR Operation against a key holding the wrong kind of value") != std::string::npos);
}

// Test RPUSH command with new list
TEST_F(ListCommandsTest, Rpush_NewList_ReturnsListSize) {
    std::vector<std::string> args = {"RPUSH", "new_list", "value1", "value2", "value3"};
    std::string result = listCommands->cmdRpush(args);
    
    EXPECT_EQ(":3\r\n", result);
    
    // Verify list was created with elements in correct order
    RedisValue* value = database->getValue("new_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        EXPECT_EQ(3, value->list_value.size());
        auto it = value->list_value.begin();
        EXPECT_EQ("value1", *it++); // First pushed element is first
        EXPECT_EQ("value2", *it++);
        EXPECT_EQ("value3", *it);   // Last pushed element is last
    }
}

// Test RPUSH command with existing list
TEST_F(ListCommandsTest, Rpush_ExistingList_AddsToEnd) {
    std::vector<std::string> args = {"RPUSH", "existing_list", "new_item1", "new_item2"};
    std::string result = listCommands->cmdRpush(args);
    
    EXPECT_EQ(":5\r\n", result); // 3 original + 2 new
    
    // Verify elements were added to end in order
    RedisValue* value = database->getValue("existing_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        EXPECT_EQ(5, value->list_value.size());
        auto it = value->list_value.begin();
        EXPECT_EQ("item1", *it++);
        EXPECT_EQ("item2", *it++);
        EXPECT_EQ("item3", *it++);
        EXPECT_EQ("new_item1", *it++);
        EXPECT_EQ("new_item2", *it);
    }
}

// Test RPUSH command with wrong number of arguments
TEST_F(ListCommandsTest, Rpush_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"RPUSH", "key"}; // Missing values
    std::string result = listCommands->cmdRpush(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
    EXPECT_TRUE(result.find("rpush") != std::string::npos);
}

// Test RPUSH command with wrong type
TEST_F(ListCommandsTest, Rpush_WrongType_ReturnsError) {
    std::vector<std::string> args = {"RPUSH", "string_key", "value1"};
    std::string result = listCommands->cmdRpush(args);
    
    EXPECT_TRUE(result.find("ERR Operation against a key holding the wrong kind of value") != std::string::npos);
}

// Test LPOP command with non-empty list
TEST_F(ListCommandsTest, Lpop_NonEmptyList_ReturnsFirstElement) {
    std::vector<std::string> args = {"LPOP", "existing_list"};
    std::string result = listCommands->cmdLpop(args);
    
    EXPECT_EQ("$5\r\nitem1\r\n", result);
    
    // Verify element was removed from front
    RedisValue* value = database->getValue("existing_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        EXPECT_EQ(2, value->list_value.size());
        auto it = value->list_value.begin();
        EXPECT_EQ("item2", *it++);
        EXPECT_EQ("item3", *it);
    }
}

// Test LPOP command removes empty list
TEST_F(ListCommandsTest, Lpop_RemovesEmptyList_DeletesKey) {
    // Create a list with one element
    RedisValue single_item_list(RedisType::LIST);
    single_item_list.list_value = {"only_item"};
    database->setValue("single_item_list", single_item_list);
    
    std::vector<std::string> args = {"LPOP", "single_item_list"};
    std::string result = listCommands->cmdLpop(args);
    
    EXPECT_EQ("$9\r\nonly_item\r\n", result);
    
    // Verify list was deleted since it's empty
    RedisValue* value = database->getValue("single_item_list");
    EXPECT_EQ(nullptr, value);
}

// Test LPOP command with empty list
TEST_F(ListCommandsTest, Lpop_EmptyList_ReturnsNull) {
    std::vector<std::string> args = {"LPOP", "empty_list"};
    std::string result = listCommands->cmdLpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LPOP command with non-existent list
TEST_F(ListCommandsTest, Lpop_NonExistentList_ReturnsNull) {
    std::vector<std::string> args = {"LPOP", "non_existent_list"};
    std::string result = listCommands->cmdLpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LPOP command with wrong type
TEST_F(ListCommandsTest, Lpop_WrongType_ReturnsNull) {
    std::vector<std::string> args = {"LPOP", "string_key"};
    std::string result = listCommands->cmdLpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LPOP command with wrong number of arguments
TEST_F(ListCommandsTest, Lpop_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"LPOP"}; // Missing key
    std::string result = listCommands->cmdLpop(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test RPOP command with non-empty list
TEST_F(ListCommandsTest, Rpop_NonEmptyList_ReturnsLastElement) {
    std::vector<std::string> args = {"RPOP", "existing_list"};
    std::string result = listCommands->cmdRpop(args);
    
    EXPECT_EQ("$5\r\nitem3\r\n", result);
    
    // Verify element was removed from end
    RedisValue* value = database->getValue("existing_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        EXPECT_EQ(2, value->list_value.size());
        auto it = value->list_value.begin();
        EXPECT_EQ("item1", *it++);
        EXPECT_EQ("item2", *it);
    }
}

// Test RPOP command removes empty list
TEST_F(ListCommandsTest, Rpop_RemovesEmptyList_DeletesKey) {
    // Create a list with one element
    RedisValue single_item_list(RedisType::LIST);
    single_item_list.list_value = {"only_item"};
    database->setValue("single_item_list", single_item_list);
    
    std::vector<std::string> args = {"RPOP", "single_item_list"};
    std::string result = listCommands->cmdRpop(args);
    
    EXPECT_EQ("$9\r\nonly_item\r\n", result);
    
    // Verify list was deleted since it's empty
    RedisValue* value = database->getValue("single_item_list");
    EXPECT_EQ(nullptr, value);
}

// Test RPOP command with empty list
TEST_F(ListCommandsTest, Rpop_EmptyList_ReturnsNull) {
    std::vector<std::string> args = {"RPOP", "empty_list"};
    std::string result = listCommands->cmdRpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test RPOP command with non-existent list
TEST_F(ListCommandsTest, Rpop_NonExistentList_ReturnsNull) {
    std::vector<std::string> args = {"RPOP", "non_existent_list"};
    std::string result = listCommands->cmdRpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test RPOP command with wrong type
TEST_F(ListCommandsTest, Rpop_WrongType_ReturnsNull) {
    std::vector<std::string> args = {"RPOP", "string_key"};
    std::string result = listCommands->cmdRpop(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test RPOP command with wrong number of arguments
TEST_F(ListCommandsTest, Rpop_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"RPOP"}; // Missing key
    std::string result = listCommands->cmdRpop(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test LLEN command with non-empty list
TEST_F(ListCommandsTest, Llen_NonEmptyList_ReturnsLength) {
    std::vector<std::string> args = {"LLEN", "existing_list"};
    std::string result = listCommands->cmdLlen(args);
    
    EXPECT_EQ(":3\r\n", result); // 3 elements
}

// Test LLEN command with empty list
TEST_F(ListCommandsTest, Llen_EmptyList_ReturnsZero) {
    std::vector<std::string> args = {"LLEN", "empty_list"};
    std::string result = listCommands->cmdLlen(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test LLEN command with non-existent list
TEST_F(ListCommandsTest, Llen_NonExistentList_ReturnsZero) {
    std::vector<std::string> args = {"LLEN", "non_existent_list"};
    std::string result = listCommands->cmdLlen(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test LLEN command with wrong type
TEST_F(ListCommandsTest, Llen_WrongType_ReturnsZero) {
    std::vector<std::string> args = {"LLEN", "string_key"};
    std::string result = listCommands->cmdLlen(args);
    
    EXPECT_EQ(":0\r\n", result);
}

// Test LLEN command with wrong number of arguments
TEST_F(ListCommandsTest, Llen_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"LLEN"}; // Missing key
    std::string result = listCommands->cmdLlen(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test LRANGE command with valid range
TEST_F(ListCommandsTest, Lrange_ValidRange_ReturnsElements) {
    std::vector<std::string> args = {"LRANGE", "long_list", "1", "3"};
    std::string result = listCommands->cmdLrange(args);
    
    // Should return elements at indices 1, 2, 3: "b", "c", "d"
    EXPECT_TRUE(result.find("b") != std::string::npos);
    EXPECT_TRUE(result.find("c") != std::string::npos);
    EXPECT_TRUE(result.find("d") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
}

// Test LRANGE command with negative indices
TEST_F(ListCommandsTest, Lrange_NegativeIndices_ReturnsCorrectElements) {
    std::vector<std::string> args = {"LRANGE", "long_list", "-3", "-1"};
    std::string result = listCommands->cmdLrange(args);
    
    // Should return last 3 elements: "c", "d", "e"
    EXPECT_TRUE(result.find("c") != std::string::npos);
    EXPECT_TRUE(result.find("d") != std::string::npos);
    EXPECT_TRUE(result.find("e") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
}

// Test LRANGE command with start > end
TEST_F(ListCommandsTest, Lrange_StartGreaterThanEnd_ReturnsEmptyArray) {
    std::vector<std::string> args = {"LRANGE", "long_list", "3", "1"};
    std::string result = listCommands->cmdLrange(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test LRANGE command with out of bounds indices
TEST_F(ListCommandsTest, Lrange_OutOfBounds_ReturnsValidRange) {
    std::vector<std::string> args = {"LRANGE", "long_list", "-10", "10"};
    std::string result = listCommands->cmdLrange(args);
    
    // Should return all elements
    EXPECT_TRUE(result.find("a") != std::string::npos);
    EXPECT_TRUE(result.find("b") != std::string::npos);
    EXPECT_TRUE(result.find("c") != std::string::npos);
    EXPECT_TRUE(result.find("d") != std::string::npos);
    EXPECT_TRUE(result.find("e") != std::string::npos);
    EXPECT_EQ('*', result[0]); // Array response
}

// Test LRANGE command with empty list
TEST_F(ListCommandsTest, Lrange_EmptyList_ReturnsEmptyArray) {
    std::vector<std::string> args = {"LRANGE", "empty_list", "0", "-1"};
    std::string result = listCommands->cmdLrange(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test LRANGE command with non-existent list
TEST_F(ListCommandsTest, Lrange_NonExistentList_ReturnsEmptyArray) {
    std::vector<std::string> args = {"LRANGE", "non_existent_list", "0", "-1"};
    std::string result = listCommands->cmdLrange(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test LRANGE command with wrong type
TEST_F(ListCommandsTest, Lrange_WrongType_ReturnsEmptyArray) {
    std::vector<std::string> args = {"LRANGE", "string_key", "0", "-1"};
    std::string result = listCommands->cmdLrange(args);
    
    EXPECT_EQ("*0\r\n", result); // Empty array
}

// Test LRANGE command with non-integer indices
TEST_F(ListCommandsTest, Lrange_NonIntegerIndices_ReturnsError) {
    std::vector<std::string> args = {"LRANGE", "long_list", "start", "end"};
    std::string result = listCommands->cmdLrange(args);
    
    EXPECT_TRUE(result.find("ERR value is not an integer or out of range") != std::string::npos);
}

// Test LRANGE command with wrong number of arguments
TEST_F(ListCommandsTest, Lrange_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"LRANGE", "long_list", "0"}; // Missing end index
    std::string result = listCommands->cmdLrange(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test LINDEX command with valid index
TEST_F(ListCommandsTest, Lindex_ValidIndex_ReturnsElement) {
    std::vector<std::string> args = {"LINDEX", "long_list", "2"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_EQ("$1\r\nc\r\n", result); // Element at index 2 is "c"
}

// Test LINDEX command with negative index
TEST_F(ListCommandsTest, Lindex_NegativeIndex_ReturnsElement) {
    std::vector<std::string> args = {"LINDEX", "long_list", "-1"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_EQ("$1\r\ne\r\n", result); // Last element is "e"
}

// Test LINDEX command with out of bounds index
TEST_F(ListCommandsTest, Lindex_OutOfBounds_ReturnsNull) {
    std::vector<std::string> args = {"LINDEX", "long_list", "10"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LINDEX command with empty list
TEST_F(ListCommandsTest, Lindex_EmptyList_ReturnsNull) {
    std::vector<std::string> args = {"LINDEX", "empty_list", "0"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LINDEX command with non-existent list
TEST_F(ListCommandsTest, Lindex_NonExistentList_ReturnsNull) {
    std::vector<std::string> args = {"LINDEX", "non_existent_list", "0"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LINDEX command with wrong type
TEST_F(ListCommandsTest, Lindex_WrongType_ReturnsNull) {
    std::vector<std::string> args = {"LINDEX", "string_key", "0"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_EQ("$-1\r\n", result); // Null response
}

// Test LINDEX command with non-integer index
TEST_F(ListCommandsTest, Lindex_NonIntegerIndex_ReturnsError) {
    std::vector<std::string> args = {"LINDEX", "long_list", "invalid"};
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_TRUE(result.find("ERR value is not an integer or out of range") != std::string::npos);
}

// Test LINDEX command with wrong number of arguments
TEST_F(ListCommandsTest, Lindex_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"LINDEX", "long_list"}; // Missing index
    std::string result = listCommands->cmdLindex(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Test LSET command with valid index
TEST_F(ListCommandsTest, Lset_ValidIndex_UpdatesElement) {
    std::vector<std::string> args = {"LSET", "long_list", "1", "new_value"};
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_EQ("+OK\r\n", result);
    
    // Verify element was updated
    RedisValue* value = database->getValue("long_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        auto it = value->list_value.begin();
        std::advance(it, 1);
        EXPECT_EQ("new_value", *it);
    }
}

// Test LSET command with negative index
TEST_F(ListCommandsTest, Lset_NegativeIndex_UpdatesElement) {
    std::vector<std::string> args = {"LSET", "long_list", "-1", "last_value"};
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_EQ("+OK\r\n", result);
    
    // Verify last element was updated
    RedisValue* value = database->getValue("long_list");
    EXPECT_TRUE(value != nullptr);
    if (value && value->type == RedisType::LIST) {
        auto it = value->list_value.end();
        std::advance(it, -1);
        EXPECT_EQ("last_value", *it);
    }
}

// Test LSET command with out of bounds index
TEST_F(ListCommandsTest, Lset_OutOfBounds_ReturnsError) {
    std::vector<std::string> args = {"LSET", "long_list", "10", "value"};
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_TRUE(result.find("ERR index out of range") != std::string::npos);
}

// Test LSET command with non-existent list
TEST_F(ListCommandsTest, Lset_NonExistentList_ReturnsError) {
    std::vector<std::string> args = {"LSET", "non_existent_list", "0", "value"};
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_TRUE(result.find("ERR no such key") != std::string::npos);
}

// Test LSET command with wrong type
TEST_F(ListCommandsTest, Lset_WrongType_ReturnsError) {
    std::vector<std::string> args = {"LSET", "string_key", "0", "value"};
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_TRUE(result.find("ERR no such key") != std::string::npos);
}

// Test LSET command with non-integer index
TEST_F(ListCommandsTest, Lset_NonIntegerIndex_ReturnsError) {
    std::vector<std::string> args = {"LSET", "long_list", "invalid", "value"};
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_TRUE(result.find("ERR value is not an integer or out of range") != std::string::npos);
}

// Test LSET command with wrong number of arguments
TEST_F(ListCommandsTest, Lset_WrongNumberOfArguments_ReturnsError) {
    std::vector<std::string> args = {"LSET", "long_list", "0"}; // Missing value
    std::string result = listCommands->cmdLset(args);
    
    EXPECT_TRUE(result.find("ERR wrong number of arguments") != std::string::npos);
}

// Integration test: Multiple list operations
TEST_F(ListCommandsTest, Integration_MultipleListOperations) {
    // Create a list with LPUSH
    std::vector<std::string> lpush_args = {"LPUSH", "integration_list", "c", "b", "a"};
    std::string lpush_result = listCommands->cmdLpush(lpush_args);
    EXPECT_EQ(":3\r\n", lpush_result);
    
    // Add to end with RPUSH
    std::vector<std::string> rpush_args = {"RPUSH", "integration_list", "d", "e"};
    std::string rpush_result = listCommands->cmdRpush(rpush_args);
    EXPECT_EQ(":5\r\n", rpush_result);
    
    // Check length with LLEN
    std::vector<std::string> llen_args = {"LLEN", "integration_list"};
    std::string llen_result = listCommands->cmdLlen(llen_args);
    EXPECT_EQ(":5\r\n", llen_result);
    
    // Get range with LRANGE
    std::vector<std::string> lrange_args = {"LRANGE", "integration_list", "0", "-1"};
    std::string lrange_result = listCommands->cmdLrange(lrange_args);
    EXPECT_TRUE(lrange_result.find("a") != std::string::npos);
    EXPECT_TRUE(lrange_result.find("b") != std::string::npos);
    EXPECT_TRUE(lrange_result.find("c") != std::string::npos);
    EXPECT_TRUE(lrange_result.find("d") != std::string::npos);
    EXPECT_TRUE(lrange_result.find("e") != std::string::npos);
    
    // Get specific element with LINDEX
    std::vector<std::string> lindex_args = {"LINDEX", "integration_list", "2"};
    std::string lindex_result = listCommands->cmdLindex(lindex_args);
    EXPECT_EQ("$1\r\nc\r\n", lindex_result);
    
    // Update element with LSET
    std::vector<std::string> lset_args = {"LSET", "integration_list", "2", "C"};
    std::string lset_result = listCommands->cmdLset(lset_args);
    EXPECT_EQ("+OK\r\n", lset_result);
    
    // Pop from front with LPOP
    std::vector<std::string> lpop_args = {"LPOP", "integration_list"};
    std::string lpop_result = listCommands->cmdLpop(lpop_args);
    EXPECT_EQ("$1\r\na\r\n", lpop_result);
    
    // Pop from end with RPOP
    std::vector<std::string> rpop_args = {"RPOP", "integration_list"};
    std::string rpop_result = listCommands->cmdRpop(rpop_args);
    EXPECT_EQ("$1\r\ne\r\n", rpop_result);
    
    // Final length
    llen_result = listCommands->cmdLlen(llen_args);
    EXPECT_EQ(":3\r\n", llen_result);
}

// Test edge case: Large list operations
TEST_F(ListCommandsTest, EdgeCase_LargeListOperations) {
    // Add many elements with RPUSH
    std::vector<std::string> rpush_args = {"RPUSH", "large_list"};
    for (int i = 0; i < 100; i++) {
        rpush_args.push_back("item_" + std::to_string(i));
    }
    
    std::string result = listCommands->cmdRpush(rpush_args);
    EXPECT_EQ(":100\r\n", result);
    
    // Check length
    std::vector<std::string> llen_args = {"LLEN", "large_list"};
    std::string llen_result = listCommands->cmdLlen(llen_args);
    EXPECT_EQ(":100\r\n", llen_result);
    
    // Get range
    std::vector<std::string> lrange_args = {"LRANGE", "large_list", "0", "99"};
    std::string lrange_result = listCommands->cmdLrange(lrange_args);
    EXPECT_TRUE(lrange_result.find("item_0") != std::string::npos);
    EXPECT_TRUE(lrange_result.find("item_99") != std::string::npos);
    
    // Pop from both ends
    listCommands->cmdLpop({"LPOP", "large_list"});
    listCommands->cmdRpop({"RPOP", "large_list"});
    
    // Final length
    llen_result = listCommands->cmdLlen(llen_args);
    EXPECT_EQ(":98\r\n", llen_result);
}

// Test edge case: Empty string values
TEST_F(ListCommandsTest, EdgeCase_EmptyStringValues) {
    std::vector<std::string> args = {"RPUSH", "empty_strings_list", "", "value", ""};
    std::string result = listCommands->cmdRpush(args);
    EXPECT_EQ(":3\r\n", result);
    
    std::vector<std::string> lrange_args = {"LRANGE", "empty_strings_list", "0", "-1"};
    std::string lrange_result = listCommands->cmdLrange(lrange_args);
    EXPECT_TRUE(lrange_result.find("") != std::string::npos); // Empty strings should be handled
    
    std::vector<std::string> lindex_args = {"LINDEX", "empty_strings_list", "0"};
    std::string lindex_result = listCommands->cmdLindex(lindex_args);
    EXPECT_EQ("$0\r\n\r\n", lindex_result); // Empty bulk string
}