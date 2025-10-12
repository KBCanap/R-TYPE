#include "../include/NetServer.hpp"
#include <cstdint>

extern "C" {
void *start_tcp_server(uint16_t port) { return new TCPServer(port); }

std::vector<ClientMessage> poll_tcp_messages(void *server) {
    return static_cast<TCPServer *>(server)->poll();
}

void stop_tcp_server(void *server) { delete static_cast<TCPServer *>(server); }

bool send_to_client(void *server, uint32_t client_id, const char *message) {
    return static_cast<TCPServer *>(server)->sendToClient(client_id,
                                                          std::string(message));
}

void disconnect_client(void *server, uint32_t client_id) {
    static_cast<TCPServer *>(server)->disconnectClient(client_id);
}

std::vector<uint32_t> get_connected_clients(void *server) {
    return static_cast<TCPServer *>(server)->getConnectedClients();
}

void *start_udp_server(uint16_t port) { return new UDPServer(port); }

std::vector<std::string> poll_udp_messages(void *server) {
    return static_cast<UDPServer *>(server)->poll();
}

void stop_udp_server(void *server) { delete static_cast<UDPServer *>(server); }
}