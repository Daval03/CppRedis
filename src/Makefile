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
        
        

# Regla principal - compilación directa sin objetos intermedios
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

# Limpieza - solo elimina el ejecutable, no objetos
clean:
	rm -f $(BUILD_DIR)/$(TARGET)

# Ejecutar
run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

# Depuración
debug: CXXFLAGS += -g
debug: $(BUILD_DIR)/$(TARGET)

.PHONY: all clean run debug