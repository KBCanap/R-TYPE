/*
** EPITECH PROJECT, 2025
** Protocol.cpp
** File description:
** Protocol message handling implementation - Extended Lobby System
*/

#include "Protocole.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

Protocol::Protocol() {}

ParsedMessage Protocol::parseMessage(const std::string &raw_message) {
    ParsedMessage result = {MessageType::TCP_ERROR, {}, false};

    if (raw_message.size() < HEADER_SIZE) {
        std::cerr << "Message too short: " << raw_message.size() << " bytes"
                  << std::endl;
        return result;
    }

    const uint8_t *data = reinterpret_cast<const uint8_t *>(raw_message.data());

    uint8_t msg_type = data[0];
    if (!isValidMessageType(msg_type)) {
        std::cerr << "Invalid message type: " << static_cast<int>(msg_type)
                  << std::endl;
        return result;
    }

    uint32_t data_length = extractDataLength(data);

    if (raw_message.size() != HEADER_SIZE + data_length) {
        std::cerr << "Invalid message size. Expected: "
                  << HEADER_SIZE + data_length
                  << ", Got: " << raw_message.size() << std::endl;
        return result;
    }

    MessageType type = static_cast<MessageType>(msg_type);

    std::vector<uint8_t> payload;
    if (data_length > 0) {
        payload.assign(data + HEADER_SIZE, data + HEADER_SIZE + data_length);
    }

    result.type = type;
    result.data = payload;
    result.valid = true;

    return result;
}

// Connection Messages
std::string Protocol::createConnect(const std::string &username) {
    std::vector<uint8_t> data;
    uint16_t username_len = htons(static_cast<uint16_t>(username.length()));

    data.push_back(static_cast<uint8_t>(username_len >> 8));
    data.push_back(static_cast<uint8_t>(username_len & 0xFF));

    for (char c : username) {
        data.push_back(static_cast<uint8_t>(c));
    }

    // Padding to 60 bytes
    while (data.size() < 2 + MAX_USERNAME_LENGTH) {
        data.push_back(0);
    }

    return createMessage(MessageType::TCP_CONNECT, data);
}

std::string Protocol::createConnectAck(uint8_t player_id) {
    std::vector<uint8_t> data = {player_id};
    return createMessage(MessageType::TCP_CONNECT_ACK, data);
}

std::string Protocol::createConnectNak(ConnectError error_code) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(error_code)};
    return createMessage(MessageType::TCP_CONNECT_NAK, data);
}

std::string Protocol::createReady() {
    return createMessage(MessageType::TCP_READY, {});
}

