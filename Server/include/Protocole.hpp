/*
** EPITECH PROJECT, 2025
** Protocol.hpp
** File description:
** Protocol message handling and parsing - Extended Lobby System
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

    // Parsing
    ParsedMessage parseMessage(const std::string &raw_message);

    // Connection Messages
    std::string createConnect(const std::string &username);
    std::string createConnectAck(uint8_t player_id);
    std::string createConnectNak(ConnectError error_code);
    std::string createReady();
    std::string createGameStart(uint16_t udp_port, uint16_t server_id,
                                uint32_t server_ip);

    // Lobby Navigation Messages
    std::string createLobbyListRequest();
    std::string createLobbyListResponse(const std::vector<LobbyInfo> &lobbies);
    std::string createLobbyInfoRequest(uint16_t lobby_id);
    std::string createLobbyInfoResponse(const LobbyInfo &lobby);

    // Join/Create Lobby Messages
    std::string createCreateLobby(const std::string &lobby_name,
                                  uint8_t max_players);
    std::string createCreateLobbyAck(uint16_t lobby_id);
    std::string createJoinLobby(uint16_t lobby_id);
    std::string createJoinLobbyAck(uint16_t lobby_id, uint8_t your_player_id,
                                   const std::vector<PlayerInfo> &players);
    std::string createJoinLobbyNak(ProtocolError error_code);
    std::string createLeaveLobby();
    std::string createLeaveLobbyAck();

    // Lobby Player Management Messages
    std::string createPlayerJoined(const PlayerInfo &player);
    std::string createPlayerLeft(uint8_t player_id);

    // Game Session Management
    std::string createGameCancelled();

    // Error Messages
    std::string createError(ProtocolError error_code);

    // Helper functions for parsing specific message types
    std::string parseUsername(const std::vector<uint8_t> &data);
    PlayerInfo parsePlayerInfo(const std::vector<uint8_t> &data,
                               std::size_t offset);
    LobbyInfo parseLobbyInfo(const std::vector<uint8_t> &data,
                             std::size_t offset);
    std::vector<PlayerInfo> parsePlayerList(const std::vector<uint8_t> &data,
                                            std::size_t offset);
    std::vector<LobbyInfo> parseLobbyList(const std::vector<uint8_t> &data);

  private:
    uint32_t extractDataLength(const uint8_t *header);
    std::string createMessage(MessageType type,
                              const std::vector<uint8_t> &data);

    bool isValidMessageType(uint8_t type);
    bool isValidDataLength(MessageType type, uint32_t length);

    // Helper to serialize PlayerInfo
    std::vector<uint8_t> serializePlayerInfo(const PlayerInfo &player);

    // Helper to serialize LobbyInfo
    std::vector<uint8_t> serializeLobbyInfo(const LobbyInfo &lobby);
};

#endif /* !PROTOCOL_HPP_ */
