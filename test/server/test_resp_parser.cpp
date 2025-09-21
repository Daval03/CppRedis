#include <gtest/gtest.h>
#include "../../src/server/resp_parser.h"

class RESPParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ===== PARSING TESTS =====

TEST_F(RESPParserTest, ParseSimpleArray) {
    std::string input = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";
    auto result = RESPParser::parse(input);
   
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], "key");
}

TEST_F(RESPParserTest, ParseEmptyArray) {
    std::string input = "*0\r\n";
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 0);
}

// MISSING: Test for null array (*-1\r\n)
TEST_F(RESPParserTest, ParseNullArray) {
    std::string input = "*-1\r\n";
    // Your parser might need to handle this - check implementation
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// MISSING: Test for null bulk strings
TEST_F(RESPParserTest, ParseNullBulkString) {
    std::string input = "*2\r\n$3\r\nGET\r\n$-1\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], ""); // Your parser returns empty string for null
}

// MISSING: Test binary data (with null bytes)
TEST_F(RESPParserTest, ParseBinaryData) {
    std::string binary_data = "hello\0world";
    binary_data = std::string("hello\0world", 11); // Explicitly include null byte
    std::string input = "*1\r\n$11\r\n" + binary_data + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 11);
    EXPECT_EQ(result[0], binary_data);
}

// MISSING: Test length validation
TEST_F(RESPParserTest, ParseStringLengthMismatch) {
    // String says 5 bytes but only provides 3
    std::string input = "*1\r\n$5\r\nhi\r\n";
    // This should be an error, not just a warning
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// MISSING: Test maximum limits
TEST_F(RESPParserTest, ParseTooManyArguments) {
    // Test MAX_ARGS limit (100 in your implementation)
    std::string input = "*101\r\n"; // One more than MAX_ARGS
    for (int i = 0; i < 101; i++) {
        input += "$1\r\na\r\n";
    }
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// MISSING: Test telnet fallback
TEST_F(RESPParserTest, ParsePlainTextCommand) {
    std::string input = "GET mykey";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], "mykey");
}

TEST_F(RESPParserTest, ParsePlainTextWithNewlines) {
    std::string input = "SET key value\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "SET");
    EXPECT_EQ(result[1], "key");
    EXPECT_EQ(result[2], "value");
}

// Enhanced error testing
TEST_F(RESPParserTest, ParseInvalidArrayHeader) {
    std::string input = "*2invalid\r\n";
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseMissingCRLF) {
    std::string input = "*2\n$3\nGET\n$3\nkey\n"; // Missing \r
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseTruncatedInput) {
    std::string input = "*2\r\n$3\r\nGET\r\n$3\r\nke"; // Truncated
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseNegativeArraySize) {
    std::string input = "*-2\r\n"; // Invalid negative size (not -1)
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseMissingBulkStringHeader) {
    std::string input = "*1\r\n"; // Missing bulk string
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseInvalidBulkStringLength) {
    std::string input = "*1\r\n$abc\r\nhello\r\n"; // Invalid length
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseEmptyInput) {
    std::string input = "";
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// ===== FORMATTING TESTS =====

TEST_F(RESPParserTest, FormatBulkString) {
    std::string result = RESPParser::formatBulkString("hello");
    EXPECT_EQ(result, "$5\r\nhello\r\n");
}

TEST_F(RESPParserTest, FormatEmptyBulkString) {
    std::string result = RESPParser::formatBulkString("");
    EXPECT_EQ(result, "$-1\r\n"); // Your implementation returns NULL for empty
}

TEST_F(RESPParserTest, FormatBulkStringWithBinaryData) {
    std::string binary_data = std::string("hello\0world", 11);
    std::string result = RESPParser::formatBulkString(binary_data);
    EXPECT_EQ(result, "$11\r\nhello\0world\r\n");
}

TEST_F(RESPParserTest, FormatInteger) {
    std::string result = RESPParser::formatInteger(42);
    EXPECT_EQ(result, ":42\r\n");
}

TEST_F(RESPParserTest, FormatNegativeInteger) {
    std::string result = RESPParser::formatInteger(-42);
    EXPECT_EQ(result, ":-42\r\n");
}

TEST_F(RESPParserTest, FormatZeroInteger) {
    std::string result = RESPParser::formatInteger(0);
    EXPECT_EQ(result, ":0\r\n");
}

TEST_F(RESPParserTest, FormatSimpleString) {
    std::string result = RESPParser::formatSimpleString("OK");
    EXPECT_EQ(result, "+OK\r\n");
}

TEST_F(RESPParserTest, FormatError) {
    std::string result = RESPParser::formatError("ERR unknown command");
    EXPECT_EQ(result, "-ERR unknown command\r\n");
}

TEST_F(RESPParserTest, FormatArray) {
    std::vector<std::string> items = {"SET", "key", "value"};
    std::string result = RESPParser::formatArray(items);
    EXPECT_EQ(result, "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
}

TEST_F(RESPParserTest, FormatEmptyArray) {
    std::vector<std::string> items = {};
    std::string result = RESPParser::formatArray(items);
    EXPECT_EQ(result, "*0\r\n");
}

TEST_F(RESPParserTest, FormatNull) {
    std::string result = RESPParser::formatNull();
    EXPECT_EQ(result, "$-1\r\n");
}

// ===== EDGE CASES - CRITICAL TESTS =====

TEST_F(RESPParserTest, ParseStringWithCarriageReturn) {
    // Test string containing \r character
    std::string input = "*1\r\n$6\r\nhello\rworld\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello\rworld"); // Should preserve \r inside data
}

TEST_F(RESPParserTest, ParseStringWithNewline) {
    // Test string containing \n character
    std::string input = "*1\r\n$6\r\nhello\nworld\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello\nworld"); // Should preserve \n inside data
}

