# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -I../src
LDFLAGS = -pthread -lgtest -lgtest_main

# Archivos fuente comunes
SRC_FILES = ../src/resp/resp_parser.cpp \
            ../src/resp/resp_value.cpp \
            ../src/resp/resp_formatter.cpp \
            ../src/utils/logger.cpp \
            ../src/utils/utility_functions.cpp

# Lista de tests
TESTS = resp/test_resp_value.cpp \
       resp/test_resp_parser.cpp    
#         redis/test_redis_commands.cpp \
#         utils/test_logger.cpp

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

test_formatter: build/resp/test_resp_formatter
	./$<

test_parser: build/resp/test_resp_parser
	./$<

# Limpiar build
clean:
	rm -rf build

.PHONY: all test_all clean