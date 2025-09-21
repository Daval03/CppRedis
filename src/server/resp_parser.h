#ifndef RESP_PARSER_H
#define RESP_PARSER_H

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>

class RESPParser {
private:
    static const size_t MAX_STRING_LENGTH = 1024 * 1024;
    static const int MAX_ARGS = 100;

public:
    static std::vector<std::string> parse(const std::string& input);
    static std::string formatBulkString(const std::string& str);
    static std::string formatInteger(int value);
    static std::string formatSimpleString(const std::string& str);
    static std::string formatError(const std::string& error);
    static std::string formatArray(const std::vector<std::string>& items);
    static std::string formatNull();
    
};

std::vector<std::string> parsePlainText(const std::string& input);

#endif