# CppRedis

A lightweight Redis-compatible server written in modern C++.

## Features
- TCP server built with standard C++ and POSIX sockets
- RESP (Redis Serialization Protocol) parser
- Thread-safe in-memory key-value store
- Core Redis commands: `PING`, `SET`, `GET`, `DEL`, `EXISTS`, `KEYS`, `INFO`, `ECHO`, `QUIT`

# Run server (default port 6379)
./cppredis