std::string Protocol::createGameStart(uint16_t udp_port, uint16_t server_id,
                                      uint32_t server_ip) {
    std::vector<uint8_t> data;

    uint16_t port_network = htons(udp_port);
    data.push_back(static_cast<uint8_t>(port_network >> 8));
    data.push_back(static_cast<uint8_t>(port_network & 0xFF));

    uint16_t server_id_network = htons(server_id);
    data.push_back(static_cast<uint8_t>(server_id_network >> 8));
    data.push_back(static_cast<uint8_t>(server_id_network & 0xFF));

    uint32_t ip_network = htonl(server_ip);
    data.push_back(static_cast<uint8_t>((ip_network >> 24) & 0xFF));
    data.push_back(static_cast<uint8_t>((ip_network >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((ip_network >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(ip_network & 0xFF));

    return createMessage(MessageType::TCP_GAME_START, data);
}

// Lobby Navigation Messages
std::string Protocol::createLobbyListRequest() {
    return createMessage(MessageType::LOBBY_LIST_REQUEST, {});
}

std::string
Protocol::createLobbyListResponse(const std::vector<LobbyInfo> &lobbies) {
    std::vector<uint8_t> data;

    uint16_t lobby_count = static_cast<uint16_t>(lobbies.size());
    data.push_back(static_cast<uint8_t>(lobby_count >> 8));
    data.push_back(static_cast<uint8_t>(lobby_count & 0xFF));

    for (const auto &lobby : lobbies) {
        std::vector<uint8_t> lobby_data = serializeLobbyInfo(lobby);
        data.insert(data.end(), lobby_data.begin(), lobby_data.end());
    }

    return createMessage(MessageType::LOBBY_LIST_RESPONSE, data);
}

std::string Protocol::createLobbyInfoRequest(uint16_t lobby_id) {
    std::vector<uint8_t> data;
    uint16_t lobby_id_network = htons(lobby_id);
    data.push_back(static_cast<uint8_t>(lobby_id_network >> 8));
    data.push_back(static_cast<uint8_t>(lobby_id_network & 0xFF));
    return createMessage(MessageType::LOBBY_INFO_REQUEST, data);
}

std::string Protocol::createLobbyInfoResponse(const LobbyInfo &lobby) {
    std::vector<uint8_t> data = serializeLobbyInfo(lobby);
    return createMessage(MessageType::LOBBY_INFO_RESPONSE, data);
}

// Join/Create Lobby Messages
std::string Protocol::createCreateLobby(const std::string &lobby_name,
                                        uint8_t max_players) {
    std::vector<uint8_t> data;

    data.push_back(max_players);

    uint16_t name_len = htons(static_cast<uint16_t>(lobby_name.length()));
    data.push_back(static_cast<uint8_t>(name_len >> 8));
    data.push_back(static_cast<uint8_t>(name_len & 0xFF));

    for (char c : lobby_name) {
        data.push_back(static_cast<uint8_t>(c));
    }

    // Padding to 32 bytes
    while (data.size() < 3 + MAX_LOBBY_NAME_LENGTH) {
        data.push_back(0);
    }

    return createMessage(MessageType::CREATE_LOBBY, data);
}

std::string Protocol::createCreateLobbyAck(uint16_t lobby_id) {
    std::vector<uint8_t> data;
    uint16_t lobby_id_network = htons(lobby_id);
    data.push_back(static_cast<uint8_t>(lobby_id_network >> 8));
    data.push_back(static_cast<uint8_t>(lobby_id_network & 0xFF));
    return createMessage(MessageType::CREATE_LOBBY_ACK, data);
}

std::string Protocol::createJoinLobby(uint16_t lobby_id) {
    std::vector<uint8_t> data;
    uint16_t lobby_id_network = htons(lobby_id);
    data.push_back(static_cast<uint8_t>(lobby_id_network >> 8));
    data.push_back(static_cast<uint8_t>(lobby_id_network & 0xFF));
    return createMessage(MessageType::JOIN_LOBBY, data);
}

std::string
Protocol::createJoinLobbyAck(uint16_t lobby_id, uint8_t your_player_id,
                             const std::vector<PlayerInfo> &players) {
    std::vector<uint8_t> data;

    uint16_t lobby_id_network = static_cast<uint16_t>(lobby_id);
    data.push_back(static_cast<uint8_t>(lobby_id_network >> 8));
    data.push_back(static_cast<uint8_t>(lobby_id_network & 0xFF));

    data.push_back(your_player_id);

    uint16_t player_count = static_cast<uint16_t>(players.size());
    data.push_back(static_cast<uint8_t>(player_count >> 8));
    data.push_back(static_cast<uint8_t>(player_count & 0xFF));

    for (const auto &player : players) {
        std::vector<uint8_t> player_data = serializePlayerInfo(player);
        data.insert(data.end(), player_data.begin(), player_data.end());
    }

    return createMessage(MessageType::JOIN_LOBBY_ACK, data);
}

std::string Protocol::createJoinLobbyNak(ProtocolError error_code) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(error_code)};
    return createMessage(MessageType::JOIN_LOBBY_NAK, data);
}

std::string Protocol::createLeaveLobby() {
    return createMessage(MessageType::LEAVE_LOBBY, {});
}

std::string Protocol::createLeaveLobbyAck() {
    return createMessage(MessageType::LEAVE_LOBBY_ACK, {});
}

// Lobby Player Management Messages
std::string Protocol::createPlayerJoined(const PlayerInfo &player) {
    std::vector<uint8_t> data = serializePlayerInfo(player);
    return createMessage(MessageType::PLAYER_JOINED, data);
}

std::string Protocol::createPlayerLeft(uint8_t player_id) {
    std::vector<uint8_t> data = {player_id};
    return createMessage(MessageType::PLAYER_LEFT, data);
}

// Game Session Management
std::string Protocol::createGameCancelled() {
    return createMessage(MessageType::GAME_CANCELLED, {});
}

// Error Messages
std::string Protocol::createError(ProtocolError error_code) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(error_code)};
    return createMessage(MessageType::TCP_ERROR, data);
}