TEST_F(RESPParserTest, ParseStringWithCRLF) {
    // Test string containing \r\n sequence
    std::string input = "*1\r\n$7\r\nhello\r\nworld\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello\r\nworld"); // Should preserve \r\n inside data
}

TEST_F(RESPParserTest, ParseMultipleNullBytes) {
    // Test string with multiple null bytes
    std::string binary_data = std::string("a\0b\0c\0d", 7);
    std::string input = "*1\r\n$7\r\n" + binary_data + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 7);
    EXPECT_EQ(result[0], binary_data);
}

TEST_F(RESPParserTest, ParseStringStartingWithNull) {
    // Test string starting with null byte
    std::string binary_data = std::string("\0hello", 6);
    std::string input = "*1\r\n$6\r\n" + binary_data + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 6);
    EXPECT_EQ(result[0], binary_data);
}

TEST_F(RESPParserTest, ParseStringEndingWithNull) {
    // Test string ending with null byte
    std::string binary_data = std::string("hello\0", 6);
    std::string input = "*1\r\n$6\r\n" + binary_data + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 6);
    EXPECT_EQ(result[0], binary_data);
}

TEST_F(RESPParserTest, ParseOnlyNullBytes) {
    // Test string containing only null bytes
    std::string binary_data = std::string("\0\0\0", 3);
    std::string input = "*1\r\n$3\r\n" + binary_data + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 3);
    EXPECT_EQ(result[0], binary_data);
}

TEST_F(RESPParserTest, ParseUTF8String) {
    // Test UTF-8 encoded string
    std::string utf8_string = "Hello 世界 🌍";
    std::string input = "*1\r\n$" + std::to_string(utf8_string.length()) + "\r\n" + utf8_string + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], utf8_string);
}

TEST_F(RESPParserTest, ParseControlCharacters) {
    // Test string with various control characters
    std::string control_chars = "\x01\x02\x03\x1F\x7F";
    std::string input = "*1\r\n$5\r\n" + control_chars + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], control_chars);
}

TEST_F(RESPParserTest, ParseHighBitCharacters) {
    // Test string with high-bit characters (0x80-0xFF)
    std::string high_bits = "\x80\x90\xA0\xF0\xFF";
    std::string input = "*1\r\n$5\r\n" + high_bits + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], high_bits);
}

// MALFORMED INPUT EDGE CASES
TEST_F(RESPParserTest, ParsePartialCRLF_OnlyR) {
    // Input with only \r but no \n
    std::string input = "*1\r$5\rhello\r";
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParsePartialCRLF_OnlyN) {
    // Input with only \n but no \r
    std::string input = "*1\n$5\nhello\n";
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseReversedCRLF) {
    // Input with \n\r instead of \r\n
    std::string input = "*1\n\r$5\n\rhello\n\r";
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseExtraData) {
    // Valid RESP followed by extra garbage
    std::string input = "*1\r\n$5\r\nhello\r\nextra_garbage_here";
    auto result = RESPParser::parse(input);
    
    // Should parse successfully and ignore extra data
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

TEST_F(RESPParserTest, ParseZeroLengthString) {
    // Bulk string with length 0 (different from null)
    std::string input = "*1\r\n$0\r\n\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], ""); // Should be empty string, not null
}

TEST_F(RESPParserTest, ParseVeryLargeValidLength) {
    // Test with length at the boundary of MAX_STRING_LENGTH
    const size_t max_len = 1024 * 1024; // 1MB from your MAX_STRING_LENGTH
    std::string large_string(max_len, 'x');
    std::string input = "*1\r\n$" + std::to_string(max_len) + "\r\n" + large_string + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), max_len);
}

