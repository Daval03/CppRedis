# CppRedis

A lightweight Redis-compatible server written in modern C++.

## Features

- TCP server built with standard C++ and POSIX sockets
- RESP (Redis Serialization Protocol 2) parser
- Thread-safe in-memory key-value store
- Modular command architecture with specialized handlers
- Automatic TTL and key expiration
- Periodic cleanup of expired keys

## Supported Data Types & Commands

### Strings
- `SET`, `GET`, `DEL`, `EXISTS`, `TYPE`
- `INCR`, `DECR`, `INCRBY`, `DECRBY`
- `STRLEN`, `APPEND`, `MGET`, `MSET`

### Lists
- `LPUSH`, `RPUSH`, `LPOP`, `RPOP`
- `LLEN`, `LRANGE`, `LINDEX`, `LSET`

### Sets
- `SADD`, `SREM`, `SISMEMBER`, `SCARD`
- `SMEMBERS`, `SPOP`

### Hashes
- `HSET`, `HGET`, `HDEL`, `HEXISTS`
- `HLEN`, `HKEYS`, `HVALS`, `HGETALL`

### TTL Management
- `EXPIRE`, `EXPIREAT`, `TTL`, `PERSIST`

### Server Commands
- `PING`, `ECHO`, `INFO`, `FLUSHALL`
- `KEYS`, `DBSIZE`, `TIME`, `QUIT`

## Usage

```bash
# Run server (default port 6379)
./cppredis

# Connect with redis-cli
redis-cli -p 6379
