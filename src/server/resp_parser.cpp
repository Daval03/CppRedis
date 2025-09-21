#include "resp_parser.h"
//RESP2 protocol

bool RESPParser::parse(const std::string& input, std::vector<std::string>& result,size_t& consumed){
    result.clear();
    consumed = 0;

    if (input.empty()) return false;
    if (input[0] != '*') return false;   // solo RESP arrays

    size_t pos = 1;                      // después de '*'
    size_t line_end = input.find("\r\n", pos);
    if (line_end == std::string::npos) return false;

    int num_args = std::stoi(input.substr(pos, line_end - pos));
    pos = line_end + 2;

    for (int i = 0; i < num_args; ++i) {
        if (pos >= input.size()) return false;

        if (input[pos] != '$') throw std::runtime_error("Expected $");
        size_t len_end = input.find("\r\n", pos + 1);
        if (len_end == std::string::npos) return false;

        int len = std::stoi(input.substr(pos + 1, len_end - pos - 1));
        pos = len_end + 2;

        if (len < 0) {
            result.emplace_back("");
            continue;
        }

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

