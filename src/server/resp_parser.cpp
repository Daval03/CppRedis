// RESP2 protocol
// Bulk Strings and Arrays of bulk strings - Ready
// Simple Strings, Errors, Integers, Mixed Arrays and Nested Arrays - Not

#include "resp_parser.h"

bool RESPParser::parse(const std::string& input, std::vector<std::string>& result, size_t& consumed){
    result.clear();
    consumed = 0;

    if (input.empty()) return false;
    if (input[0] != '*') return false;   // only RESP arrays

    size_t pos = 1;                      // after '*'
    size_t line_end;
    if (!findCRLF(input, pos, line_end)) return false;

    int num_args;
    if (!safeStringToInt(input.substr(pos, line_end - pos), num_args)) return false;
    if (num_args < 0 || num_args > MAX_ARGS) return false;  // Boundary validation
    
    pos = line_end + 2;

    for (int i = 0; i < num_args; ++i) {
        if (pos >= input.size()) return false;

        if (input[pos] != '$') return false;  // Changed exception to return false
        
        size_t len_end;
        if (!findCRLF(input, pos + 1, len_end)) return false;

        int len;
        if (!safeStringToInt(input.substr(pos + 1, len_end - pos - 1), len)) return false;
        pos = len_end + 2;

        if (len == -1) {  // NULL bulk string
            result.emplace_back("");
            continue;
        }

        if (len < 0 || static_cast<size_t>(len) > MAX_STRING_LENGTH) return false;  // Size validation
        if (pos + len + 2 > input.size()) return false;

        result.emplace_back(input.substr(pos, len));
        pos += len;

        if (pos + 2 > input.size() || input[pos] != '\r' || input[pos + 1] != '\n')
            return false;
        pos += 2;
    }

    consumed = pos; 
    return true;
}

std::string RESPParser::formatBulkString(const std::string& str) {
    if (str.empty()) {
        return "$0\r\n\r\n";  // Empty string, not NULL
    }
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string RESPParser::formatInteger(int value) {
    return ":" + std::to_string(value) + "\r\n";
}

std::string RESPParser::formatSimpleString(const std::string& str) {
    return "+" + str + "\r\n";
}

std::string RESPParser::formatError(const std::string& error) {
    return "-" + error + "\r\n";
}

std::string RESPParser::formatArray(const std::vector<std::string>& items) {
    std::string response = "*" + std::to_string(items.size()) + "\r\n";
    for (const auto& item : items) {
        response += formatBulkString(item);
    }
    return response;
}

std::string RESPParser::formatNull() {
    return "$-1\r\n";
}

// Private helpers for security
bool RESPParser::findCRLF(const std::string& input, size_t start, size_t& end) {
    end = input.find("\r\n", start);
    return end != std::string::npos;
}

bool RESPParser::safeStringToInt(const std::string& str, int& result) {
    if (str.empty()) return false;
    
    try {
        size_t processed;
        long long val = std::stoll(str, &processed);
        
        // Verify that the entire string was processed
        if (processed != str.length()) return false;
        
        // Verify int range
        if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
            return false;
        }
        
        result = static_cast<int>(val);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<std::string> parsePlainText(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;
    
    while (iss >> token) {
        // Remove \r\n characters
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        if (!token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}