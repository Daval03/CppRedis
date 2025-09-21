#include "../src/server/tcp_server.h"

int main() {

    try {
        TCPServer server(6379);
        std::cout << "Starting TCP server on port 6379..." << std::endl;
        std::cout << "Connect using: telnet localhost 6379" << std::endl;
        std::cout << "Type 'exit' to disconnect" << std::endl;
        
        server.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}