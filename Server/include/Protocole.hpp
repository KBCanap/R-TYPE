/*
** EPITECH PROJECT, 2025
** Protocol.hpp
** File description:
** Protocol message handling and parsing
*/

#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include "MessageType.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct ParsedMessage {
    MessageType type;
    std::vector<uint8_t> data;
    bool valid;
};

class Protocol {
  public:
    Protocol();
    ~Protocol() = default;

    ParsedMessage parseMessage(const std::string &raw_message);

    std::string createConnectAck(uint8_t player_id);
    std::string createConnectNak(ConnectError error_code);
    std::string createGameStart(uint16_t udp_port);
    std::string createError(ProtocolError error_code);

  private:
    uint32_t extractDataLength(const uint8_t *header);
    std::string createMessage(MessageType type,
                              const std::vector<uint8_t> &data);

    bool isValidMessageType(uint8_t type);
    bool isValidDataLength(MessageType type, uint32_t length);
};

#endif /* !PROTOCOL_HPP_ */