TEST_F(RESPParserTest, ParseStringLengthExceedsLimit) {
    // Test with length exceeding MAX_STRING_LENGTH
    const size_t over_limit = 1024 * 1024 + 1; // 1MB + 1
    std::string input = "*1\r\n$" + std::to_string(over_limit) + "\r\n";
    
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseMaxArguments) {
    // Test with exactly MAX_ARGS (should succeed)
    const int max_args = 100;
    std::string input = "*" + std::to_string(max_args) + "\r\n";
    for (int i = 0; i < max_args; i++) {
        input += "$1\r\na\r\n";
    }
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), max_args);
}

TEST_F(RESPParserTest, ParseExcessiveArguments) {
    // Test with MAX_ARGS + 1 (should fail)
    const int over_limit = 101;
    std::string input = "*" + std::to_string(over_limit) + "\r\n";
    
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// TELNET FALLBACK EDGE CASES
TEST_F(RESPParserTest, ParseTelnetWithMultipleSpaces) {
    std::string input = "SET   key    value   ";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "SET");
    EXPECT_EQ(result[1], "key");
    EXPECT_EQ(result[2], "value");
}

TEST_F(RESPParserTest, ParseTelnetWithTabs) {
    std::string input = "GET\tkey\tvalue";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], "key");
    EXPECT_EQ(result[2], "value");
}

TEST_F(RESPParserTest, ParseTelnetWithMixedWhitespace) {
    std::string input = " \t SET \t key \r\n value \t ";
    auto result = RESPParser::parse(input);
    
    EXPECT_GE(result.size(), 3); // Should have at least SET, key, value
    EXPECT_EQ(result[0], "SET");
    EXPECT_EQ(result[1], "key");
    EXPECT_EQ(result[2], "value");
}

TEST_F(RESPParserTest, ParseTelnetEmptyCommand) {
    std::string input = "   \r\n  ";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 0); // Should be empty after filtering
}

// INTEGER BOUNDARY TESTS
TEST_F(RESPParserTest, ParseLargeArraySize) {
    // Test with valid but large array size
    std::string input = "*50\r\n";
    for (int i = 0; i < 50; i++) {
        input += "$4\r\ntest\r\n";
    }
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 50);
}

