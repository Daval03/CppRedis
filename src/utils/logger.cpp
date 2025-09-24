#include "logger.h"


void logCommand(const std::vector<std::string>& args, size_t bytes_consumed) {
    std::cout << "🐛 [CMD] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i == 0) {
            std::cout << "\033[1;32m" << args[i] << "\033[0m"; // Command in green
        } else {
            std::cout << "\033[1;34m" << args[i] << "\033[0m"; // Arguments in blue
        }
        if (i < args.size() - 1) std::cout << " ";
    }
    std::cout << " (\033[1;33m" << bytes_consumed << " bytes\033[0m)" << std::endl;
}

void logResponse(const std::string& response) {
    std::string preview = response.substr(0, std::min(size_t(50), response.size()));
    std::cout << "📤 [RESP] \033[0;35m" << preview << "\033[0m";
    if (response.size() > 50) std::cout << "...";
    std::cout << " (\033[1;33m" << response.size() << " bytes\033[0m)" << std::endl;
}

void logConnection(const std::string& action, const std::string& client_ip, int client_port) {
    if (!client_ip.empty() && client_port != 0) {
        std::cout << "🔌 [" << action << "] " << client_ip << ":" << client_port << std::endl;
    } else {
        std::cout << "🔌 [" << action << "]" << std::endl;
    }
}

void logError(const std::string& error_message) {
    std::cout << "❌ [ERROR] \033[1;31m" << error_message << "\033[0m" << std::endl;
}