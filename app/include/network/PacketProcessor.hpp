#pragma once
#include "../../../ecs/include/network/INetwork.hpp"
#include <queue>

namespace network {

// Handles packet sequencing and protocol parsing/serialization
class PacketProcessor {
  public:
    PacketProcessor();

    uint32_t getNextSendSequence();
    void addPacket(const UDPPacket &packet);
    std::vector<UDPPacket> getProcessedPackets();

    static TCPMessage parseTCPMessage(const std::vector<uint8_t> &data);
    static std::vector<uint8_t> serializeTCPMessage(const TCPMessage &msg);
    static UDPPacket parseUDPPacket(const std::vector<uint8_t> &data);
    static std::vector<uint8_t> serializeUDPPacket(const UDPPacket &packet);
    static EntityData parseEntityCreate(const std::vector<uint8_t> &data);
    static std::vector<EntityUpdateData>
    parseEntityUpdate(const std::vector<uint8_t> &data);
    static std::vector<uint32_t>
    parseEntityDestroy(const std::vector<uint8_t> &data);
    static std::vector<EntityData>
    parseGameState(const std::vector<uint8_t> &data);
    static uint32_t parsePlayerAssignment(const std::vector<uint8_t> &data);
    static std::vector<uint8_t>
    serializePlayerInput(const PlayerInputData &input);
    static UDPPacket createClientPing(uint32_t timestamp, uint8_t player_id);
    static uint32_t floatToNetwork(float value);
    static float networkToFloat(uint32_t value);

  private:
    uint32_t next_send_sequence_;
    uint32_t last_received_sequence_;
    std::queue<UDPPacket> processed_packets_;
};

} // namespace network