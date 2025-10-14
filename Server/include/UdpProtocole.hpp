/*
** EPITECH PROJECT, 2025
** UdpProtocole.hpp
** File description:
** UDP Protocol message handling and parsing for R-Type
*/

#ifndef UDPPROTOCOLE_HPP_
#define UDPPROTOCOLE_HPP_

#include "UdpMessageType.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct ParsedUdpMessage {
    UdpMessageType type;
    uint32_t sequence_num;
    std::vector<uint8_t> data;
    bool valid;
};

struct Entity {
    uint32_t net_id;
    EntityType type;
    uint32_t health;
    float position_x;
    float position_y;
};

class UdpProtocole {
  public:
    UdpProtocole();
    ~UdpProtocole() = default;

    ParsedUdpMessage parseMessage(const std::string &raw_message);

    std::string createPlayerAssignment(uint32_t player_net_id,
                                       uint32_t sequence_num);
    std::string createEntityCreate(const Entity &entity, uint32_t sequence_num);
    std::string createEntityUpdate(const std::vector<Entity> &entities,
                                   uint32_t sequence_num);
    std::string createEntityDestroy(const std::vector<uint32_t> &net_ids,
                                    uint32_t sequence_num);
    std::string createGameState(const std::vector<Entity> &entities,
                                uint32_t sequence_num);

    static uint32_t extractHealth24bit(const uint8_t *data);
    static void packHealth24bit(uint8_t *dest, uint32_t health);
    static float ntohf(uint32_t netfloat);
    static uint32_t htonf(float hostfloat);

  private:
    uint32_t extractDataLength(const uint8_t *header);
    uint32_t extractSequenceNum(const uint8_t *header);
    std::string createMessage(UdpMessageType type, uint32_t sequence_num,
                              const std::vector<uint8_t> &data);

    bool isValidMessageType(uint8_t type);
    bool isValidDataLength(UdpMessageType type, uint32_t length);
};

#endif /* !UDPPROTOCOLE_HPP_ */
