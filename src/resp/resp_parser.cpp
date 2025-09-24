// RESP2 protocol - Complete Implementation
// Supports: Simple Strings, Errors, Integers, Bulk Strings, Arrays (including nested), and NULL values

#include "resp_parser.h"

// New comprehensive parsing method
bool RESPParser::parse(const std::string& input, RESPValue& result, size_t& consumed) {
    consumed = 0;
    if (input.empty()) return false;
    
    size_t pos = 0;
    return parseValue(input, pos, result, 0) && (consumed = pos, true);
}


// Core recursive parsing logic
bool RESPParser::parseValue(const std::string& input, size_t& pos, RESPValue& result, int depth) {
    if (pos >= input.size()) return false;
    if (depth > MAX_DEPTH) return false;  // Prevent stack overflow
    
    char type_marker = input[pos];
    
    switch (type_marker) {
        case '+': return parseSimpleString(input, pos, result);
        case '-': return parseError(input, pos, result);
        case ':': return parseInteger(input, pos, result);
        case '$': return parseBulkString(input, pos, result);
        case '*': return parseArray(input, pos, result, depth);
        default: return false;
    }
}

// Parse Simple String: +OK\r\n
bool RESPParser::parseSimpleString(const std::string& input, size_t& pos, RESPValue& result) {
    if (pos >= input.size() || input[pos] != '+') return false;
    
    size_t line_end;
    if (!findCRLF(input, pos + 1, line_end)) return false;
    
    std::string str = input.substr(pos + 1, line_end - pos - 1);
    result = RESPValue(RESPType::SIMPLE_STRING, str);
    pos = line_end + 2;
    return true;
}

// Parse Error: -ERR unknown command\r\n
bool RESPParser::parseError(const std::string& input, size_t& pos, RESPValue& result) {
    if (pos >= input.size() || input[pos] != '-') return false;
    
    size_t line_end;
    if (!findCRLF(input, pos + 1, line_end)) return false;
    
    std::string error = input.substr(pos + 1, line_end - pos - 1);
    result = RESPValue(RESPType::ERROR, error);
    pos = line_end + 2;
    return true;
}

// Parse Integer: :1000\r\n
bool RESPParser::parseInteger(const std::string& input, size_t& pos, RESPValue& result) {
    if (pos >= input.size() || input[pos] != ':') return false;
    
    size_t line_end;
    if (!findCRLF(input, pos + 1, line_end)) return false;
    
    long long value;
    if (!safeStringToLongLong(input.substr(pos + 1, line_end - pos - 1), value)) return false;
    
    result = RESPValue(RESPType::INTEGER, value);
    pos = line_end + 2;
    return true;
}

// Parse Bulk String: $6\r\nfoobar\r\n or $-1\r\n (NULL)
bool RESPParser::parseBulkString(const std::string& input, size_t& pos, RESPValue& result) {
    if (pos >= input.size() || input[pos] != '$') return false;
    
    size_t len_end;
    if (!findCRLF(input, pos + 1, len_end)) return false;
    
    int len;
    if (!safeStringToInt(input.substr(pos + 1, len_end - pos - 1), len)) return false;
    pos = len_end + 2;
    
    if (len == -1) {  // NULL bulk string
        result = RESPValue(RESPType::NULL_VALUE, std::nullopt);
        return true;
    }
    
    if (len < 0 || static_cast<size_t>(len) > MAX_STRING_LENGTH) return false;
    if (pos + len + 2 > input.size()) return false;
    
    std::string str = input.substr(pos, len);
    pos += len;
    
    if (pos + 2 > input.size() || input[pos] != '\r' || input[pos + 1] != '\n')
        return false;
    pos += 2;
    
    result = RESPValue(RESPType::BULK_STRING, str);
    return true;
}

// Parse Array: *2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
bool RESPParser::parseArray(const std::string& input, size_t& pos, RESPValue& result, int depth) {
    if (pos >= input.size() || input[pos] != '*') return false;
    
    size_t line_end;
    if (!findCRLF(input, pos + 1, line_end)) return false;
    
    int num_elements;
    if (!safeStringToInt(input.substr(pos + 1, line_end - pos - 1), num_elements)) return false;
    
    if (num_elements == -1) {  // NULL array
        result = RESPValue(RESPType::NULL_VALUE, std::nullopt);
        pos = line_end + 2;
        return true;
    }
    
    if (num_elements < 0 || num_elements > MAX_ARGS) return false;
    pos = line_end + 2;
    
    std::vector<RESPValue> elements;
    elements.reserve(num_elements);
    
    for (int i = 0; i < num_elements; ++i) {
        RESPValue element(RESPType::NULL_VALUE);  // Initialize with explicit type
        if (!parseValue(input, pos, element, depth + 1)) return false;
        elements.push_back(std::move(element));
    }
    
    result = RESPValue(RESPType::ARRAY, elements);
    return true;
}

