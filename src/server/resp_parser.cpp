#include "resp_parser.h"


std::vector<std::string> RESPParser::parse(const std::string& input) {
    std::vector<std::string> result;
    
    if (input.empty()) {
        throw std::invalid_argument("Empty input");
    }
    
    if (input[0] != '*') {
        // Fallback for plain text commands (telnet compatibility)
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

    std::istringstream ss(input);
    std::string line;
    
    // Read first line *<num>
    if (!std::getline(ss, line)) {
        throw std::invalid_argument("Invalid RESP format: missing array header");
    }
    
    if (line.empty() || line[0] != '*') {
        throw std::invalid_argument("Invalid RESP format: expected array");
    }
    
    // Remove \r if present
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    
    // Extract number of arguments
    int num_args;
    try {
        num_args = std::stoi(line.substr(1));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid RESP format: invalid array size");
    }
    
    if (num_args < 0) {
        throw std::invalid_argument("Invalid RESP format: negative array size");
    }
    
    if (num_args > MAX_ARGS) {
        throw std::invalid_argument("Invalid RESP format: too many arguments");
    }
    
    // Read each argument
    for (int i = 0; i < num_args; ++i) {
        // Read line $<len>
        if (!std::getline(ss, line)) {
            throw std::invalid_argument("Invalid RESP format: missing bulk string header");
        }
        
        if (line.empty() || line[0] != '$') {
            throw std::invalid_argument("Invalid RESP format: expected bulk string");
        }
        
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        int len;
        try {
            len = std::stoi(line.substr(1));
        } catch (const std::exception& e) {
            throw std::invalid_argument("Invalid RESP format: invalid bulk string length");
        }
        
        if (len < 0) {
            result.push_back(""); // NULL string represented as empty
            continue;
        }
        
        if (len > static_cast<int>(MAX_STRING_LENGTH)) {
            throw std::invalid_argument("Invalid RESP format: string too long");
        }
        
        // Read the argument content
        if (!std::getline(ss, line)) {
            throw std::invalid_argument("Invalid RESP format: missing bulk string content");
        }
        
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // Validate length
        if (static_cast<int>(line.length()) != len) {
            std::cerr << "Warning: Expected length " << len << " but got " << line.length() << std::endl;
        }
        
        result.push_back(line);
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