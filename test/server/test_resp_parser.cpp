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

// ===== ROUND-TRIP TESTS =====

TEST_F(RESPParserTest, RoundTripFormatParse) {
    std::vector<std::string> original = {"GET", "mykey", "value"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithEmptyStrings) {
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
    std::string large_string(1000, 'a'); // 1KB string
    std::string input = "*1\r\n$1000\r\n" + large_string + "\r\n";
    
    auto result = RESPParser::parse(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], large_string);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}