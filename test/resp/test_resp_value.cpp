#include <gtest/gtest.h>
#include "resp/resp_value.h"

class RESPValueTest : public ::testing::Test {};

// Test default constructor
TEST_F(RESPValueTest, DefaultConstructor) {
    RESPValue value;
    
    EXPECT_EQ(value.type, RESPType::NULL_VALUE);
    EXPECT_TRUE(value.isNull());
    EXPECT_FALSE(value.isString());
    EXPECT_FALSE(value.isInteger());
    EXPECT_FALSE(value.isBulkString());
    EXPECT_FALSE(value.isArray());
    EXPECT_FALSE(value.isError());
}

// Test constructor with type only
TEST_F(RESPValueTest, TypeConstructor) {
    // Test NULL_VALUE
    RESPValue nullValue(RESPType::NULL_VALUE);
    EXPECT_EQ(nullValue.type, RESPType::NULL_VALUE);
    EXPECT_TRUE(nullValue.isNull());
    
    // Test INTEGER
    RESPValue intValue(RESPType::INTEGER);
    EXPECT_EQ(intValue.type, RESPType::INTEGER);
    EXPECT_TRUE(intValue.isInteger());
    EXPECT_EQ(intValue.getInteger(), 0);
    
    // Test ARRAY
    RESPValue arrayValue(RESPType::ARRAY);
    EXPECT_EQ(arrayValue.type, RESPType::ARRAY);
    EXPECT_TRUE(arrayValue.isArray());
    EXPECT_TRUE(arrayValue.getArray().empty());
}

// Test string constructor
TEST_F(RESPValueTest, StringConstructor) {
    std::string testString = "Hello World";
    
    // Test SIMPLE_STRING
    RESPValue simpleString(RESPType::SIMPLE_STRING, testString);
    EXPECT_EQ(simpleString.type, RESPType::SIMPLE_STRING);
    EXPECT_TRUE(simpleString.isString());
    EXPECT_EQ(simpleString.getString(), testString);
    
    // Test ERROR
    RESPValue errorValue(RESPType::ERROR, "Error message");
    EXPECT_EQ(errorValue.type, RESPType::ERROR);
    EXPECT_TRUE(errorValue.isError());
    EXPECT_TRUE(errorValue.isString());
    EXPECT_EQ(errorValue.getString(), "Error message");
    
    // Test BULK_STRING
    RESPValue bulkString(RESPType::BULK_STRING, "Bulk string content");
    EXPECT_EQ(bulkString.type, RESPType::BULK_STRING);
    EXPECT_TRUE(bulkString.isBulkString());
    EXPECT_TRUE(bulkString.isString());
    EXPECT_EQ(bulkString.getString(), "Bulk string content");
}

// Test integer constructor
TEST_F(RESPValueTest, IntegerConstructor) {
    long long testInt = 12345;
    RESPValue intValue(RESPType::INTEGER, testInt);
    
    EXPECT_EQ(intValue.type, RESPType::INTEGER);
    EXPECT_TRUE(intValue.isInteger());
    EXPECT_FALSE(intValue.isString());
    EXPECT_FALSE(intValue.isArray());
    EXPECT_FALSE(intValue.isNull());
    EXPECT_EQ(intValue.getInteger(), testInt);
}

// Test negative integer
TEST_F(RESPValueTest, NegativeInteger) {
    long long negativeInt = -9876;
    RESPValue intValue(RESPType::INTEGER, negativeInt);
    
    EXPECT_EQ(intValue.getInteger(), negativeInt);
}

// Test array constructor
TEST_F(RESPValueTest, ArrayConstructor) {
    std::vector<RESPValue> testArray = {
        RESPValue(RESPType::SIMPLE_STRING, "item1"),
        RESPValue(RESPType::INTEGER, 42),
        RESPValue(RESPType::BULK_STRING, "bulk item")
    };
    
    RESPValue arrayValue(RESPType::ARRAY, testArray);
    
    EXPECT_EQ(arrayValue.type, RESPType::ARRAY);
    EXPECT_TRUE(arrayValue.isArray());
    EXPECT_FALSE(arrayValue.isString());
    EXPECT_FALSE(arrayValue.isInteger());
    
    const auto& returnedArray = arrayValue.getArray();
    EXPECT_EQ(returnedArray.size(), 3);
    EXPECT_TRUE(returnedArray[0].isString());
    EXPECT_EQ(returnedArray[0].getString(), "item1");
    EXPECT_TRUE(returnedArray[1].isInteger());
    EXPECT_EQ(returnedArray[1].getInteger(), 42);
    EXPECT_TRUE(returnedArray[2].isBulkString());
    EXPECT_EQ(returnedArray[2].getString(), "bulk item");
}

// Test nullopt constructor
TEST_F(RESPValueTest, NulloptConstructor) {
    RESPValue nullValue(RESPType::NULL_VALUE, std::nullopt);
    
    EXPECT_EQ(nullValue.type, RESPType::NULL_VALUE);
    EXPECT_TRUE(nullValue.isNull());
    EXPECT_FALSE(nullValue.isString());
    EXPECT_FALSE(nullValue.isInteger());
}

// Test empty array
TEST_F(RESPValueTest, EmptyArray) {
    std::vector<RESPValue> emptyArray;
    RESPValue arrayValue(RESPType::ARRAY, emptyArray);
    
    EXPECT_TRUE(arrayValue.isArray());
    EXPECT_TRUE(arrayValue.getArray().empty());
}

// Test getString with non-string type
TEST_F(RESPValueTest, GetStringFromNonString) {
    RESPValue intValue(RESPType::INTEGER, 123);
    
    // Should return empty string when data is not string type
    EXPECT_EQ(intValue.getString(), "");
}

