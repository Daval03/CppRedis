#ifndef RESP_PARSER_H
#define RESP_PARSER_H

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <limits>

class RESPParser {
private:
    static const size_t MAX_STRING_LENGTH = 1024 * 1024;  // 1MB
    static const int MAX_ARGS = 1000;
    // Helpers for security
    static bool findCRLF(const std::string& input, size_t start, size_t& end);
    static bool safeStringToInt(const std::string& str, int& result);

public:
    static bool parse(const std::string& input, std::vector<std::string>& result, size_t& consumed);
    static std::string formatBulkString(const std::string& str);
    static std::string formatInteger(int value);
    static std::string formatSimpleString(const std::string& str);
    static std::string formatError(const std::string& error);
    static std::string formatArray(const std::vector<std::string>& items);
    static std::string formatNull();
    
    
};
//utils
std::vector<std::string> parsePlainText(const std::string& input);


#endif