#pragma once
#include "../../../ecs/include/network/INetwork.hpp"
#include <queue>

namespace network {

/**
 * @class PacketProcessor
 * @brief Handles packet sequencing and protocol parsing/serialization
 */
class PacketProcessor {
  public:
    /**
     * @brief Construct packet processor
     */
    PacketProcessor();

    /**
     * @brief Assign next sequence number for outgoing packet
     * @return New sequence number
     */
    uint32_t getNextSendSequence();

    /**
     * @brief Add packet to processing queue
     * @param packet Received packet
     */
    void addPacket(const UDPPacket &packet);

    /**
     * @brief Get packets ready for processing
     * @return Vector of packets in received order
     */
    std::vector<UDPPacket> getProcessedPackets();

    /**
     * @brief Parse TCP message from raw bytes
     * @param data Raw message data
     * @return Parsed TCP message
     */
    static TCPMessage parseTCPMessage(const std::vector<uint8_t> &data);

    /**
     * @brief Serialize TCP message to bytes
     * @param msg TCP message to serialize
     * @return Serialized bytes
     */
    static std::vector<uint8_t> serializeTCPMessage(const TCPMessage &msg);

    /**
     * @brief Parse UDP packet from raw bytes
     * @param data Raw packet data
     * @return Parsed UDP packet
     */
    static UDPPacket parseUDPPacket(const std::vector<uint8_t> &data);

    /**
     * @brief Serialize UDP packet to bytes
     * @param packet UDP packet to serialize
     * @return Serialized bytes
     */
    static std::vector<uint8_t> serializeUDPPacket(const UDPPacket &packet);

    /**
     * @brief Parse ENTITY_CREATE message
     * @param data Message payload
     * @return Entity data
     */
    static EntityData parseEntityCreate(const std::vector<uint8_t> &data);

    /**
     * @brief Parse ENTITY_UPDATE message
     * @param data Message payload
     * @return Vector of entity updates
     */
    static std::vector<EntityUpdateData>
    parseEntityUpdate(const std::vector<uint8_t> &data);

    /**
     * @brief Parse ENTITY_DESTROY message
     * @param data Message payload
     * @return Vector of NET_IDs to destroy
     */
    static std::vector<uint32_t>
    parseEntityDestroy(const std::vector<uint8_t> &data);

    /**
     * @brief Parse GAME_STATE message
     * @param data Message payload
     * @return Vector of all entities
     */
    static std::vector<EntityData>
    parseGameState(const std::vector<uint8_t> &data);

    /**
     * @brief Parse PLAYER_ASSIGNMENT message
     * @param data Message payload
     * @return Player NET_ID
     */
    static uint32_t parsePlayerAssignment(const std::vector<uint8_t> &data);

    /**
     * @brief Serialize PLAYER_INPUT message
     * @param input Player input data
     * @return Serialized payload
     */
    static std::vector<uint8_t>
    serializePlayerInput(const PlayerInputData &input);

    /**
     * @brief Create CLIENT_PING message
     * @param timestamp Client timestamp for latency calculation
     * @return Serialized CLIENT_PING packet
     */
    static UDPPacket createClientPing(uint32_t timestamp);

    /**
     * @brief Convert float to network byte order
     * @param value Float value
     * @return Network byte order uint32_t
     */
    static uint32_t floatToNetwork(float value);

    /**
     * @brief Convert network byte order to float
     * @param value Network byte order uint32_t
     * @return Float value
     */
    static float networkToFloat(uint32_t value);

  private:
    uint32_t next_send_sequence_;
    uint32_t last_received_sequence_;
    std::queue<UDPPacket> processed_packets_;
};

} // namespace network