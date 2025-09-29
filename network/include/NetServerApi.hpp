#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct ClientMessage {
    std::string client_endpoint;
    std::string message;
    uint32_t client_id;
};

extern "C" {
    // TCP
    void* start_tcp_server(uint16_t port);
    std::vector<ClientMessage> poll_tcp_messages(void* server);
    void stop_tcp_server(void* server);
    bool send_to_client(void* server, uint32_t client_id, const char* message);
    void disconnect_client(void* server, uint32_t client_id);
    std::vector<uint32_t> get_connected_clients(void* server);

    // UDP (inchangé)
    void* start_udp_server(uint16_t port);
    std::vector<std::string> poll_udp_messages(void* server);
    void stop_udp_server(void* server);
}