// Test getInteger with non-integer type
TEST_F(RESPValueTest, GetIntegerFromNonInteger) {
    RESPValue stringValue(RESPType::SIMPLE_STRING, "not a number");
    
    // Should return 0 when data is not integer type
    EXPECT_EQ(stringValue.getInteger(), 0);
}

// Test getArray with non-array type
TEST_F(RESPValueTest, GetArrayFromNonArray) {
    RESPValue stringValue(RESPType::SIMPLE_STRING, "not an array");
    
    // This should throw std::bad_variant_access, so we need to catch it
    try {
        stringValue.getArray();
        FAIL() << "Expected std::bad_variant_access exception";
    } catch (const std::bad_variant_access&) {
        // Expected behavior
        SUCCEED();
    } catch (...) {
        FAIL() << "Expected std::bad_variant_access, but got different exception";
    }
}

// Test type checking methods comprehensively
TEST_F(RESPValueTest, TypeCheckingMethods) {
    // Test SIMPLE_STRING
    RESPValue simpleString(RESPType::SIMPLE_STRING, "test");
    EXPECT_TRUE(simpleString.isString());
    EXPECT_FALSE(simpleString.isInteger());
    EXPECT_FALSE(simpleString.isBulkString());
    EXPECT_FALSE(simpleString.isArray());
    EXPECT_FALSE(simpleString.isNull());
    EXPECT_FALSE(simpleString.isError());
    
    // Test BULK_STRING
    RESPValue bulkString(RESPType::BULK_STRING, "test");
    EXPECT_TRUE(bulkString.isString());
    EXPECT_TRUE(bulkString.isBulkString());
    EXPECT_FALSE(bulkString.isInteger());
    EXPECT_FALSE(bulkString.isArray());
    EXPECT_FALSE(bulkString.isNull());
    EXPECT_FALSE(bulkString.isError());
    
    // Test ERROR
    RESPValue errorValue(RESPType::ERROR, "error");
    EXPECT_TRUE(errorValue.isString());
    EXPECT_TRUE(errorValue.isError());
    EXPECT_FALSE(errorValue.isInteger());
    EXPECT_FALSE(errorValue.isBulkString());
    EXPECT_FALSE(errorValue.isArray());
    EXPECT_FALSE(errorValue.isNull());
    
    // Test INTEGER
    RESPValue intValue(RESPType::INTEGER, 123);
    EXPECT_TRUE(intValue.isInteger());
    EXPECT_FALSE(intValue.isString());
    EXPECT_FALSE(intValue.isBulkString());
    EXPECT_FALSE(intValue.isArray());
    EXPECT_FALSE(intValue.isNull());
    EXPECT_FALSE(intValue.isError());
    
    // Test ARRAY
    RESPValue arrayValue(RESPType::ARRAY, std::vector<RESPValue>());
    EXPECT_TRUE(arrayValue.isArray());
    EXPECT_FALSE(arrayValue.isString());
    EXPECT_FALSE(arrayValue.isInteger());
    EXPECT_FALSE(arrayValue.isBulkString());
    EXPECT_FALSE(arrayValue.isNull());
    EXPECT_FALSE(arrayValue.isError());
    
    // Test NULL_VALUE
    RESPValue nullValue(RESPType::NULL_VALUE);
    EXPECT_TRUE(nullValue.isNull());
    EXPECT_FALSE(nullValue.isString());
    EXPECT_FALSE(nullValue.isInteger());
    EXPECT_FALSE(nullValue.isBulkString());
    EXPECT_FALSE(nullValue.isArray());
    EXPECT_FALSE(nullValue.isError());
}

// Test edge cases for string values
TEST_F(RESPValueTest, StringEdgeCases) {
    // Empty string
    RESPValue emptyString(RESPType::SIMPLE_STRING, "");
    EXPECT_EQ(emptyString.getString(), "");
    
    // String with special characters
    std::string specialChars = "Line1\nLine2\tTab\rReturn";
    RESPValue specialString(RESPType::BULK_STRING, specialChars);
    EXPECT_EQ(specialString.getString(), specialChars);
}

// Test large integer values
TEST_F(RESPValueTest, LargeIntegerValues) {
    long long largePositive = 9223372036854775807LL; // LLONG_MAX
    RESPValue largePosValue(RESPType::INTEGER, largePositive);
    EXPECT_EQ(largePosValue.getInteger(), largePositive);
    
    long long largeNegative = -9223372036854775807LL;
    RESPValue largeNegValue(RESPType::INTEGER, largeNegative);
    EXPECT_EQ(largeNegValue.getInteger(), largeNegative);
}

// Test nested arrays
TEST_F(RESPValueTest, NestedArrays) {
    std::vector<RESPValue> innerArray = {
        RESPValue(RESPType::INTEGER, 1),
        RESPValue(RESPType::INTEGER, 2)
    };
    
    std::vector<RESPValue> outerArray = {
        RESPValue(RESPType::SIMPLE_STRING, "nested"),
        RESPValue(RESPType::ARRAY, innerArray)
    };
    
    RESPValue nestedArrayValue(RESPType::ARRAY, outerArray);
    
    EXPECT_TRUE(nestedArrayValue.isArray());
    const auto& returnedArray = nestedArrayValue.getArray();
    EXPECT_EQ(returnedArray.size(), 2);
    EXPECT_TRUE(returnedArray[1].isArray());
    
    const auto& nested = returnedArray[1].getArray();
    EXPECT_EQ(nested.size(), 2);
    EXPECT_EQ(nested[0].getInteger(), 1);
    EXPECT_EQ(nested[1].getInteger(), 2);
}