// Helper functions for parsing
std::string Protocol::parseUsername(const std::vector<uint8_t> &data) {
    if (data.size() < 2)
        return "";

    std::cout << "DATA SIZE: " << data.size() << " data : " << data[2] << " "
              << data[3] << std::endl;
    uint16_t username_len;
    memcpy(&username_len, data.data(), sizeof(uint16_t));
    username_len = ntohs(username_len);


    std::cout << "USERNAME LENGTH: " << username_len << std::endl;
    if (data.size() < 2 + username_len)
        return "";

    return std::string(data.begin() + 2, data.begin() + 2 + username_len);
}

PlayerInfo Protocol::parsePlayerInfo(const std::vector<uint8_t> &data,
                                     std::size_t offset) {
    PlayerInfo player;
    std::memset(&player, 0, sizeof(PlayerInfo));

    if (data.size() < offset + 4)
        return player;

    player.player_id = data[offset];
    player.ready_flag = data[offset + 1];

    uint16_t username_len =
        (static_cast<uint16_t>(data[offset + 2]) << 8) | data[offset + 3];
    player.username_length = ntohs(username_len);

    std::size_t copy_len =
        std::min(static_cast<std::size_t>(player.username_length),
                 MAX_USERNAME_LENGTH - 1);
    if (data.size() >= offset + 4 + copy_len) {
        std::memcpy(player.username, &data[offset + 4], copy_len);
        player.username[copy_len] = '\0';
    }

    return player;
}

LobbyInfo Protocol::parseLobbyInfo(const std::vector<uint8_t> &data,
                                   std::size_t offset) {
    LobbyInfo lobby;
    std::memset(&lobby, 0, sizeof(LobbyInfo));

    if (data.size() < offset + 8)
        return lobby;

    lobby.lobby_id =
        (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];
    lobby.lobby_id = ntohs(lobby.lobby_id);

    lobby.player_count = data[offset + 2];
    lobby.max_players = data[offset + 3];

    lobby.lobby_name_length =
        (static_cast<uint16_t>(data[offset + 4]) << 8) | data[offset + 5];
    lobby.lobby_name_length = ntohs(lobby.lobby_name_length);

    std::size_t copy_len =
        std::min(static_cast<std::size_t>(lobby.lobby_name_length),
                 MAX_LOBBY_NAME_LENGTH - 1);
    if (data.size() >= offset + 6 + copy_len) {
        std::memcpy(lobby.lobby_name, &data[offset + 6], copy_len);
        lobby.lobby_name[copy_len] = '\0';
    }

    if (data.size() >= offset + 6 + MAX_LOBBY_NAME_LENGTH + 1) {
        lobby.status = data[offset + 6 + MAX_LOBBY_NAME_LENGTH];
    }

    return lobby;
}

std::vector<PlayerInfo>
Protocol::parsePlayerList(const std::vector<uint8_t> &data,
                          std::size_t offset) {
    std::vector<PlayerInfo> players;

    if (data.size() < offset + 2)
        return players;

    uint16_t player_count =
        (static_cast<uint16_t>(data[offset]) << 8) | data[offset + 1];
    player_count = ntohs(player_count);

    std::size_t current_offset = offset + 2;
    for (uint16_t i = 0; i < player_count; ++i) {
        if (data.size() < current_offset + 4 + MAX_USERNAME_LENGTH)
            break;

        PlayerInfo player = parsePlayerInfo(data, current_offset);
        players.push_back(player);

        current_offset += 4 + MAX_USERNAME_LENGTH;
    }

    return players;
}