// Formatting methods
std::string RESPParser::format(const RESPValue& value) {
    switch (value.type) {
        case RESPType::SIMPLE_STRING:
            return formatSimpleString(value.getString());
        case RESPType::ERROR:
            return formatError(value.getString());
        case RESPType::INTEGER:
            return formatInteger(value.getInteger());
        case RESPType::BULK_STRING:
            return formatBulkString(value.getString());
        case RESPType::ARRAY:
            return formatArray(value.getArray());
        case RESPType::NULL_VALUE:
            return formatNull();
        default:
            return "";
    }
}

std::string RESPParser::formatBulkString(const std::string& str) {
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

std::string RESPParser::formatInteger(long long value) {
    return ":" + std::to_string(value) + "\r\n";
}

std::string RESPParser::formatSimpleString(const std::string& str) {
    return "+" + str + "\r\n";
}

std::string RESPParser::formatError(const std::string& error) {
    return "-" + error + "\r\n";
}

std::string RESPParser::formatArray(const std::vector<RESPValue>& items) {
    std::string response = "*" + std::to_string(items.size()) + "\r\n";
    for (const auto& item : items) {
        response += format(item);
    }
    return response;
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

// Utility methods
std::vector<std::string> RESPParser::toStringVector(const RESPValue& value) {
    std::vector<std::string> result;
    
    if (!value.isArray()) {
        if (value.isNull()) {
            result.push_back("");  // Represent NULL as empty string for compatibility
        } else if (value.isString() || value.isBulkString()) {
            result.push_back(value.getString());
        } else if (value.isInteger()) {
            result.push_back(std::to_string(value.getInteger()));
        }
        return result;
    }
    
    const auto& arr = value.getArray();
    result.reserve(arr.size());
    
    for (const auto& element : arr) {
        if (element.isNull()) {
            result.push_back("");  // NULL as empty string
        } else if (element.isString() || element.isBulkString()) {
            result.push_back(element.getString());
        } else if (element.isInteger()) {
            result.push_back(std::to_string(element.getInteger()));
        } else if (element.isArray()) {
            // Flatten nested arrays (basic approach)
            auto nested = toStringVector(element);
            result.insert(result.end(), nested.begin(), nested.end());
        }
    }
    
    return result;
}

void RESPParser::printValue(const RESPValue& value, int indent) {
    std::string indentStr(indent * 2, ' ');
    
    switch (value.type) {
        case RESPType::SIMPLE_STRING:
            std::cout << indentStr << "SimpleString: \"" << value.getString() << "\"" << std::endl;
            break;
        case RESPType::ERROR:
            std::cout << indentStr << "Error: \"" << value.getString() << "\"" << std::endl;
            break;
        case RESPType::INTEGER:
            std::cout << indentStr << "Integer: " << value.getInteger() << std::endl;
            break;
        case RESPType::BULK_STRING:
            std::cout << indentStr << "BulkString: \"" << value.getString() << "\"" << std::endl;
            break;
        case RESPType::ARRAY:
            std::cout << indentStr << "Array[" << value.getArray().size() << "]:" << std::endl;
            for (const auto& element : value.getArray()) {
                printValue(element, indent + 1);
            }
            break;
        case RESPType::NULL_VALUE:
            std::cout << indentStr << "NULL" << std::endl;
            break;
    }
}

// Private helper methods
bool RESPParser::findCRLF(const std::string& input, size_t start, size_t& end) {
    end = input.find("\r\n", start);
    return end != std::string::npos;
}

bool RESPParser::safeStringToInt(const std::string& str, int& result) {
    if (str.empty()) return false;
    
    try {
        size_t processed;
        long long val = std::stoll(str, &processed);
        
        if (processed != str.length()) return false;
        
        if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
            return false;
        }
        
        result = static_cast<int>(val);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool RESPParser::safeStringToLongLong(const std::string& str, long long& result) {
    if (str.empty()) return false;
    
    try {
        size_t processed;
        result = std::stoll(str, &processed);
        return processed == str.length();
    } catch (const std::exception&) {
        return false;
    }
}

// Plain text parser utility
std::vector<std::string> parsePlainText(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;
    
    while (iss >> token) {
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        if (!token.empty()) {
            result.push_back(token);
        }
    }
    return result;
}
