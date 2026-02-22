/*
** EPITECH PROJECT, 2025
** GameSession.hpp
** File description:
** Game session management with extended lobby system
*/

#ifndef GAMESESSION_HPP_
#define GAMESESSION_HPP_

#include "network/protocol/MessageType.hpp"
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using ClientId = size_t;

/** @brief Represents a player in a lobby */
struct LobbyPlayer {
    ClientId client_id;     ///< Unique client connection ID
    uint8_t player_id;      ///< In-game player ID (1-4)
    std::string username;   ///< Player display name
    bool ready;             ///< Ready status for game start

    LobbyPlayer() : client_id(0), player_id(0), username(""), ready(false) {}
    LobbyPlayer(ClientId cid, uint8_t pid, const std::string &name)
        : client_id(cid), player_id(pid), username(name), ready(false) {}

    /** @brief Converts to network protocol format */
    PlayerInfo toPlayerInfo() const {
        PlayerInfo info;
        info.player_id = player_id;
        info.ready_flag = ready ? 1 : 0;
        info.username_length = htons(static_cast<uint16_t>(username.length()));
        std::memset(info.username, 0, MAX_USERNAME_LENGTH);
        std::strncpy(info.username, username.c_str(), MAX_USERNAME_LENGTH - 1);
        return info;
    }
};

/** @brief Represents a game lobby waiting room */
struct Lobby {
    uint16_t lobby_id;          ///< Unique lobby identifier
    std::string lobby_name;     ///< Display name
    uint8_t max_players;        ///< Maximum player capacity (1-4)
    LobbyStatus status;         ///< Current status (WAITING, IN_GAME, etc.)
    std::vector<LobbyPlayer> players; ///< Players in this lobby
    uint8_t level_id;           ///< Level selection (1=Level1, 2=Level2, 99=Endless)

    Lobby()
        : lobby_id(0), lobby_name(""), max_players(4),
          status(LobbyStatus::WAITING), level_id(1) {}

    Lobby(uint16_t id, const std::string &name, uint8_t max, uint8_t level = 1)
        : lobby_id(id), lobby_name(name), max_players(max),
          status(LobbyStatus::WAITING), level_id(level) {}

    /** @brief Checks if lobby is at max capacity */
    bool isFull() const { return players.size() >= max_players; }

    /** @brief Checks if lobby has no players */
    bool isEmpty() const { return players.empty(); }

    /** @brief Checks if all players are ready to start */
    bool allPlayersReady() const {
        if (players.empty())
            return false;
        for (const auto &player : players) {
            if (!player.ready)
                return false;
        }
        return true;
    }

    /** @brief Finds next available player ID slot (1-4)
     * @return Available player ID or nullopt if full */
    std::optional<uint8_t> getNextPlayerId() const {
        if (isFull())
            return std::nullopt;

        bool used[MAX_PLAYERS + 1] = {false};
        for (const auto &player : players) {
            if (player.player_id <= MAX_PLAYERS) {
                used[player.player_id] = true;
            }
        }

        for (uint8_t i = 1; i <= max_players; ++i) {
            if (!used[i])
                return i;
        }
        return std::nullopt;
    }

    /** @brief Converts to network protocol format */
    LobbyInfo toLobbyInfo() const {
        LobbyInfo info;
        info.lobby_id = htons(lobby_id);
        info.player_count = static_cast<uint8_t>(players.size());
        info.max_players = max_players;
        info.lobby_name_length =
            htons(static_cast<uint16_t>(lobby_name.length()));
        std::memset(info.lobby_name, 0, MAX_LOBBY_NAME_LENGTH);
        std::strncpy(info.lobby_name, lobby_name.c_str(),
                     MAX_LOBBY_NAME_LENGTH - 1);
        info.status = static_cast<uint8_t>(status);
        std::memset(info.reserved, 0, 3);
        return info;
    }
};

