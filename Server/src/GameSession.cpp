/*
** EPITECH PROJECT, 2025
** GameSession.cpp
** File description:
** Implementation of game session management
*/

#include "GameSession.hpp"
#include <algorithm>
#include <iostream>

GameSession::GameSession() : _next_lobby_id(1) {}

bool GameSession::addClient(ClientId client_id, const std::string &username) {
    if (!isValidUsername(username)) {
        std::cerr << "Invalid username: " << username << std::endl;
        return false;
    }

    if (_clients.find(client_id) != _clients.end()) {
        std::cerr << "Client " << client_id << " already exists" << std::endl;
        return false;
    }

    _clients[client_id] = username;
    std::cout << "Client " << client_id << " added with username: " << username
              << std::endl;
    return true;
}

void GameSession::removeClient(ClientId client_id) {
    auto it = _clients.find(client_id);
    if (it == _clients.end()) {
        return;
    }

    leaveLobby(client_id);

    _clients.erase(it);
    std::cout << "Client " << client_id << " removed" << std::endl;
}

std::optional<std::string>
GameSession::getClientUsername(ClientId client_id) const {
    auto it = _clients.find(client_id);
    if (it != _clients.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool GameSession::isClientInLobby(ClientId client_id) const {
    return _client_to_lobby.find(client_id) != _client_to_lobby.end();
}

std::optional<uint16_t> GameSession::getClientLobby(ClientId client_id) const {
    auto it = _client_to_lobby.find(client_id);
    if (it != _client_to_lobby.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<uint16_t> GameSession::createLobby(ClientId creator_id,
                                                 const std::string &lobby_name,
                                                 uint8_t max_players) {
    // Validate client exists
    if (_clients.find(creator_id) == _clients.end()) {
        std::cerr << "Client " << creator_id << " not found" << std::endl;
        return std::nullopt;
    }

    // Check if client is already in a lobby
    if (isClientInLobby(creator_id)) {
        std::cerr << "Client " << creator_id << " is already in a lobby"
                  << std::endl;
        return std::nullopt;
    }

    // Validate parameters
    if (!isValidLobbyName(lobby_name)) {
        std::cerr << "Invalid lobby name: " << lobby_name << std::endl;
        return std::nullopt;
    }

    if (!isValidMaxPlayers(max_players)) {
        std::cerr << "Invalid max_players: " << static_cast<int>(max_players)
                  << std::endl;
        return std::nullopt;
    }

    // Create lobby
    uint16_t lobby_id = getNextLobbyId();
    Lobby lobby(lobby_id, lobby_name, max_players);

    // Add creator as first player (player_id = 1)
    std::string username = _clients[creator_id];
    LobbyPlayer creator(creator_id, 1, username);
    lobby.players.push_back(creator);

    // Store lobby
    _lobbies[lobby_id] = lobby;
    _client_to_lobby[creator_id] = lobby_id;

    std::cout << "Lobby " << lobby_id << " created by client " << creator_id
              << " (name: " << lobby_name
              << ", max: " << static_cast<int>(max_players) << ")" << std::endl;

    return lobby_id;
}

bool GameSession::deleteLobby(uint16_t lobby_id) {
    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return false;
    }

    // Remove all players from client_to_lobby mapping
    for (const auto &player : it->second.players) {
        _client_to_lobby.erase(player.client_id);
    }

    _lobbies.erase(it);
    std::cout << "Lobby " << lobby_id << " deleted" << std::endl;
    return true;
}

std::optional<uint8_t> GameSession::joinLobby(ClientId client_id,
                                              uint16_t lobby_id) {
    // Validate client exists
    if (_clients.find(client_id) == _clients.end()) {
        std::cerr << "Client " << client_id << " not found" << std::endl;
        return std::nullopt;
    }

    // Check if client is already in a lobby
    if (isClientInLobby(client_id)) {
        std::cerr << "Client " << client_id << " is already in a lobby"
                  << std::endl;
        return std::nullopt;
    }

    // Find lobby
    auto lobby_it = _lobbies.find(lobby_id);
    if (lobby_it == _lobbies.end()) {
        std::cerr << "Lobby " << lobby_id << " not found" << std::endl;
        return std::nullopt;
    }

    Lobby &lobby = lobby_it->second;

    // Check if lobby is full
    if (lobby.isFull()) {
        std::cerr << "Lobby " << lobby_id << " is full" << std::endl;
        return std::nullopt;
    }

    // Check if lobby has already started
    if (lobby.status == LobbyStatus::IN_GAME) {
        std::cerr << "Lobby " << lobby_id << " game already started"
                  << std::endl;
        return std::nullopt;
    }

    // Get next available player ID
    auto player_id = lobby.getNextPlayerId();
    if (!player_id.has_value()) {
        std::cerr << "No available player ID in lobby " << lobby_id
                  << std::endl;
        return std::nullopt;
    }

    // Add player to lobby
    std::string username = _clients[client_id];
    LobbyPlayer player(client_id, player_id.value(), username);
    lobby.players.push_back(player);

    // Update mapping
    _client_to_lobby[client_id] = lobby_id;

    std::cout << "Client " << client_id << " joined lobby " << lobby_id
              << " as player " << static_cast<int>(player_id.value())
              << std::endl;

    return player_id.value();
}

bool GameSession::leaveLobby(ClientId client_id) {
    // Check if client is in a lobby
    auto lobby_id_opt = getClientLobby(client_id);
    if (!lobby_id_opt.has_value()) {
        return false;
    }

    uint16_t lobby_id = lobby_id_opt.value();
    auto lobby_it = _lobbies.find(lobby_id);
    if (lobby_it == _lobbies.end()) {
        // Cleanup invalid mapping
        _client_to_lobby.erase(client_id);
        return false;
    }

    Lobby &lobby = lobby_it->second;

    // Remove player from lobby
    auto player_it = std::find_if(
        lobby.players.begin(), lobby.players.end(),
        [client_id](const LobbyPlayer &p) { return p.client_id == client_id; });

    if (player_it == lobby.players.end()) {
        // Player not found in lobby (shouldn't happen)
        _client_to_lobby.erase(client_id);
        return false;
    }

    uint8_t player_id = player_it->player_id;
    lobby.players.erase(player_it);
    _client_to_lobby.erase(client_id);

    std::cout << "Client " << client_id << " (player "
              << static_cast<int>(player_id) << ") left lobby " << lobby_id
              << std::endl;

    // Delete lobby if empty
    if (lobby.isEmpty()) {
        deleteLobby(lobby_id);
    }

    return true;
}

bool GameSession::setPlayerReady(ClientId client_id, bool ready) {
    // Check if client is in a lobby
    auto lobby_id_opt = getClientLobby(client_id);
    if (!lobby_id_opt.has_value()) {
        std::cerr << "Client " << client_id << " is not in a lobby"
                  << std::endl;
        return false;
    }

    uint16_t lobby_id = lobby_id_opt.value();
    auto player = findPlayerInLobby(lobby_id, client_id);
    if (!player.has_value()) {
        std::cerr << "Player not found in lobby" << std::endl;
        return false;
    }

    player.value()->ready = ready;
    std::cout << "Client " << client_id << " ready status set to " << ready
              << std::endl;

    // Update lobby status
    auto lobby_it = _lobbies.find(lobby_id);
    if (lobby_it != _lobbies.end()) {
        if (lobby_it->second.allPlayersReady() && !lobby_it->second.isEmpty()) {
            lobby_it->second.status = LobbyStatus::READY;
            std::cout << "Lobby " << lobby_id << " is now READY" << std::endl;
        } else if (lobby_it->second.status == LobbyStatus::READY) {
            lobby_it->second.status = LobbyStatus::WAITING;
            std::cout << "Lobby " << lobby_id << " is now WAITING" << std::endl;
        }
    }

    return true;
}

std::vector<LobbyInfo> GameSession::getAllLobbies() const {
    std::vector<LobbyInfo> lobbies;
    lobbies.reserve(_lobbies.size());

    for (const auto &[id, lobby] : _lobbies) {
        // Only include lobbies that are waiting or ready (not in-game or closing)
        if (lobby.status == LobbyStatus::WAITING || lobby.status == LobbyStatus::READY) {
            lobbies.push_back(lobby.toLobbyInfo());
        }
    }

    return lobbies;
}

std::optional<LobbyInfo> GameSession::getLobbyInfo(uint16_t lobby_id) const {
    auto it = _lobbies.find(lobby_id);
    if (it != _lobbies.end()) {
        return it->second.toLobbyInfo();
    }
    return std::nullopt;
}

std::optional<Lobby> GameSession::getLobby(uint16_t lobby_id) const {
    auto it = _lobbies.find(lobby_id);
    if (it != _lobbies.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<PlayerInfo> GameSession::getLobbyPlayers(uint16_t lobby_id) const {
    std::vector<PlayerInfo> players;

    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return players;
    }

    const Lobby &lobby = it->second;
    players.reserve(lobby.players.size());

    for (const auto &player : lobby.players) {
        players.push_back(player.toPlayerInfo());
    }

    return players;
}

bool GameSession::canStartGame(uint16_t lobby_id) const {
    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return false;
    }

    const Lobby &lobby = it->second;

    // Need at least 2 players
    if (lobby.players.size() < MIN_PLAYERS) {
        return false;
    }

    // All players must be ready
    if (!lobby.allPlayersReady()) {
        return false;
    }

    // Lobby must not already be in game
    if (lobby.status == LobbyStatus::IN_GAME || lobby.status == LobbyStatus::CLOSING) {
        return false;
    }

    return true;
}

bool GameSession::startGame(uint16_t lobby_id) {
    if (!canStartGame(lobby_id)) {
        std::cerr << "Cannot start game for lobby " << lobby_id << std::endl;
        return false;
    }

    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return false;
    }

    it->second.status = LobbyStatus::IN_GAME;
    std::cout << "Game started for lobby " << lobby_id << std::endl;
    return true;
}

uint16_t GameSession::getNextLobbyId() {
    return _next_lobby_id++;
}

std::optional<LobbyPlayer*> GameSession::findPlayerInLobby(uint16_t lobby_id, 
                                                            ClientId client_id) {
    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return std::nullopt;
    }

    Lobby &lobby = it->second;
    auto player_it = std::find_if(lobby.players.begin(), lobby.players.end(),
                                   [client_id](const LobbyPlayer &p) {
                                       return p.client_id == client_id;
                                   });

    if (player_it != lobby.players.end()) {
        return &(*player_it);
    }

    return std::nullopt;
}

std::optional<const LobbyPlayer*> GameSession::findPlayerInLobby(uint16_t lobby_id,
                                                                  ClientId client_id) const {
    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return std::nullopt;
    }

    const Lobby &lobby = it->second;
    auto player_it = std::find_if(lobby.players.begin(), lobby.players.end(),
                                   [client_id](const LobbyPlayer &p) {
                                       return p.client_id == client_id;
                                   });

    if (player_it != lobby.players.end()) {
        return &(*player_it);
    }

    return std::nullopt;
}

bool GameSession::isValidLobbyName(const std::string &name) const {
    if (name.empty() || name.length() > MAX_LOBBY_NAME_LENGTH - 1) {
        return false;
    }

    for (char c : name) {
        if (!std::isprint(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}

bool GameSession::isValidUsername(const std::string &name) const {
    if (name.empty() || name.length() > MAX_USERNAME_LENGTH - 1) {
        return false;
    }

    for (char c : name) {
        if (!std::isprint(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}

bool GameSession::isValidMaxPlayers(uint8_t max_players) const {
    return max_players >= MIN_PLAYERS && max_players <= MAX_PLAYERS;
}
