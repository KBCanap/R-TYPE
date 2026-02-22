#pragma once
#include "../../../ecs/include/components.hpp"
#include "../../../ecs/include/network/NetworkCommands.hpp"
#include "../../../ecs/include/network/NetworkComponents.hpp"
#include "../../../ecs/include/registery.hpp"
#include "../../../ecs/include/render/IRenderWindow.hpp"
#include "game/BossManager.hpp"
#include "game/EnemyManager.hpp"
#include "game/PlayerManager.hpp"
#include "PacketProcessor.hpp"
#include <atomic>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

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

    /**
     * @brief Get current player score (from server)
     * @return Player score
     */
    uint32_t getScore() const { return player_score_.load(); }

    /**
     * @brief Check if player entity was created at least once
     * @return true if player entity has been created
     */
    bool hasPlayerEntityBeenCreated() const { return player_entity_created_.load(); }

    /**
     * @brief Check if victory message was received from server
     * @return true if victory was achieved
     */
    bool hasVictory() const { return victory_received_.load(); }

    /**
     * @brief Check if beam is active for an entity
     * @param net_id Network ID to check
     * @return true if beam is active
     */
    bool isBeamActive(uint32_t net_id) const {
        return beam_active_net_ids_.count(net_id) > 0;
    }

    /**
     * @brief Get all entities that currently have an active beam
     * @return Vector of ECS entity handles with active beams
     */
    std::vector<entity> getBeamActiveEntities() const {
        std::vector<entity> result;
        std::lock_guard<std::mutex> lock(net_id_mutex_);
        for (uint32_t nid : beam_active_net_ids_) {
            auto it = net_id_to_entity_.find(nid);
            if (it != net_id_to_entity_.end()) {
                result.push_back(it->second);
            }
        }
        return result;
    }

  private:
    entity createPlayerEntity(const network::CreateEntityCommand &cmd);
    entity createEnemyEntity(const network::CreateEntityCommand &cmd);
    entity createEnemyLevel2Entity(const network::CreateEntityCommand &cmd);
    entity createEnemyLevel2SpreadEntity(const network::CreateEntityCommand &cmd);
    entity createEnemyKamikazeEntity(const network::CreateEntityCommand &cmd);

    /**
     * @brief Create boss entity from command
     * @param cmd Create entity command
     * @return Created entity
     */
    entity createBossEntity(const network::CreateEntityCommand &cmd);
    entity createBossLevel2Part1Entity(const network::CreateEntityCommand &cmd);
    entity createBossLevel2Part2Entity(const network::CreateEntityCommand &cmd);
    entity createBossLevel2Part3Entity(const network::CreateEntityCommand &cmd);
    entity createProjectileEntity(const network::CreateEntityCommand &cmd);
    entity createPowerUpEntity(const network::CreateEntityCommand &cmd);
    entity createPowerUpCompanionEntity(const network::CreateEntityCommand &cmd);
    entity createActiveCompanionEntity(const network::CreateEntityCommand &cmd);

    registry &registry_;
    render::IRenderWindow &window_;
    PlayerManager &player_manager_;
    EnemyManager &enemy_manager_;
    BossManager &boss_manager_;

    std::atomic<uint32_t> assigned_player_net_id_{0};
    std::atomic<uint32_t> player_score_{0};
    std::atomic<bool> player_entity_created_{false};
    std::atomic<bool> victory_received_{false};

    mutable std::mutex net_id_mutex_;
    std::unordered_map<uint32_t, entity> net_id_to_entity_;
    network::PacketProcessor packet_processor_;

    // Track power-up entities for collection detection
    std::unordered_map<uint32_t, int> powerup_net_id_to_type_;  // net_id -> type (0=shield, 1=spread)

    // Track which player entities have an active beam (laser powerup)
    std::unordered_set<uint32_t> beam_active_net_ids_;
};