#pragma once
#include "INetwork.hpp"
#include "NetworkCommands.hpp"

namespace systems {

/**
 * @brief Initialize network system with abstract interface
 * @param manager Network interface implementation (INetwork*)
 */
void initialize_network_system(network::INetwork *manager);

/**
 * @brief Set network command handler
 * @param handler Command handler implementation
 */
void set_network_command_handler(network::INetworkCommandHandler *handler);

/**
 * @brief Network system - processes network messages and emits commands
 * @param dt Delta time
 */
void network_system(float dt);

/**
 * @brief Get current network manager
 * @return Pointer to network manager or nullptr
 */
network::INetwork *get_network_manager();

} // namespace systems