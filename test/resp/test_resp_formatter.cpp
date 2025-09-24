#include <gtest/gtest.h>
#include "resp/resp_formatter.h"

class RESPFormatterTest : public ::testing::Test {};

// Test formatError function
TEST_F(RESPFormatterTest, FormatError) {
    std::string message = "Error message";
    std::string result = RESPFormatter::formatError(message);
    EXPECT_EQ(result, "-Error message\r\n");
    
    // Test with empty string
    std::string empty_result = RESPFormatter::formatError("");
    EXPECT_EQ(empty_result, "-\r\n");
    
    // Test with special characters
    std::string special_result = RESPFormatter::formatError("Error: something went wrong");
    EXPECT_EQ(special_result, "-Error: something went wrong\r\n");
}

// Test formatSimpleString function
TEST_F(RESPFormatterTest, FormatSimpleString) {
    std::string str = "OK";
    std::string result = RESPFormatter::formatSimpleString(str);
    EXPECT_EQ(result, "+OK\r\n");
    
    // Test with empty string
    std::string empty_result = RESPFormatter::formatSimpleString("");
    EXPECT_EQ(empty_result, "+\r\n");
    
    // Test with spaces
    std::string space_result = RESPFormatter::formatSimpleString("Hello World");
    EXPECT_EQ(space_result, "+Hello World\r\n");
}

// Test formatBulkString function
TEST_F(RESPFormatterTest, FormatBulkString) {
    std::string str = "Hello";
    std::string result = RESPFormatter::formatBulkString(str);
    EXPECT_EQ(result, "$5\r\nHello\r\n");
    
    // Test with empty string
    std::string empty_result = RESPFormatter::formatBulkString("");
    EXPECT_EQ(empty_result, "$0\r\n\r\n");
    
    // Test with longer string
    std::string long_result = RESPFormatter::formatBulkString("This is a longer string");
    EXPECT_EQ(long_result, "$23\r\nThis is a longer string\r\n");
    
    // Test with special characters
    std::string special_result = RESPFormatter::formatBulkString("Line1\nLine2\r\n");
    EXPECT_NE(special_result, "$12\r\nLine1\nLine2\r\n\r\n");
}

// Test formatInteger function
TEST_F(RESPFormatterTest, FormatInteger) {
    long long value = 42;
    std::string result = RESPFormatter::formatInteger(value);
    EXPECT_EQ(result, ":42\r\n");
    
    // Test with zero
    std::string zero_result = RESPFormatter::formatInteger(0);
    EXPECT_EQ(zero_result, ":0\r\n");
    
    // Test with negative number
    std::string negative_result = RESPFormatter::formatInteger(-123);
    EXPECT_EQ(negative_result, ":-123\r\n");
    
    // Test with large number
    std::string large_result = RESPFormatter::formatInteger(1234567890);
    EXPECT_EQ(large_result, ":1234567890\r\n");
}

// Test formatNull function
TEST_F(RESPFormatterTest, FormatNull) {
    std::string result = RESPFormatter::formatNull();
    EXPECT_EQ(result, "$-1\r\n");
}

// Test formatArray function
TEST_F(RESPFormatterTest, FormatArray) {
    // Test with empty array
    std::vector<std::string> empty_array;
    std::string empty_result = RESPFormatter::formatArray(empty_array);
    EXPECT_EQ(empty_result, "*0\r\n");
    
    // Test with single element
    std::vector<std::string> single_array = {"Hello"};
    std::string single_result = RESPFormatter::formatArray(single_array);
    EXPECT_EQ(single_result, "*1\r\n$5\r\nHello\r\n");
    
    // Test with multiple elements
    std::vector<std::string> multi_array = {"Hello", "World", "!"};
    std::string multi_result = RESPFormatter::formatArray(multi_array);
    EXPECT_EQ(multi_result, "*3\r\n$5\r\nHello\r\n$5\r\nWorld\r\n$1\r\n!\r\n");

    // Test with empty strings in array
    std::vector<std::string> empty_strings_array = {"Hello", "", "World"};
    std::string empty_strings_result = RESPFormatter::formatArray(empty_strings_array);
                                    
    EXPECT_EQ(empty_strings_result, "*3\r\n$5\r\nHello\r\n$0\r\n\r\n$5\r\nWorld\r\n");

    // Test with null elements (empty strings treated as null in your implementation)
    std::vector<std::string> null_array = {"First", "", "Last"};
    std::string null_result = RESPFormatter::formatArray(null_array);
    EXPECT_EQ(null_result, "*3\r\n$5\r\nFirst\r\n$0\r\n\r\n$4\r\nLast\r\n");
}

// Test edge cases for formatArray
TEST_F(RESPFormatterTest, FormatArrayEdgeCases) {
    // Test with very long strings
    std::string long_string(100, 'A');
    std::vector<std::string> long_array = {long_string};
    std::string long_result = RESPFormatter::formatArray(long_array);
    EXPECT_EQ(long_result, "*1\r\n$100\r\n" + long_string + "\r\n");
    
    // Test with mixed types simulation
    std::vector<std::string> mixed_array = {"string", "123", ""};
    std::string mixed_result = RESPFormatter::formatArray(mixed_array);
    EXPECT_EQ(mixed_result, "*3\r\n$6\r\nstring\r\n$3\r\n123\r\n$0\r\n\r\n");
}

// Test RESP protocol compliance for various types
TEST_F(RESPFormatterTest, RESPProtocolCompliance) {
    // Error type should start with '-'
    EXPECT_EQ(RESPFormatter::formatError("test")[0], '-');
    
    // Simple string should start with '+'
    EXPECT_EQ(RESPFormatter::formatSimpleString("test")[0], '+');
    
    // Bulk string should start with '$'
    EXPECT_EQ(RESPFormatter::formatBulkString("test")[0], '$');
    
    // Integer should start with ':'
    EXPECT_EQ(RESPFormatter::formatInteger(1)[0], ':');
    
    // Array should start with '*'
    EXPECT_EQ(RESPFormatter::formatArray({"test"})[0], '*');
    
    // All should end with \r\n
    std::string error_result = RESPFormatter::formatError("test");
    EXPECT_EQ(error_result.substr(error_result.length() - 2), "\r\n");
    
    std::string string_result = RESPFormatter::formatSimpleString("test");
    EXPECT_EQ(string_result.substr(string_result.length() - 2), "\r\n");
    
    std::string bulk_result = RESPFormatter::formatBulkString("test");
    EXPECT_EQ(bulk_result.substr(bulk_result.length() - 2), "\r\n");
    
    std::string int_result = RESPFormatter::formatInteger(1);
    EXPECT_EQ(int_result.substr(int_result.length() - 2), "\r\n");
    
    std::string array_result = RESPFormatter::formatArray({"test"});
    EXPECT_EQ(array_result.substr(array_result.length() - 2), "\r\n");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}