TEST_F(RESPParserTest, ParseArraySizeIntegerOverflow) {
    // Test with number that might cause integer overflow
    std::string input = "*999999999999999999999\r\n";
    
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

TEST_F(RESPParserTest, ParseBulkStringLengthOverflow) {
    // Test with bulk string length that might overflow
    std::string input = "*1\r\n$999999999999999999999\r\nhello\r\n";
    
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// NESTED SCENARIOS
TEST_F(RESPParserTest, ParseCommandWithSpecialCharacters) {
    // Redis commands can have complex arguments
    std::string input = "*3\r\n$4\r\nEVAL\r\n$25\r\nreturn redis.call('get', KEYS[1])\r\n$1\r\n1\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "EVAL");
    EXPECT_EQ(result[1], "return redis.call('get', KEYS[1])");
    EXPECT_EQ(result[2], "1");
}

TEST_F(RESPParserTest, ParseCommandWithJSON) {
    // Test with JSON-like string
    std::string json = R"({"key": "value", "nested": {"inner": true}})";
    std::string input = "*2\r\n$3\r\nSET\r\n$" + std::to_string(json.length()) + "\r\n" + json + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "SET");
    EXPECT_EQ(result[1], json);
}

// ===== ROUND-TRIP TESTS =====

TEST_F(RESPParserTest, RoundTripFormatParse) {
    std::vector<std::string> original = {"GET", "mykey", "value"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithEmptyStringsSize) {
    std::vector<std::string> original = {"SET", "", "value"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    
    // Note: Your parser converts empty strings to NULL bulk strings
    // This test might fail depending on your implementation
    EXPECT_EQ(parsed.size(), original.size());
}

TEST_F(RESPParserTest, RoundTripWithBinaryData) {
    std::vector<std::string> original = {"SET", "key", std::string("binary\0data", 11)};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithControlCharacters) {
    std::vector<std::string> original = {"SET", "key\r\nwith\tcrlf", "value\x01\x02"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithUTF8) {
    std::vector<std::string> original = {"SET", "🔑", "世界"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithEmptyStrings) {
    // Note: This test might reveal the null handling issue in your parser
    std::vector<std::string> original = {"SET", "", "value"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    
    EXPECT_EQ(parsed.size(), original.size());
    EXPECT_EQ(parsed[0], original[0]); // "SET"
    // This might fail if your parser converts empty strings to nulls
    EXPECT_EQ(parsed[1], original[1]); // "" 
    EXPECT_EQ(parsed[2], original[2]); // "value"
}

TEST_F(RESPParserTest, RoundTripStressTest) {
    // Large round-trip test with various edge cases
    std::vector<std::string> original;
    original.push_back("MSET");
    original.push_back("key1");
    original.push_back("simple_value");
    original.push_back("key2");
    original.push_back(std::string("binary\0value", 12));
    original.push_back("key3");
    original.push_back("value\r\nwith\ncrlf");
    original.push_back("key4");
    original.push_back("🌍🔑");
    original.push_back("key5");
    original.push_back("\x01\x02\x03\x7F\xFF");
    
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    
    EXPECT_EQ(parsed.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(parsed[i], original[i]) << "Mismatch at index " << i;
    }
}

// ===== PERFORMANCE/STRESS TESTS =====

TEST_F(RESPParserTest, ParseLargeArray) {
    // Test with maximum allowed arguments
    std::string input = "*100\r\n"; // MAX_ARGS
    for (int i = 0; i < 100; i++) {
        input += "$4\r\ntest\r\n";
    }
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 100);
    for (const auto& item : result) {
        EXPECT_EQ(item, "test");
    }
}

TEST_F(RESPParserTest, ParseLargeString) {
    // Test with large bulk string (but within limits)
    std::string large_string(100000, 'a'); // 100KB string
    std::string input = "*1\r\n$100000\r\n" + large_string + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], large_string);
    EXPECT_EQ(result[0].length(), 100000);
}

TEST_F(RESPParserTest, ParseManySmallStrings) {
    // Test parsing many small strings
    std::string input = "*50\r\n";
    std::vector<std::string> expected;
    
    for (int i = 0; i < 50; i++) {
        std::string value = "val" + std::to_string(i);
        input += "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
        expected.push_back(value);
    }
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result, expected);
}

TEST_F(RESPParserTest, FormatLargeArray) {
    // Test formatting large array
    std::vector<std::string> large_array;
    for (int i = 0; i < 100; i++) {
        large_array.push_back("item" + std::to_string(i));
    }
    
    std::string result = RESPParser::formatArray(large_array);
    
    // Round-trip test
    auto parsed = RESPParser::parse(result);
    EXPECT_EQ(parsed, large_array);
}

// ===== MEMORY SAFETY TESTS =====

TEST_F(RESPParserTest, ParseRepeatedNullBytes) {
    // Test with many consecutive null bytes (potential memory issues)
    std::string nulls(1000, '\0');
    std::string input = "*1\r\n$1000\r\n" + nulls + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 1000);
    EXPECT_EQ(result[0], nulls);
}

TEST_F(RESPParserTest, ParseVeryLongSingleCommand) {
    // Test a single very long command string
    std::string long_cmd(10000, 'x');
    std::string input = "*1\r\n$10000\r\n" + long_cmd + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], long_cmd);
}

// ===== COMPATIBILITY TESTS =====

TEST_F(RESPParserTest, ParseRedisCommandExamples) {
    // Real Redis command examples
    
    // SET command
    {
        std::string input = "*3\r\n$3\r\nSET\r\n$6\r\nmykey\r\n$7\r\nmyvalue\r\n";
        auto result = RESPParser::parse(input);
        EXPECT_EQ(result.size(), 3);
        EXPECT_EQ(result[0], "SET");
        EXPECT_EQ(result[1], "mykey");
        EXPECT_EQ(result[2], "myvalue");
    }
    
    // LPUSH with multiple values
    {
        std::string input = "*4\r\n$5\r\nLPUSH\r\n$6\r\nmylist\r\n$5\r\nvalue1\r\n$5\r\nvalue2\r\n";
        auto result = RESPParser::parse(input);
        EXPECT_EQ(result.size(), 4);
        EXPECT_EQ(result[0], "LPUSH");
        EXPECT_EQ(result[1], "mylist");
        EXPECT_EQ(result[2], "value1");
        EXPECT_EQ(result[3], "value2");
    }
    
    // HMSET command
    {
        std::string input = "*5\r\n$5\r\nHMSET\r\n$4\r\nhash\r\n$4\r\nkey1\r\n$6\r\nvalue1\r\n$4\r\nkey2\r\n";
        auto result = RESPParser::parse(input);
        EXPECT_EQ(result.size(), 5);
        EXPECT_EQ(result[0], "HMSET");
        EXPECT_EQ(result[1], "hash");
    }
}

TEST_F(RESPParserTest, ParseWithWindowsLineEndings) {
    // Some clients might send different line endings
    std::string input = "*2\r\n$4\r\nPING\r\n$0\r\n\r\n";
    auto result = RESPParser::parse(input);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "PING");
    EXPECT_EQ(result[1], ""); // Empty string
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}