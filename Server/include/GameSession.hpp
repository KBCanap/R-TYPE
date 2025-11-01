/*
** EPITECH PROJECT, 2025
** GameSession.hpp
** File description:
** Game session management with extended lobby system
*/

#ifndef GAMESESSION_HPP_
#define GAMESESSION_HPP_

#include "MessageType.hpp"
#include <cstring>
// Platform-specific network includes
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using ClientId = size_t;

// Structure pour représenter un joueur dans un lobby
struct LobbyPlayer {
    ClientId client_id;
    uint8_t player_id;
    std::string username;
    bool ready;
    
    LobbyPlayer() : client_id(0), player_id(0), username(""), ready(false) {}
    LobbyPlayer(ClientId cid, uint8_t pid, const std::string &name)
        : client_id(cid), player_id(pid), username(name), ready(false) {}
    
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

// Structure pour représenter un lobby
struct Lobby {
    uint16_t lobby_id;
    std::string lobby_name;
    uint8_t max_players;
    LobbyStatus status;
    std::vector<LobbyPlayer> players;
    
    Lobby() : lobby_id(0), lobby_name(""), max_players(4), status(LobbyStatus::WAITING) {}
    
    Lobby(uint16_t id, const std::string &name, uint8_t max)
        : lobby_id(id), lobby_name(name), max_players(max), status(LobbyStatus::WAITING) {}
    
    bool isFull() const { return players.size() >= max_players; }
    
    bool isEmpty() const { return players.empty(); }
    
    bool allPlayersReady() const {
        if (players.empty()) return false;
        for (const auto &player : players) {
            if (!player.ready) return false;
        }
        return true;
    }
    
    std::optional<uint8_t> getNextPlayerId() const {
        if (isFull()) return std::nullopt;
        
        bool used[MAX_PLAYERS + 1] = {false};
        for (const auto &player : players) {
            if (player.player_id <= MAX_PLAYERS) {
                used[player.player_id] = true;
            }
        }
        
        for (uint8_t i = 1; i <= max_players; ++i) {
            if (!used[i]) return i;
        }
        return std::nullopt;
    }
    
    LobbyInfo toLobbyInfo() const {
        LobbyInfo info;
        info.lobby_id = htons(lobby_id);
        info.player_count = static_cast<uint8_t>(players.size());
        info.max_players = max_players;
        info.lobby_name_length = htons(static_cast<uint16_t>(lobby_name.length()));
        std::memset(info.lobby_name, 0, MAX_LOBBY_NAME_LENGTH);
        std::strncpy(info.lobby_name, lobby_name.c_str(), MAX_LOBBY_NAME_LENGTH - 1);
        info.status = static_cast<uint8_t>(status);
        std::memset(info.reserved, 0, 3);
        return info;
    }
};

class GameSession {
  public:
    GameSession();
    ~GameSession() = default;

    // Client Management
    bool addClient(ClientId client_id, const std::string &username);
    void removeClient(ClientId client_id);
    std::optional<std::string> getClientUsername(ClientId client_id) const;
    bool isClientInLobby(ClientId client_id) const;
    std::optional<uint16_t> getClientLobby(ClientId client_id) const;

    // Lobby Creation & Destruction
    std::optional<uint16_t> createLobby(ClientId creator_id, const std::string &lobby_name, 
                                        uint8_t max_players);
    bool deleteLobby(uint16_t lobby_id);

    // Lobby Operations
    std::optional<uint8_t> joinLobby(ClientId client_id, uint16_t lobby_id);
    bool leaveLobby(ClientId client_id);
    bool setPlayerReady(ClientId client_id, bool ready = true);

    // Lobby Queries
    std::vector<LobbyInfo> getAllLobbies() const;
    std::optional<LobbyInfo> getLobbyInfo(uint16_t lobby_id) const;
    std::optional<Lobby> getLobby(uint16_t lobby_id) const;
    std::vector<PlayerInfo> getLobbyPlayers(uint16_t lobby_id) const;

    // Game Start Conditions
    bool canStartGame(uint16_t lobby_id) const;
    bool startGame(uint16_t lobby_id);

    // Statistics
    size_t getLobbyCount() const { return _lobbies.size(); }
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
    uint16_t getNextLobbyId();
    std::optional<LobbyPlayer*> findPlayerInLobby(uint16_t lobby_id, ClientId client_id);
    std::optional<const LobbyPlayer*> findPlayerInLobby(uint16_t lobby_id, ClientId client_id) const;
    bool isValidLobbyName(const std::string &name) const;
    bool isValidUsername(const std::string &name) const;
    bool isValidMaxPlayers(uint8_t max_players) const;
};

#endif /* !GAMESESSION_HPP_ */
