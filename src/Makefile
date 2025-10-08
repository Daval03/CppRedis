
# Compilador y flags
CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -I.

# Nombre del ejecutable
TARGET = cppredis
BUILD_DIR = build

# Archivos fuente (lista explícita para mejor control)
SRCS = main.cpp \
       server/connection_manager.cpp \
       server/tcp_server.cpp \
       utils/logger.cpp \
       utils/utility_functions.cpp \
       resp/resp_formatter.cpp \
       resp/resp_parser.cpp \
       resp/resp_value.cpp \
       redis/command_handler.cpp \
       redis/database/redis_database.cpp \
       redis/commands/string_commands.cpp \
	redis/commands/hash_commands.cpp \
	redis/commands/set_commands.cpp \
	redis/commands/ttl_commands.cpp \
	redis/commands/server_commands.cpp \
	redis/commands/list_commands.cpp

# build all
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(BUILD_DIR)/$(TARGET)

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

debug: CXXFLAGS += -g
debug: $(BUILD_DIR)/$(TARGET)

.PHONY: all clean run debug