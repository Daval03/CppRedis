#include <gtest/gtest.h>
#include <thread> 
#include "redis/database/redis_value.h"

TEST(RedisValueTest, DefaultConstructor) {
    RedisValue val;
    EXPECT_EQ(val.type, RedisType::STRING);
    EXPECT_FALSE(val.has_expiry);
}

TEST(RedisValueTest, TypeConstructor) {
    RedisValue val_list(RedisType::LIST);
    EXPECT_EQ(val_list.type, RedisType::LIST);

    RedisValue val_hash(RedisType::HASH);
    EXPECT_EQ(val_hash.type, RedisType::HASH);
    
    RedisValue val_stream(RedisType::STREAM);
    EXPECT_EQ(val_stream.type, RedisType::STREAM);
}

TEST(RedisValueTest, StringConstructor) {
    const std::string test_str = "hello gtest!";
    RedisValue val(test_str);
    EXPECT_EQ(val.type, RedisType::STRING);
    EXPECT_EQ(val.string_value, test_str);
    EXPECT_FALSE(val.has_expiry);
}


TEST(RedisValueTest, SetExpiry) {
    RedisValue val;
    ASSERT_FALSE(val.has_expiry);
    val.setExpiry(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(val.has_expiry);
    
    EXPECT_GT(val.expiry, std::chrono::system_clock::now());
}

TEST(RedisValueTest, ClearExpiry) {
    RedisValue val;
    val.setExpiry(std::chrono::milliseconds(100));
    ASSERT_TRUE(val.has_expiry); 
    val.clearExpiry();
    EXPECT_FALSE(val.has_expiry);
}

TEST(RedisValueTest, IsExpired) {
    RedisValue val;

    EXPECT_FALSE(val.isExpired());

    val.setExpiry(std::chrono::milliseconds(500));
    EXPECT_FALSE(val.isExpired());

    val.setExpiry(std::chrono::milliseconds(10)); 
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); 
    EXPECT_TRUE(val.isExpired());
}

TEST(RedisValueTest, ImmediateExpiration) {
    RedisValue val;

    val.setExpiry(std::chrono::milliseconds(-100));
    EXPECT_TRUE(val.isExpired());

    val.setExpiry(std::chrono::milliseconds(0));
    EXPECT_TRUE(val.isExpired());
}