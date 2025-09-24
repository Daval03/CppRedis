#include "resp_value.h"

// Constructors
RESPValue::RESPValue() : type(RESPType::NULL_VALUE), data(std::nullopt) {}

RESPValue::RESPValue(RESPType t) : type(t) {
    // Initialize data based on type
    if (t == RESPType::NULL_VALUE) {
        data = std::nullopt;
    } else if (t == RESPType::INTEGER) {
        data = 0LL;
    } else if (t == RESPType::ARRAY) {
        data = std::vector<RESPValue>();
    }
    // For string types, data remains uninitialized (caller should set it)
}

RESPValue::RESPValue(RESPType t, const std::string& s) : type(t), data(s) {}

RESPValue::RESPValue(RESPType t, long long i) : type(t), data(i) {}

RESPValue::RESPValue(RESPType t, const std::vector<RESPValue>& arr) : type(t), data(arr) {}

RESPValue::RESPValue(RESPType t, std::nullopt_t) : type(t), data(std::nullopt) {}

// Helper methods
bool RESPValue::isString() const { 
    return type == RESPType::SIMPLE_STRING || type == RESPType::ERROR || type == RESPType::BULK_STRING; 
}

bool RESPValue::isInteger() const { 
    return type == RESPType::INTEGER; 
}

bool RESPValue::isBulkString() const { 
    return type == RESPType::BULK_STRING; 
}

bool RESPValue::isArray() const { 
    return type == RESPType::ARRAY; 
}

bool RESPValue::isNull() const { 
    return type == RESPType::NULL_VALUE; 
}

bool RESPValue::isError() const { 
    return type == RESPType::ERROR; 
}

std::string RESPValue::getString() const {
    if (std::holds_alternative<std::string>(data)) {
        return std::get<std::string>(data);
    }
    return "";
}

long long RESPValue::getInteger() const {
    if (std::holds_alternative<long long>(data)) {
        return std::get<long long>(data);
    }
    return 0;
}

const std::vector<RESPValue>& RESPValue::getArray() const {
    return std::get<std::vector<RESPValue>>(data);
}