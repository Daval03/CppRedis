# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -I../src
LDFLAGS = -pthread -lgtest -lgtest_main -lgmock

# Archivos fuente comunes
SRC_FILES = ../src/server/connection_manager.cpp \
       		../src/server/tcp_server.cpp \
			../src/utils/logger.cpp \
			../src/utils/utility_functions.cpp \
			../src/resp/resp_formatter.cpp \
			../src/resp/resp_parser.cpp \
			../src/resp/resp_value.cpp \
			../src/redis/command_handler.cpp \
			../src/redis/database/redis_database.cpp \
			../src/redis/commands/string_commands.cpp \
			../src/redis/commands/hash_commands.cpp \
			../src/redis/commands/set_commands.cpp \
			../src/redis/commands/ttl_commands.cpp \
			../src/redis/commands/server_commands.cpp \
			../src/redis/commands/list_commands.cpp

# Lista de tests
TESTS = resp/test_resp_value.cpp \
		resp/test_resp_formatter.cpp \
		resp/test_resp_parser.cpp \
		utils/test_utility_functions.cpp \
		server/test_connection_manager.cpp \
		server/test_tcp_server.cpp \
		redis/test_redis_value.cpp \
		redis/test_redis_database.cpp \
		redis/test_ttl_commands.cpp \
		redis/test_string_commands.cpp \
		redis/test_set_commands.cpp \
		redis/test_server_commands.cpp \
		redis/test_list_commands.cpp \
		redis/test_hash_commands.cpp 

# Generar nombres de ejecutables en build
TEST_TARGETS = $(TESTS:%.cpp=build/%)

# Regla principal
all: $(TEST_TARGETS)

# Regla genérica para compilar tests
build/%: %.cpp $(SRC_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_FILES) $(LDFLAGS)

# Ejecutar todos los tests
test_all: $(TEST_TARGETS)
	@for test in $^; do \
		echo "Ejecutando $$test..."; \
		./$$test; \
	done

1: build/resp/test_resp_formatter
	./$<

# Limpiar build
clean:
	rm -rf build/*
	clear

.PHONY: all test_all clean