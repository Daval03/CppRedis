#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
void logCommand(const std::vector<std::string>& args, size_t bytes_consumed);
void logResponse(const std::string& response);
void logConnection(const std::string& action, const std::string& client_ip = "", int client_port = 0);
void logError(const std::string& error_message);

#endif // LOGGER_H