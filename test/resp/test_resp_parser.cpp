#include <gtest/gtest.h>
#include "resp/resp_parser.h"
#include "resp/resp_value.h"

class RESPParserTest : public ::testing::Test {};

// Test Simple Strings
TEST_F(RESPParserTest, ParseSimpleString) {
    RESPValue result;
    size_t consumed;
    
    // Valid simple string
    EXPECT_TRUE(RESPParser::parse("+OK\r\n", result, consumed));
    EXPECT_EQ(consumed, 5);
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.getString(), "OK");
    
    // Empty simple string
    EXPECT_TRUE(RESPParser::parse("+\r\n", result, consumed));
    EXPECT_EQ(result.getString(), "");
    
    // Invalid - missing CRLF
    EXPECT_FALSE(RESPParser::parse("+OK", result, consumed));
}

// Test Errors
TEST_F(RESPParserTest, ParseError) {
    RESPValue result;
    size_t consumed;
    
    // Valid error
    EXPECT_TRUE(RESPParser::parse("-ERR unknown command\r\n", result, consumed));
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.getString(), "ERR unknown command");
    
    // Complex error message
    EXPECT_TRUE(RESPParser::parse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n", result, consumed));
    EXPECT_EQ(result.getString(), "WRONGTYPE Operation against a key holding the wrong kind of value");
}

// Test Integers
TEST_F(RESPParserTest, ParseInteger) {
    RESPValue result;
    size_t consumed;
    
    // Positive integer
    EXPECT_TRUE(RESPParser::parse(":1000\r\n", result, consumed));
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.getInteger(), 1000);
    
    // Negative integer
    EXPECT_TRUE(RESPParser::parse(":-123\r\n", result, consumed));
    EXPECT_EQ(result.getInteger(), -123);
    
    // Zero
    EXPECT_TRUE(RESPParser::parse(":0\r\n", result, consumed));
    EXPECT_EQ(result.getInteger(), 0);
    
    // Large integer
    EXPECT_TRUE(RESPParser::parse(":9223372036854775807\r\n", result, consumed));
    EXPECT_EQ(result.getInteger(), 9223372036854775807LL);
    
    // Invalid integer format
    EXPECT_FALSE(RESPParser::parse(":abc\r\n", result, consumed));
}

// Test Bulk Strings
TEST_F(RESPParserTest, ParseBulkString) {
    RESPValue result;
    size_t consumed;
    
    // Valid bulk string
    EXPECT_TRUE(RESPParser::parse("$6\r\nfoobar\r\n", result, consumed));
    EXPECT_TRUE(result.isBulkString());
    EXPECT_EQ(result.getString(), "foobar");
    EXPECT_EQ(consumed, 12);
    
    // Empty bulk string
    EXPECT_TRUE(RESPParser::parse("$0\r\n\r\n", result, consumed));
    EXPECT_EQ(result.getString(), "");
    
    // NULL bulk string
    EXPECT_TRUE(RESPParser::parse("$-1\r\n", result, consumed));
    EXPECT_TRUE(result.isNull());
    
    // Bulk string with special characters
    EXPECT_TRUE(RESPParser::parse("$5\r\nhello\r\n", result, consumed));
    EXPECT_EQ(result.getString(), "hello");
    
    // Invalid - length mismatch
    EXPECT_FALSE(RESPParser::parse("$5\r\nabc\r\n", result, consumed));
    
    // Invalid - missing data
    EXPECT_FALSE(RESPParser::parse("$10\r\nshort\r\n", result, consumed));
}

// Test Arrays
TEST_F(RESPParserTest, ParseArray) {
    RESPValue result;
    size_t consumed;
    
    // Empty array
    EXPECT_TRUE(RESPParser::parse("*0\r\n", result, consumed));
    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.getArray().size(), 0);
    
    // Array with integers
    EXPECT_TRUE(RESPParser::parse("*3\r\n:1\r\n:2\r\n:3\r\n", result, consumed));
    EXPECT_TRUE(result.isArray());
    auto arr = result.getArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(arr[0].isInteger());
    EXPECT_EQ(arr[0].getInteger(), 1);
    EXPECT_EQ(arr[1].getInteger(), 2);
    EXPECT_EQ(arr[2].getInteger(), 3);
    
    // Array with mixed types
    EXPECT_TRUE(RESPParser::parse("*5\r\n:1\r\n+hello\r\n-world\r\n$5\r\nworld\r\n:-42\r\n", result, consumed));
    arr = result.getArray();
    EXPECT_EQ(arr.size(), 5);
    EXPECT_TRUE(arr[0].isInteger());
    EXPECT_TRUE(arr[1].isString());
    EXPECT_TRUE(arr[2].isError());
    EXPECT_TRUE(arr[3].isBulkString());
    EXPECT_TRUE(arr[4].isInteger());
    
    // NULL array
    EXPECT_TRUE(RESPParser::parse("*-1\r\n", result, consumed));
    EXPECT_TRUE(result.isNull());
    
    // Array with bulk strings
    EXPECT_TRUE(RESPParser::parse("*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n", result, consumed));
    arr = result.getArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].getString(), "foo");
    EXPECT_EQ(arr[1].getString(), "bar");
}

