#ifndef RESP_VALUE_H
#define RESP_VALUE_H

#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "enum/resp_type.h"

// RESP2 Value representation
struct RESPValue {
    RESPType type;
    std::variant<std::string, long long, std::vector<RESPValue>, std::nullopt_t> data;
    
    // Constructors
    RESPValue();
    RESPValue(RESPType t);
    RESPValue(RESPType t, const std::string& s);
    RESPValue(RESPType t, long long i);
    RESPValue(RESPType t, const std::vector<RESPValue>& arr);
    RESPValue(RESPType t, std::nullopt_t);
    
    // Helper methods
    bool isString() const;
    bool isInteger() const;
    bool isBulkString() const;
    bool isArray() const;
    bool isNull() const;
    bool isError() const;
    
    std::string getString() const;
    long long getInteger() const;
    const std::vector<RESPValue>& getArray() const;
};

#endif // RESP_VALUE_H