/**
 * @brief Manages client connections, lobbies, and game sessions
 *
 * Tracks clients across multiple lobbies and handles lobby lifecycle
 * from creation to game start.
 */
class GameSession {
  public:
    GameSession();
    ~GameSession() = default;

    // Client Management
    /** @brief Registers a new client connection
     * @return True if added successfully */
    bool addClient(ClientId client_id, const std::string &username);

    /** @brief Removes client and leaves their lobby if any */
    void removeClient(ClientId client_id);

    /** @brief Gets username for a client ID */
    std::optional<std::string> getClientUsername(ClientId client_id) const;

    /** @brief Checks if client is currently in a lobby */
    bool isClientInLobby(ClientId client_id) const;

    /** @brief Gets lobby ID client is in */
    std::optional<uint16_t> getClientLobby(ClientId client_id) const;

    // Lobby Creation & Destruction
    /** @brief Creates new lobby with creator as first player
     * @return Lobby ID or nullopt if failed */
    std::optional<uint16_t> createLobby(ClientId creator_id,
                                        const std::string &lobby_name,
                                        uint8_t max_players,
                                        uint8_t level_id = 1);

    /** @brief Deletes lobby (must be empty) */
    bool deleteLobby(uint16_t lobby_id);

    // Lobby Operations
    /** @brief Adds client to lobby
     * @return Assigned player ID (1-4) or nullopt if failed */
    std::optional<uint8_t> joinLobby(ClientId client_id, uint16_t lobby_id);

    /** @brief Removes client from their current lobby */
    bool leaveLobby(ClientId client_id);

    /** @brief Sets player ready status */
    bool setPlayerReady(ClientId client_id, bool ready = true);

    // Lobby Queries
    /** @brief Gets list of all active lobbies */
    std::vector<LobbyInfo> getAllLobbies() const;

    /** @brief Gets info for specific lobby */
    std::optional<LobbyInfo> getLobbyInfo(uint16_t lobby_id) const;

    /** @brief Gets full lobby object */
    std::optional<Lobby> getLobby(uint16_t lobby_id) const;

    /** @brief Gets list of players in lobby */
    std::vector<PlayerInfo> getLobbyPlayers(uint16_t lobby_id) const;

    // Game Start Conditions
    /** @brief Checks if lobby can start (all players ready) */
    bool canStartGame(uint16_t lobby_id) const;

    /** @brief Marks lobby as IN_GAME */
    bool startGame(uint16_t lobby_id);

    // Statistics
    /** @brief Returns number of active lobbies */
    size_t getLobbyCount() const { return _lobbies.size(); }

    /** @brief Returns number of connected clients */
    size_t getClientCount() const { return _clients.size(); }

  private:
    // Client data: ClientId -> username
    std::unordered_map<ClientId, std::string> _clients;

    // Client to Lobby mapping: ClientId -> LobbyId
    std::unordered_map<ClientId, uint16_t> _client_to_lobby;

    // Lobbies: LobbyId -> Lobby
    std::unordered_map<uint16_t, Lobby> _lobbies;

    // Next lobby ID counter
    uint16_t _next_lobby_id;

    // Helper methods
    /** @brief Generates next unique lobby ID */
    uint16_t getNextLobbyId();

    /** @brief Finds player in lobby (mutable version) */
    std::optional<LobbyPlayer *> findPlayerInLobby(uint16_t lobby_id,
                                                   ClientId client_id);

    /** @brief Finds player in lobby (const version) */
    std::optional<const LobbyPlayer *>
    findPlayerInLobby(uint16_t lobby_id, ClientId client_id) const;

    /** @brief Validates lobby name (length, characters) */
    bool isValidLobbyName(const std::string &name) const;

    /** @brief Validates username (length, characters) */
    bool isValidUsername(const std::string &name) const;

    /** @brief Validates max_players is in range (1-4) */
    bool isValidMaxPlayers(uint8_t max_players) const;
};

#endif /* !GAMESESSION_HPP_ */