// Test Nested Arrays
TEST_F(RESPParserTest, ParseNestedArray) {
    RESPValue result;
    size_t consumed;
    
    // Simple nested array
    EXPECT_TRUE(RESPParser::parse("*2\r\n*2\r\n:1\r\n:2\r\n*2\r\n:3\r\n:4\r\n", result, consumed));
    EXPECT_TRUE(result.isArray());
    auto arr = result.getArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_TRUE(arr[0].isArray());
    EXPECT_TRUE(arr[1].isArray());
    
    auto inner1 = arr[0].getArray();
    auto inner2 = arr[1].getArray();
    EXPECT_EQ(inner1.size(), 2);
    EXPECT_EQ(inner2.size(), 2);
    EXPECT_EQ(inner1[0].getInteger(), 1);
    EXPECT_EQ(inner1[1].getInteger(), 2);
    EXPECT_EQ(inner2[0].getInteger(), 3);
    EXPECT_EQ(inner2[1].getInteger(), 4);
    
    // Complex nested array
    EXPECT_TRUE(RESPParser::parse("*2\r\n*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n*1\r\n+hello\r\n", result, consumed));
    arr = result.getArray();
    EXPECT_TRUE(arr[0].isArray());
    EXPECT_TRUE(arr[1].isArray());
}

// Test Error Cases
TEST_F(RESPParserTest, ErrorCases) {
    RESPValue result;
    size_t consumed;
    
    // Empty input
    EXPECT_FALSE(RESPParser::parse("", result, consumed));
    
    // Invalid type marker
    EXPECT_FALSE(RESPParser::parse("invalid\r\n", result, consumed));
    
    // Truncated input
    EXPECT_FALSE(RESPParser::parse("*2\r\n:1", result, consumed));
    
    // Invalid bulk string length
    EXPECT_FALSE(RESPParser::parse("$abc\r\n", result, consumed));
    
    // Bulk string length too large
    EXPECT_FALSE(RESPParser::parse("$1000000000\r\n", result, consumed));
    
    // Array with too many elements
    std::string largeArray = "*1001\r\n";
    for (int i = 0; i < 1001; ++i) {
        largeArray += ":1\r\n";
    }
    EXPECT_FALSE(RESPParser::parse(largeArray, result, consumed));
}

// Test toStringVector Utility
TEST_F(RESPParserTest, ToStringVector) {
    // Single string value
    RESPValue strVal(RESPType::BULK_STRING, "hello");
    auto vec1 = RESPParser::toStringVector(strVal);
    EXPECT_EQ(vec1.size(), 1);
    EXPECT_EQ(vec1[0], "hello");
    
    // Single integer value
    RESPValue intVal(RESPType::INTEGER, 42LL);
    auto vec2 = RESPParser::toStringVector(intVal);
    EXPECT_EQ(vec2.size(), 1);
    EXPECT_EQ(vec2[0], "42");
    
    // NULL value
    RESPValue nullVal(RESPType::NULL_VALUE);
    auto vec3 = RESPParser::toStringVector(nullVal);
    EXPECT_EQ(vec3.size(), 1);
    EXPECT_EQ(vec3[0], "");
    
    // Array with nested array (flattening)
    std::vector<RESPValue> nestedElements;
    nestedElements.push_back(RESPValue(RESPType::INTEGER, 1LL));
    nestedElements.push_back(RESPValue(RESPType::SIMPLE_STRING, "a"));
    
    std::vector<RESPValue> elements;
    elements.push_back(RESPValue(RESPType::INTEGER, 2LL));
    elements.push_back(RESPValue(RESPType::ARRAY, nestedElements));
    elements.push_back(RESPValue(RESPType::BULK_STRING, "b"));
    
    RESPValue arrayVal(RESPType::ARRAY, elements);
    auto vec4 = RESPParser::toStringVector(arrayVal);
    EXPECT_EQ(vec4.size(), 4); // 2 + 2 (flattened) elements
    EXPECT_EQ(vec4[0], "2");
    EXPECT_EQ(vec4[1], "1");
    EXPECT_EQ(vec4[2], "a");
    EXPECT_EQ(vec4[3], "b");
}

// Test Plain Text Parser
TEST_F(RESPParserTest, PlainTextParser) {
    auto result1 = parsePlainText("SET key value");
    EXPECT_EQ(result1.size(), 3);
    EXPECT_EQ(result1[0], "SET");
    EXPECT_EQ(result1[1], "key");
    EXPECT_EQ(result1[2], "value");
    
    auto result2 = parsePlainText("  MULTI   LINE  \r\nCOMMAND  ");
    EXPECT_EQ(result2.size(), 3);
    EXPECT_EQ(result2[0], "MULTI");
    EXPECT_EQ(result2[1], "LINE");
    EXPECT_EQ(result2[2], "COMMAND");
    
    auto result3 = parsePlainText("");
    EXPECT_TRUE(result3.empty());
}

// Test Edge Cases
TEST_F(RESPParserTest, EdgeCases) {
    RESPValue result;
    size_t consumed;
    
    // Bulk string with exact MAX_STRING_LENGTH (if we want to test boundary)
    // Note: This might be too large for practical testing
    
    // Array with null elements
    EXPECT_TRUE(RESPParser::parse("*3\r\n:1\r\n$-1\r\n:2\r\n", result, consumed));
    EXPECT_TRUE(result.isArray());
    auto arr = result.getArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(arr[0].isInteger());
    EXPECT_TRUE(arr[1].isNull());
    EXPECT_TRUE(arr[2].isInteger());
    
    // Very deep nesting (should fail due to MAX_DEPTH)
    std::string deepNested;
    for (int i = 0; i < 101; ++i) {
        deepNested += "*1\r\n";
    }
    deepNested += ":1\r\n";
    EXPECT_FALSE(RESPParser::parse(deepNested, result, consumed));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}