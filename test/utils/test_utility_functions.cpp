#include <gtest/gtest.h>
#include "utils/utility_functions.h"

class UtilityFunctionsTest : public ::testing::Test {};

// Tests for isInteger function
TEST_F(UtilityFunctionsTest, IsInteger_ValidPositiveInteger) {
    EXPECT_TRUE(UtilityFunctions::isInteger("123"));
    EXPECT_TRUE(UtilityFunctions::isInteger("0"));
    EXPECT_TRUE(UtilityFunctions::isInteger("+123"));
    EXPECT_TRUE(UtilityFunctions::isInteger("999999999"));
}

TEST_F(UtilityFunctionsTest, IsInteger_ValidNegativeInteger) {
    EXPECT_TRUE(UtilityFunctions::isInteger("-123"));
    EXPECT_TRUE(UtilityFunctions::isInteger("-0"));
    EXPECT_TRUE(UtilityFunctions::isInteger("-999999999"));
}

TEST_F(UtilityFunctionsTest, IsInteger_InvalidInputs) {
    EXPECT_FALSE(UtilityFunctions::isInteger(""));
    EXPECT_FALSE(UtilityFunctions::isInteger("abc"));
    EXPECT_FALSE(UtilityFunctions::isInteger("123abc"));
    EXPECT_FALSE(UtilityFunctions::isInteger("12.34"));
    EXPECT_FALSE(UtilityFunctions::isInteger("123 "));
    EXPECT_FALSE(UtilityFunctions::isInteger(" 123"));
    EXPECT_FALSE(UtilityFunctions::isInteger("+"));
    EXPECT_FALSE(UtilityFunctions::isInteger("-"));
    EXPECT_FALSE(UtilityFunctions::isInteger("++123"));
    EXPECT_FALSE(UtilityFunctions::isInteger("--123"));
}

// Tests for parseInt function
TEST_F(UtilityFunctionsTest, ParseInt_ValidIntegers) {
    EXPECT_EQ(UtilityFunctions::parseInt("123"), 123);
    EXPECT_EQ(UtilityFunctions::parseInt("0"), 0);
    EXPECT_EQ(UtilityFunctions::parseInt("-123"), -123);
    EXPECT_EQ(UtilityFunctions::parseInt("+456"), 456);
    EXPECT_EQ(UtilityFunctions::parseInt("9223372036854775807"), 9223372036854775807LL); // LLONG_MAX
}

TEST_F(UtilityFunctionsTest, ParseInt_InvalidInputsReturnsZero) {
    EXPECT_EQ(UtilityFunctions::parseInt(""), 0);
    EXPECT_EQ(UtilityFunctions::parseInt("abc"), 0);
    EXPECT_EQ(UtilityFunctions::parseInt("123abc"), 123);
    EXPECT_EQ(UtilityFunctions::parseInt("12.34"), 12);
}

// Tests for toUpper function
TEST_F(UtilityFunctionsTest, ToUpper_BasicConversion) {
    EXPECT_EQ(UtilityFunctions::toUpper("hello"), "HELLO");
    EXPECT_EQ(UtilityFunctions::toUpper("HELLO"), "HELLO");
    EXPECT_EQ(UtilityFunctions::toUpper("Hello World"), "HELLO WORLD");
    EXPECT_EQ(UtilityFunctions::toUpper("123abc"), "123ABC");
    EXPECT_EQ(UtilityFunctions::toUpper(""), "");
}

TEST_F(UtilityFunctionsTest, ToUpper_SpecialCharacters) {
    EXPECT_EQ(UtilityFunctions::toUpper("hello@world.com"), "HELLO@WORLD.COM");
    EXPECT_EQ(UtilityFunctions::toUpper("test_case"), "TEST_CASE");
    EXPECT_EQ(UtilityFunctions::toUpper("mixed123ABC"), "MIXED123ABC");
}

