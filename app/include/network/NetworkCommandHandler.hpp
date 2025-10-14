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
#include <atomic>
#include <mutex>
#include <optional>
#include <unordered_map>

// Handles network commands and manages entity creation/updates
class NetworkCommandHandler : public network::INetworkCommandHandler {
  public:
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

    std::optional<entity> findEntityByNetId(uint32_t net_id) const;

    /**
     * @brief Get assigned player NET_ID
     * @return Player NET_ID or 0 if not assigned
     */
    uint32_t getAssignedPlayerNetId() const { 
        return assigned_player_net_id_.load();
    }

  private:
    entity createPlayerEntity(const network::CreateEntityCommand &cmd);
    entity createEnemyEntity(const network::CreateEntityCommand &cmd);

    /**
     * @brief Create boss entity from command
     * @param cmd Create entity command
     * @return Created entity
     */
    entity createBossEntity(const network::CreateEntityCommand &cmd);
    entity createProjectileEntity(const network::CreateEntityCommand &cmd);

    registry &registry_;
    render::IRenderWindow &window_;
    PlayerManager &player_manager_;
    EnemyManager &enemy_manager_;
    BossManager &boss_manager_;

    std::atomic<uint32_t> assigned_player_net_id_{0};

    mutable std::mutex net_id_mutex_;
    std::unordered_map<uint32_t, entity> net_id_to_entity_;
    network::PacketProcessor packet_processor_;
};