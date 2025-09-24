#ifndef RESP_PARSER_H
#define RESP_PARSER_H

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <limits>
#include <variant>
#include <optional>
#include "enum/resp_type.h"
#include "resp_value.h"


class RESPParser {
private:
    static const size_t MAX_STRING_LENGTH = 1024 * 1024;  // 1MB
    static const int MAX_ARGS = 1000;
    static const int MAX_DEPTH = 100;  // Prevent stack overflow in nested arrays
    
    // Private parsing helpers
    static bool findCRLF(const std::string& input, size_t start, size_t& end);
    static bool safeStringToInt(const std::string& str, int& result);
    static bool safeStringToLongLong(const std::string& str, long long& result);
    static bool parseValue(const std::string& input, size_t& pos, RESPValue& result, int depth = 0);
    static bool parseSimpleString(const std::string& input, size_t& pos, RESPValue& result);
    static bool parseError(const std::string& input, size_t& pos, RESPValue& result);
    static bool parseInteger(const std::string& input, size_t& pos, RESPValue& result);
    static bool parseBulkString(const std::string& input, size_t& pos, RESPValue& result);
    static bool parseArray(const std::string& input, size_t& pos, RESPValue& result, int depth);

public:
    // New comprehensive parsing method
    static bool parse(const std::string& input, RESPValue& result, size_t& consumed);
    
    // Utility methods
    static std::vector<std::string> toStringVector(const RESPValue& value);
    static void printValue(const RESPValue& value, int indent = 0);
};

// Utils
std::vector<std::string> parsePlainText(const std::string& input);

#endif