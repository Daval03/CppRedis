#include "resp_parser.h"
//RESP2 protocol

std::vector<std::string> RESPParser::parse(const std::string& input) {
    std::vector<std::string> result;
    
    if (input.empty()) {
        throw std::invalid_argument("Empty input");
    }
    
    if (input[0] != '*') {
        // Fallback for plain text commands (telnet compatibility)
        return parsePlainText(input);
    }

    size_t pos = 0;
    
    // Read first line *<num>
    size_t line_end = input.find("\r\n", pos);
    if (line_end == std::string::npos) {
        throw std::invalid_argument("Invalid RESP format: missing CRLF after array header");
    }
    
    std::string header = input.substr(pos, line_end - pos);
    if (header.empty() || header[0] != '*') {
        throw std::invalid_argument("Invalid RESP format: expected array");
    }
    
    // Extract number of arguments
    int num_args;
    try {
        num_args = std::stoi(header.substr(1));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid RESP format: invalid array size");
    }
    
    if (num_args < 0) {
        throw std::invalid_argument("Invalid RESP format: negative array size");
    }
    
    if (num_args > MAX_ARGS) {
        throw std::invalid_argument("Invalid RESP format: too many arguments");
    }
    
    pos = line_end + 2; // Skip \r\n
    
    // Read each argument
    for (int i = 0; i < num_args; ++i) {
        if (pos >= input.length()) {
            throw std::invalid_argument("Invalid RESP format: truncated input");
        }
        
        // Read line $<len>
        line_end = input.find("\r\n", pos);
        if (line_end == std::string::npos) {
            throw std::invalid_argument("Invalid RESP format: missing CRLF after bulk string header");
        }
        
        std::string length_line = input.substr(pos, line_end - pos);
        if (length_line.empty() || length_line[0] != '$') {
            throw std::invalid_argument("Invalid RESP format: expected bulk string");
        }
        
        int len;
        try {
            len = std::stoi(length_line.substr(1));
        } catch (const std::exception& e) {
            throw std::invalid_argument("Invalid RESP format: invalid bulk string length");
        }
        
        pos = line_end + 2; // Skip \r\n
        
        if (len < 0) {
            result.push_back(""); // NULL string represented as empty
            continue;
        }
        
        if (len > static_cast<int>(MAX_STRING_LENGTH)) {
            throw std::invalid_argument("Invalid RESP format: string too long");
        }
        
        // Binary-safe reading: read exact number of bytes
        if (pos + len + 2 > input.length()) {
            throw std::invalid_argument("Invalid RESP format: truncated bulk string data");
        }
        
        std::string data = input.substr(pos, len);
        pos += len;
        
        // Verify CRLF terminator
        if (pos + 2 > input.length() || input.substr(pos, 2) != "\r\n") {
            throw std::invalid_argument("Invalid RESP format: missing CRLF after bulk string data");
        }
        
        pos += 2; // Skip \r\n
        result.push_back(data);
    }
    
    return result;
}

std::string RESPParser::formatBulkString(const std::string& str) {
    if (str.empty()) {
        return "$-1\r\n"; // NULL bulk string
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