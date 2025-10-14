#pragma once
#include "../../../ecs/include/network/ANetworkManager.hpp"
#include "PacketProcessor.hpp"
#include <mutex>

namespace network {

// Client-side network manager with packet processing
class NetworkManager : public ANetworkManager {
  public:
    NetworkManager();
    ~NetworkManager() override = default;

    std::vector<TCPMessage> pollTCP() override;
    std::vector<UDPPacket> pollUDP() override;
    bool sendTCP(MessageType msg_type,
                 const std::vector<uint8_t> &data) override;
    bool sendUDP(const UDPPacket &packet) override;
    bool sendPlayerInput(const PlayerInputData &input) override;
    void update(float dt) override;

    uint32_t getAssignedPlayerNetId() const;
    void sendPlayerInput(uint8_t direction);

    struct NetworkEntity {
        uint8_t type;
        float pos_x;
        float pos_y;
        uint32_t health;
    };

    const std::unordered_map<uint32_t, NetworkEntity> &
    getNetworkEntities() const {
        return network_entities_;
    }

    uint32_t getAssignedNetId() const { return assigned_player_net_id_; }
    uint32_t getGameScore() const { return game_score_; }
    void processMessages();

  protected:
    void initializeUDPSocket() override;

  private:
    bool sendClientPing(uint32_t timestamp);
    void handlePlayerAssignment(const UDPPacket &packet);
    void resetConnectionState();

    PacketProcessor packet_processor_;

    mutable std::mutex player_mutex_;
    uint32_t assigned_player_net_id_ = 0;
    bool player_assigned_ = false;

    static constexpr int MAX_PING_RETRIES = 3;
    static constexpr float PING_RETRY_INTERVAL_S = 1.0f;
    static constexpr float PING_TOTAL_TIMEOUT_S = 10.0f;

    bool udp_ping_sent_ = false;
    int ping_retry_count_ = 0;
    std::chrono::steady_clock::time_point last_ping_time_;
    std::chrono::steady_clock::time_point ping_start_time_;

    std::unordered_map<uint32_t, NetworkEntity> network_entities_;
    uint32_t game_score_ = 0;
};

} // namespace network