// Tests for matchPattern function
TEST_F(UtilityFunctionsTest, MatchPattern_WildcardMatches) {
    EXPECT_TRUE(UtilityFunctions::matchPattern("*", ""));
    EXPECT_TRUE(UtilityFunctions::matchPattern("*", "anything"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("*", "hello world"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("*", "123"));
}

TEST_F(UtilityFunctionsTest, MatchPattern_QuestionMarkMatches) {
    EXPECT_TRUE(UtilityFunctions::matchPattern("h?llo", "hello"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("te?t", "test"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("a???", "abcd"));
    EXPECT_FALSE(UtilityFunctions::matchPattern("h?llo", "hllo")); // too short
    EXPECT_FALSE(UtilityFunctions::matchPattern("h?llo", "heello")); // too long
}

TEST_F(UtilityFunctionsTest, MatchPattern_ExactMatches) {
    EXPECT_TRUE(UtilityFunctions::matchPattern("hello", "hello"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("test", "test"));
    EXPECT_FALSE(UtilityFunctions::matchPattern("hello", "hell"));
    EXPECT_FALSE(UtilityFunctions::matchPattern("hello", "helloo"));
}

TEST_F(UtilityFunctionsTest, MatchPattern_ComplexPatterns) {
    EXPECT_TRUE(UtilityFunctions::matchPattern("h*llo", "hello"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("h*llo", "hllo"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("h*llo", "heeeeeello"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("*world", "hello world"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("hello*", "hello world"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("h*e*l*o", "hello"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("h*e*l*o", "heeeeelllllloooo"));
    
    EXPECT_FALSE(UtilityFunctions::matchPattern("h*llo", "hll"));
    EXPECT_FALSE(UtilityFunctions::matchPattern("h*llo", "hall"));
}

TEST_F(UtilityFunctionsTest, MatchPattern_EdgeCases) {
    EXPECT_TRUE(UtilityFunctions::matchPattern("", ""));
    EXPECT_FALSE(UtilityFunctions::matchPattern("", "a"));
    EXPECT_FALSE(UtilityFunctions::matchPattern("a", ""));
    EXPECT_TRUE(UtilityFunctions::matchPattern("*", ""));
    EXPECT_TRUE(UtilityFunctions::matchPattern("**", "anything"));
    EXPECT_TRUE(UtilityFunctions::matchPattern("***", "test"));
}

// Tests for isValidKey function
TEST_F(UtilityFunctionsTest, IsValidKey_ValidKeys) {
    EXPECT_TRUE(UtilityFunctions::isValidKey("a"));
    EXPECT_TRUE(UtilityFunctions::isValidKey("valid_key"));
    EXPECT_TRUE(UtilityFunctions::isValidKey("key123"));
    EXPECT_TRUE(UtilityFunctions::isValidKey("A"));
    EXPECT_TRUE(UtilityFunctions::isValidKey("key_with_underscore"));
    EXPECT_TRUE(UtilityFunctions::isValidKey(std::string(511, 'a'))); // max length - 1
    EXPECT_TRUE(UtilityFunctions::isValidKey(std::string(510, 'a')));
}

TEST_F(UtilityFunctionsTest, IsValidKey_InvalidKeys) {
    EXPECT_FALSE(UtilityFunctions::isValidKey(""));
    EXPECT_FALSE(UtilityFunctions::isValidKey(std::string(512, 'a'))); // exactly max length
    EXPECT_FALSE(UtilityFunctions::isValidKey(std::string(1000, 'a'))); // way too long
}

// Integration tests
TEST_F(UtilityFunctionsTest, Integration_ParseValidInteger) {
    std::string validInt = "123";
    EXPECT_TRUE(UtilityFunctions::isInteger(validInt));
    EXPECT_EQ(UtilityFunctions::parseInt(validInt), 123);
}

TEST_F(UtilityFunctionsTest, Integration_ParseInvalidInteger) {
    std::string invalidInt = "abc";
    EXPECT_FALSE(UtilityFunctions::isInteger(invalidInt));
    EXPECT_EQ(UtilityFunctions::parseInt(invalidInt), 0);
}

TEST_F(UtilityFunctionsTest, Integration_KeyValidationWithPattern) {
    std::string key = "TEST_KEY_123";
    std::string pattern = "TEST_*";
    
    EXPECT_TRUE(UtilityFunctions::isValidKey(key));
    EXPECT_TRUE(UtilityFunctions::matchPattern(pattern, key));
    EXPECT_EQ(UtilityFunctions::toUpper(key), key); // already uppercase
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}