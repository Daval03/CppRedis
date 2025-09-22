#!/bin/bash

# Test script for Redis server
PORT=6379

echo "Testing Redis server on port $PORT..."
echo "======================================"

# Test basic commands
echo "1. Testing PING..."
redis-cli -p $PORT PING

echo "2. Testing SET and GET..."
redis-cli -p $PORT SET key1 "value1"
redis-cli -p $PORT GET key1

echo "3. Testing INCR..."
redis-cli -p $PORT SET counter 10
redis-cli -p $PORT INCR counter
redis-cli -p $PORT GET counter

echo "4. Testing LIST operations..."
redis-cli -p $PORT LPUSH mylist "item1"
redis-cli -p $PORT LPUSH mylist "item2"
redis-cli -p $PORT LRANGE mylist 0 -1

echo "5. Testing SET operations..."
redis-cli -p $PORT SADD myset "elem1"
redis-cli -p $PORT SADD myset "elem2"
redis-cli -p $PORT SMEMBERS myset

echo "6. Testing HASH operations..."
redis-cli -p $PORT HSET user:1 name "John"
redis-cli -p $PORT HSET user:1 age "30"
redis-cli -p $PORT HGETALL user:1

echo "7. Testing DBSIZE and KEYS..."
redis-cli -p $PORT DBSIZE
redis-cli -p $PORT KEYS "*"

echo "8. Testing TTL..."
redis-cli -p $PORT SET tempkey "tempvalue" EX 3
redis-cli -p $PORT TTL tempkey

echo "======================================"
echo "Test completed!"