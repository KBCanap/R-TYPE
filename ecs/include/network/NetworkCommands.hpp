#pragma once
#include "INetwork.hpp"
#include <functional>
#include <vector>

namespace network {

/**
 * @enum NetworkCommandType
 * @brief Types of commands issued by network system
 */
enum class NetworkCommandType {
    CREATE_ENTITY,
    UPDATE_ENTITY,
    DESTROY_ENTITY,
    FULL_STATE_SYNC,
    CONNECTION_STATUS_CHANGED,
    PLAYER_ASSIGNED
};

/**
 * @struct CreateEntityCommand
 * @brief Command to create a new networked entity
 */
struct CreateEntityCommand {
    uint32_t net_id;
    EntityType entity_type;
    uint32_t health;
    float position_x;
    float position_y;
};

/**
 * @struct UpdateEntityCommand
 * @brief Command to update an existing entity
 */
struct UpdateEntityCommand {
    uint32_t net_id;
    uint32_t health;
    float position_x;
    float position_y;
};

/**
 * @struct DestroyEntityCommand
 * @brief Command to destroy an entity
 */
struct DestroyEntityCommand {
    uint32_t net_id;
};

/**
 * @struct FullStateSyncCommand
 * @brief Command for full game state synchronization
 */
struct FullStateSyncCommand {
    std::vector<CreateEntityCommand> entities;
};

/**
 * @struct ConnectionStatusCommand
 * @brief Command for connection status changes
 */
struct ConnectionStatusCommand {
    ConnectionState old_state;
    ConnectionState new_state;
    uint8_t player_id;
    uint16_t udp_port;
};

/**
 * @struct PlayerAssignmentCommand
 * @brief Command for player NET_ID assignment
 */
struct PlayerAssignmentCommand {
    uint32_t player_net_id;
};

/**
 * @class INetworkCommandHandler
 * @brief Interface for handling network commands
 */
class INetworkCommandHandler {
  public:
    virtual ~INetworkCommandHandler() = default;

    /**
     * @brief Handle entity creation command
     * @param cmd Create entity command
     */
    virtual void onCreateEntity(const CreateEntityCommand &cmd) = 0;

    /**
     * @brief Handle entity update command
     * @param cmd Update entity command
     */
    virtual void onUpdateEntity(const UpdateEntityCommand &cmd) = 0;

    /**
     * @brief Handle entity destruction command
     * @param cmd Destroy entity command
     */
    virtual void onDestroyEntity(const DestroyEntityCommand &cmd) = 0;

    /**
     * @brief Handle full state synchronization
     * @param cmd Full state sync command
     */
    virtual void onFullStateSync(const FullStateSyncCommand &cmd) = 0;

    /**
     * @brief Handle connection status change
     * @param cmd Connection status command
     */
    virtual void
    onConnectionStatusChanged(const ConnectionStatusCommand &cmd) = 0;

    /**
     * @brief Handle player NET_ID assignment
     * @param cmd Player assignment command
     */
    virtual void onPlayerAssignment(const PlayerAssignmentCommand &cmd) = 0;

    /**
     * @brief Handle raw TCP message from network layer
     * @param msg Raw TCP message
     */
    virtual void onRawTCPMessage(const TCPMessage &msg) = 0;

    /**
     * @brief Handle raw UDP packet from network layer
     * @param packet Raw UDP packet
     */
    virtual void onRawUDPPacket(const UDPPacket &packet) = 0;
};

} // namespace network