#include "utility_functions.h"


bool UtilityFunctions::isInteger(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        if (str.length() == 1) return false;
        start = 1;
    }
    
    for (size_t i = start; i < str.length(); i++) {
        if (!std::isdigit(str[i])) return false;
    }
    
    return true;
}

long long UtilityFunctions::parseInt(const std::string& str) {
    try {
        return std::stoll(str);
    } catch (...) {
        return 0;
    }
}

std::string UtilityFunctions::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool UtilityFunctions::matchPattern(const std::string& pattern, const std::string& str) {
    if (pattern == "*") return true;
    
    // Simple pattern matching - supports * and ?
    size_t p = 0, s = 0;
    size_t star_p = std::string::npos, star_s = 0;
    
    while (s < str.length()) {
        if (p < pattern.length() && (pattern[p] == '?' || pattern[p] == str[s])) {
            p++;
            s++;
        } else if (p < pattern.length() && pattern[p] == '*') {
            star_p = p++;
            star_s = s;
        } else if (star_p != std::string::npos) {
            p = star_p + 1;
            s = ++star_s;
        } else {
            return false;
        }
    }
    
    while (p < pattern.length() && pattern[p] == '*') {
        p++;
    }
    
    return p == pattern.length();
}

bool UtilityFunctions::isValidKey(const std::string& key) {
    return !key.empty() && key.length() < 512;//magic number
}