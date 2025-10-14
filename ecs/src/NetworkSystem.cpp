/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** NetworkSystem
*/

#include "../include/network/NetworkSystem.hpp"
#include <iostream>

namespace systems {

static network::INetwork *g_network_manager = nullptr;
static network::INetworkCommandHandler *g_command_handler = nullptr;

void initialize_network_system(network::INetwork *manager) {
    g_network_manager = manager;
}

void set_network_command_handler(network::INetworkCommandHandler *handler) {
    g_command_handler = handler;
}

network::INetwork *get_network_manager() { return g_network_manager; }

void network_system(float dt) {
    if (!g_network_manager || !g_command_handler) {
        return;
    }

    g_network_manager->update(dt);

    auto tcp_messages = g_network_manager->pollTCP();
    for (const auto &msg : tcp_messages) {
        g_command_handler->onRawTCPMessage(msg);
    }

    auto udp_packets = g_network_manager->pollUDP();
    for (const auto &packet : udp_packets) {
        g_command_handler->onRawUDPPacket(packet);
    }
}

} // namespace systems