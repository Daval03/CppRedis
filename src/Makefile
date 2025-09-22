# # Compilador y flags
# CXX = g++
# CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread

# # Nombre del ejecutable
# TARGET = cppredis

# # Directorios
# BUILD_DIR = build
# SRC_DIR = .
# SERVER_DIR = server
# UTILS_DIR = utils
# RESP_DIR = resp
# REDIS_DIR = redis
# REDIS_DATABASE_DIR = redis/database
# REDIS_COMMANDS_DIR = redis/commands

# # Archivos fuente
# SRCS = $(SRC_DIR)/main.cpp \
#         $(SERVER_DIR)/tcp_server.cpp \
# 		$(SERVER_DIR)/connection_manager.cpp \
# 		$(RESP_DIR)/resp_parser.cpp \
# 		$(RESP_DIR)/resp_value.cpp \
# 		$(RESP_DIR)/resp_formatter.cpp \
# 		$(REDIS_DIR)/command_handler.cpp \
# 		$(REDIS_DATABASE_DIR)/redis_database.cpp \
# 		$(REDIS_COMMANDS_DIR)/string_commands.cpp \
# 		$(UTILS_DIR)/utility_functions.cpp \
# 		$(UTILS_DIR)/logger.cpp 

        
# test:
# 	g++ -std=c++17 -Wall -Wextra -O2 -pthread $(RESP_DIR)/resp_parser.cpp $(RESP_DIR)/resp_value.cpp -o $(BUILD_DIR)/test
# 	./$(BUILD_DIR)/test

# # build all
# all: $(BUILD_DIR)/$(TARGET)

# $(BUILD_DIR)/$(TARGET): $(SRCS)
# 	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

# # Delete all
# clean:
# 	rm -f $(BUILD_DIR)/$(TARGET)

# # run
# run: $(BUILD_DIR)/$(TARGET)
# 	./$(BUILD_DIR)/$(TARGET)

# # debug
# debug: CXXFLAGS += -g
# debug: $(BUILD_DIR)/$(TARGET)

# .PHONY: all clean run debug test

# Compilador y flags
CXX = g++
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