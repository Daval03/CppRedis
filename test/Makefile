CXX       = g++
CXXFLAGS  = -std=c++17 -I../src -pthread
LDFLAGS   = -lgtest -lgtest_main -pthread

BUILD_DIR       = build
SRC_SERVER_DIR  = ../src/server
TEST_SERVER_DIR = ../test/server

# === List of test ===
TESTS = test_resp_parser   # More test

# Commands for each file
test_resp_parser: $(BUILD_DIR)/test_resp_parser
	./$<

# Compile each test 
$(BUILD_DIR)/%: $(SRC_SERVER_DIR)/resp_parser.cpp $(TEST_SERVER_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# Compile all test
test_all: $(addprefix $(BUILD_DIR)/, $(TESTS))
	@for t in $(TESTS); do echo "== Running $$t =="; ./$(BUILD_DIR)/$$t; done

clean:
	rm -f $(BUILD_DIR)/*

.PHONY: $(TESTS) test_all clean
