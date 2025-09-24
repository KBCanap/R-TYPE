#include "NetServer.hpp"
#include <cstdint>

extern "C" {
    void* start_tcp_server(uint16_t port) {
        return new TCPServer(port);
    }

    std::vector<std::string> poll_tcp_messages(void* server) {
        return static_cast<TCPServer*>(server)->poll();
    }

    void stop_tcp_server(void* server) {
        delete static_cast<TCPServer*>(server);
    }

    void* start_udp_server(uint16_t port) {
        return new UDPServer(port);
    }

    std::vector<std::string> poll_udp_messages(void* server) {
        return static_cast<UDPServer*>(server)->poll();
    }

    void stop_udp_server(void* server) {
        delete static_cast<UDPServer*>(server);
    }
}