std::vector<LobbyInfo>
Protocol::parseLobbyList(const std::vector<uint8_t> &data) {
    std::vector<LobbyInfo> lobbies;

    if (data.size() < 2)
        return lobbies;

    uint16_t lobby_count = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    lobby_count = ntohs(lobby_count);

    std::size_t current_offset = 2;
    for (uint16_t i = 0; i < lobby_count; ++i) {
        if (data.size() < current_offset + 8 + MAX_LOBBY_NAME_LENGTH)
            break;

        LobbyInfo lobby = parseLobbyInfo(data, current_offset);
        lobbies.push_back(lobby);

        current_offset +=
            8 + MAX_LOBBY_NAME_LENGTH + 4; // +4 for status and reserved
    }

    return lobbies;
}

// Private helper methods
uint32_t Protocol::extractDataLength(const uint8_t *header) {
    uint32_t length = 0;
    length |= (static_cast<uint32_t>(header[1]) << 16);
    length |= (static_cast<uint32_t>(header[2]) << 8);
    length |= static_cast<uint32_t>(header[3]);
    return length;
}

std::string Protocol::createMessage(MessageType type,
                                    const std::vector<uint8_t> &data) {
    std::string message;
    message.reserve(HEADER_SIZE + data.size());

    message.push_back(static_cast<uint8_t>(type));

    uint32_t length = data.size();
    message.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
    message.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
    message.push_back(static_cast<uint8_t>(length & 0xFF));

    message.insert(message.end(), data.begin(), data.end());

    return message;
}

bool Protocol::isValidMessageType(uint8_t type) {
    switch (static_cast<MessageType>(type)) {
    case MessageType::TCP_CONNECT:
    case MessageType::TCP_CONNECT_ACK:
    case MessageType::TCP_CONNECT_NAK:
    case MessageType::TCP_READY:
    case MessageType::TCP_GAME_START:
    case MessageType::LOBBY_LIST_REQUEST:
    case MessageType::LOBBY_LIST_RESPONSE:
    case MessageType::LOBBY_INFO_REQUEST:
    case MessageType::LOBBY_INFO_RESPONSE:
    case MessageType::CREATE_LOBBY:
    case MessageType::CREATE_LOBBY_ACK:
    case MessageType::JOIN_LOBBY:
    case MessageType::JOIN_LOBBY_ACK:
    case MessageType::JOIN_LOBBY_NAK:
    case MessageType::LEAVE_LOBBY:
    case MessageType::LEAVE_LOBBY_ACK:
    case MessageType::PLAYER_JOINED:
    case MessageType::PLAYER_LEFT:
    case MessageType::GAME_CANCELLED:
    case MessageType::TCP_ERROR:
        return true;
    default:
        return false;
    }
}

bool Protocol::isValidDataLength(MessageType type, uint32_t length) {
    return true;
}

std::vector<uint8_t> Protocol::serializePlayerInfo(const PlayerInfo &player) {
    std::vector<uint8_t> data;

    data.push_back(player.player_id);
    data.push_back(player.ready_flag);

    uint16_t username_len = htons(player.username_length);
    data.push_back(static_cast<uint8_t>(username_len >> 8));
    data.push_back(static_cast<uint8_t>(username_len & 0xFF));

    for (std::size_t i = 0; i < MAX_USERNAME_LENGTH; ++i) {
        data.push_back(static_cast<uint8_t>(player.username[i]));
    }

    return data;
}

std::vector<uint8_t> Protocol::serializeLobbyInfo(const LobbyInfo &lobby) {
    std::vector<uint8_t> data;

    uint16_t lobby_id_network = htons(lobby.lobby_id);
    data.push_back(static_cast<uint8_t>(lobby_id_network >> 8));
    data.push_back(static_cast<uint8_t>(lobby_id_network & 0xFF));

    data.push_back(lobby.player_count);
    data.push_back(lobby.max_players);

    uint16_t name_len = htons(lobby.lobby_name_length);
    data.push_back(static_cast<uint8_t>(name_len >> 8));
    data.push_back(static_cast<uint8_t>(name_len & 0xFF));

    for (std::size_t i = 0; i < MAX_LOBBY_NAME_LENGTH; ++i) {
        data.push_back(static_cast<uint8_t>(lobby.lobby_name[i]));
    }

    data.push_back(lobby.status);

    for (std::size_t i = 0; i < 3; ++i) {
        data.push_back(lobby.reserved[i]);
    }

    return data;
}
