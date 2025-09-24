#include "resp/resp_formatter.h"

std::string RESPFormatter::formatError(const std::string& message) {
    return "-" + message + "\r\n";
}

std::string RESPFormatter::formatSimpleString(const std::string& str) {
    return "+" + str + "\r\n";
}

std::string RESPFormatter::formatBulkString(const std::string& str) {
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string RESPFormatter::formatInteger(long long value) {
    return ":" + std::to_string(value) + "\r\n";
}

std::string RESPFormatter::formatArray(const std::vector<std::string>& items) {
    std::string response = "*" + std::to_string(items.size()) + "\r\n";
    for (const auto& item : items) {
        response += formatBulkString(item);
    }
    return response;
}

std::string RESPFormatter::formatNull() {
    return "$-1\r\n";
}