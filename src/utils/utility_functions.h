#pragma once
#include <string>
#include <cctype>
#include <algorithm>

class UtilityFunctions {
public:
    static bool isInteger(const std::string& str);
    static long long parseInt(const std::string& str);
    static std::string toUpper(const std::string& str);
    static bool matchPattern(const std::string& pattern, const std::string& str);
    static bool isValidKey(const std::string& key);
};