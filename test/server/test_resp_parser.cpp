#include <gtest/gtest.h>
#include "../../src/server/resp_parser.h"

class RESPParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Tests de parsing
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

TEST_F(RESPParserTest, ParseInvalidArrayHeader) {
    std::string input = "*2invalid\r\n";
    EXPECT_THROW(RESPParser::parse(input), std::invalid_argument);
}

// Tests de formato
TEST_F(RESPParserTest, FormatBulkString) {
    std::string result = RESPParser::formatBulkString("hello");
    EXPECT_EQ(result, "$5\r\nhello\r\n");
}

TEST_F(RESPParserTest, FormatInteger) {
    std::string result = RESPParser::formatInteger(42);
    EXPECT_EQ(result, ":42\r\n");
}

TEST_F(RESPParserTest, FormatArray) {
    std::vector<std::string> items = {"SET", "key", "value"};
    std::string result = RESPParser::formatArray(items);
    EXPECT_EQ(result, "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
}

// Test de round-trip
TEST_F(RESPParserTest, RoundTripFormatParse) {
    std::vector<std::string> original = {"GET", "mykey", "value"};
    std::string formatted = RESPParser::formatArray(original);
    std::vector<std::string> parsed = RESPParser::parse(formatted);
    EXPECT_EQ(parsed, original);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}