#pragma once
#include "../../../ecs/include/components.hpp"
#include "../../../ecs/include/network/NetworkCommands.hpp"
#include "../../../ecs/include/network/NetworkComponents.hpp"
#include "../../../ecs/include/registery.hpp"
#include "../../../ecs/include/render/IRenderWindow.hpp"
#include "../boss_manager.hpp"
#include "../enemy_manager.hpp"
#include "../player_manager.hpp"
#include "PacketProcessor.hpp"
#include <unordered_map>
#include <optional> 

/**
 * @class NetworkCommandHandler
 * @brief Handles network commands and manages entity creation/updates
 */
class NetworkCommandHandler : public network::INetworkCommandHandler {
  public:
    /**
     * @brief Construct command handler
     * @param registry Game registry
     * @param window Render window
     * @param player_mgr Player manager
     * @param enemy_mgr Enemy manager
     * @param boss_mgr Boss manager
     */
    NetworkCommandHandler(registry &registry, render::IRenderWindow &window,
                          PlayerManager &player_mgr, EnemyManager &enemy_mgr,
                          BossManager &boss_mgr);

    void onCreateEntity(const network::CreateEntityCommand &cmd) override;
    void onUpdateEntity(const network::UpdateEntityCommand &cmd) override;
    void onDestroyEntity(const network::DestroyEntityCommand &cmd) override;
    void onFullStateSync(const network::FullStateSyncCommand &cmd) override;
    void onConnectionStatusChanged(
        const network::ConnectionStatusCommand &cmd) override;
    void
    onPlayerAssignment(const network::PlayerAssignmentCommand &cmd) override;

    void onRawTCPMessage(const network::TCPMessage &msg) override;
    void onRawUDPPacket(const network::UDPPacket &packet) override;

    /**
     * @brief Find local entity by network ID
     * @param net_id Network ID
     * @return Optional entity
     */
    std::optional<entity> findEntityByNetId(uint32_t net_id) const;

    /**
     * @brief Get assigned player NET_ID
     * @return Player NET_ID or 0 if not assigned
     */
    uint32_t getAssignedPlayerNetId() const { return assigned_player_net_id_; }

  private:
    /**
     * @brief Create player entity from command
     * @param cmd Create entity command
     * @return Created entity
     */
    entity createPlayerEntity(const network::CreateEntityCommand &cmd);

    /**
     * @brief Create enemy entity from command
     * @param cmd Create entity command
     * @return Created entity
     */
    entity createEnemyEntity(const network::CreateEntityCommand &cmd);

    /**
     * @brief Create projectile entity from command
     * @param cmd Create entity command
     * @return Created entity
     */
    entity createProjectileEntity(const network::CreateEntityCommand &cmd);

    registry &registry_;
    render::IRenderWindow &window_;
    PlayerManager &player_manager_;
    EnemyManager &enemy_manager_;
    BossManager &boss_manager_;

    std::unordered_map<uint32_t, entity> net_id_to_entity_;
    network::PacketProcessor packet_processor_;
    uint32_t assigned_player_net_id_ = 0;
};