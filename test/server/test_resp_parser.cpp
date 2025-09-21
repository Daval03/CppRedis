#include <gtest/gtest.h>
#include "../../src/server/resp_parser.h"

class RESPParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
    
    // Helper function to parse with the current interface
    bool parseHelper(const std::string& input, std::vector<std::string>& result) {
        size_t consumed;
        return RESPParser::parse(input, result, consumed);
    }
};

// ===== PARSING TESTS =====

TEST_F(RESPParserTest, ParseSimpleArray) {
    std::string input = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], "key");
}

TEST_F(RESPParserTest, ParseEmptyArray) {
    std::string input = "*0\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 0);
}

TEST_F(RESPParserTest, ParseNullArray) {
    std::string input = "*-1\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    // Your parser returns false for invalid array size
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseNullBulkString) {
    std::string input = "*2\r\n$3\r\nGET\r\n$-1\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], ""); // Your parser returns empty string for null
}

TEST_F(RESPParserTest, ParseBinaryData) {
    std::string binary_data = std::string("hello\0world", 11);
    std::string input = "*1\r\n$11\r\n" + binary_data + "\r\n";
    
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 11);
    EXPECT_EQ(result[0], binary_data);
}

TEST_F(RESPParserTest, ParseStringLengthMismatch) {
    std::string input = "*1\r\n$5\r\nhi\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    // Should return false for length mismatch
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseTooManyArguments) {
    std::string input = "*101\r\n";
    for (int i = 0; i < 101; i++) {
        input += "$1\r\na\r\n";
    }
    
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    // Should return false for too many arguments
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParsePlainTextCommand) {
    std::string input = "GET mykey";
    auto result = parsePlainText(input);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "GET");
    EXPECT_EQ(result[1], "mykey");
}

TEST_F(RESPParserTest, ParsePlainTextWithNewlines) {
    std::string input = "SET key value\r\n";
    auto result = parsePlainText(input);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "SET");
    EXPECT_EQ(result[1], "key");
    EXPECT_EQ(result[2], "value");
}

TEST_F(RESPParserTest, ParseInvalidArrayHeader) {
    std::string input = "*2invalid\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseMissingCRLF) {
    std::string input = "*2\n$3\nGET\n$3\nkey\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseTruncatedInput) {
    std::string input = "*2\r\n$3\r\nGET\r\n$3\r\nke";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseNegativeArraySize) {
    std::string input = "*-2\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseMissingBulkStringHeader) {
    std::string input = "*1\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseInvalidBulkStringLength) {
    std::string input = "*1\r\n$abc\r\nhello\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

TEST_F(RESPParserTest, ParseEmptyInput) {
    std::string input = "";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

// ===== FORMATTING TESTS =====

TEST_F(RESPParserTest, FormatBulkString) {
    std::string result = RESPParser::formatBulkString("hello");
    EXPECT_EQ(result, "$5\r\nhello\r\n");
}

TEST_F(RESPParserTest, FormatEmptyBulkString) {
    std::string result = RESPParser::formatBulkString("");
    EXPECT_EQ(result, "$0\r\n\r\n"); // Your implementation returns empty string, not null
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
    std::string input = "*1\r\n$6\r\nhello\rworld\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello\rworld");
}

TEST_F(RESPParserTest, ParseStringWithNewline) {
    std::string input = "*1\r\n$6\r\nhello\nworld\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello\nworld");
}

TEST_F(RESPParserTest, ParseStringWithCRLF) {
    std::string input = "*1\r\n$7\r\nhello\r\nworld\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello\r\nworld");
}

TEST_F(RESPParserTest, ParseMultipleNullBytes) {
    std::string binary_data = std::string("a\0b\0c\0d", 7);
    std::string input = "*1\r\n$7\r\n" + binary_data + "\r\n";
    
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].length(), 7);
    EXPECT_EQ(result[0], binary_data);
}

TEST_F(RESPParserTest, ParseZeroLengthString) {
    std::string input = "*1\r\n$0\r\n\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST_F(RESPParserTest, ParseMaxArguments) {
    const int max_args = 100;
    std::string input = "*" + std::to_string(max_args) + "\r\n";
    for (int i = 0; i < max_args; i++) {
        input += "$1\r\na\r\n";
    }
    
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), max_args);
}

TEST_F(RESPParserTest, ParseExcessiveArguments) {
    const int over_limit = 101;
    std::string input = "*" + std::to_string(over_limit) + "\r\n";
    
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_FALSE(success);
}

// ===== ROUND-TRIP TESTS =====

TEST_F(RESPParserTest, RoundTripFormatParse) {
    std::vector<std::string> original = {"GET", "mykey", "value"};
    std::string formatted = RESPParser::formatArray(original);
    
    std::vector<std::string> parsed;
    size_t consumed;
    bool success = RESPParser::parse(formatted, parsed, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithBinaryData) {
    std::vector<std::string> original = {"SET", "key", std::string("binary\0data", 11)};
    std::string formatted = RESPParser::formatArray(original);
    
    std::vector<std::string> parsed;
    size_t consumed;
    bool success = RESPParser::parse(formatted, parsed, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(parsed, original);
}

TEST_F(RESPParserTest, RoundTripWithUTF8) {
    std::vector<std::string> original = {"SET", "🔑", "世界"};
    std::string formatted = RESPParser::formatArray(original);
    
    std::vector<std::string> parsed;
    size_t consumed;
    bool success = RESPParser::parse(formatted, parsed, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(parsed, original);
}

// ===== COMPATIBILITY TESTS =====

TEST_F(RESPParserTest, ParseRedisCommandExamples) {
    // SET command
    std::string input = "*3\r\n$3\r\nSET\r\n$6\r\nmykey\r\n$7\r\nmyvalue\r\n";
    std::vector<std::string> result;
    size_t consumed;
    bool success = RESPParser::parse(input, result, consumed);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "SET");
    EXPECT_EQ(result[1], "mykey");
    EXPECT_EQ(result[2], "myvalue");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}