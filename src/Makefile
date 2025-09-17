# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread

# Nombre del ejecutable
TARGET = cppredis

# Directorios
BUILD_DIR = build
SRC_DIR = .
SERVER_DIR = server

# Archivos fuente
SRCS = $(SRC_DIR)/main.cpp \
        $(SERVER_DIR)/tcp_server.cpp \
		$(SERVER_DIR)/resp_parser.cpp \
		$(SERVER_DIR)/redis_command_handler.cpp \
		$(SERVER_DIR)/redis_store.cpp \
		$(SERVER_DIR)/connection_manager.cpp 
        
        

# build all
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

# Delete all
clean:
	rm -f $(BUILD_DIR)/$(TARGET)

# run
run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

# debug
debug: CXXFLAGS += -g
debug: $(BUILD_DIR)/$(TARGET)

.PHONY: all clean run debug