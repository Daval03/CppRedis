#pragma once
#include <string>
#include <vector>

class RESPFormatter {
public:
    static std::string formatError(const std::string& message);
    static std::string formatSimpleString(const std::string& str);
    static std::string formatBulkString(const std::string& str);
    static std::string formatInteger(long long value);
    static std::string formatArray(const std::vector<std::string>& items);
    static std::string formatNull();
};