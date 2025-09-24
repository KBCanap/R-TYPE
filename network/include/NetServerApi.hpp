#pragma once
#include <string>
#include <vector>
#include <cstdint>

extern "C" {
    // TCP
    void* start_tcp_server(uint16_t port);
    std::vector<std::string> poll_tcp_messages(void* server);
    void stop_tcp_server(void* server);

    // UDP
    void* start_udp_server(uint16_t port);
    std::vector<std::string> poll_udp_messages(void* server);
    void stop_udp_server(void* server);
}
