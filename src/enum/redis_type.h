#pragma once

// Enum para los tipos de valores que maneja RedisValue
enum class RedisType {
    STRING,
    LIST,
    SET,
    HASH,
    ZSET,
    STREAM
};
