/*
** EPITECH PROJECT, 2025
** GameSession.hpp
** File description:
** Game session management for up to 4 players
*/

#ifndef GAMESESSION_HPP_
#define GAMESESSION_HPP_

#include "ClientState.hpp"
#include "MessageType.hpp"
#include <unordered_map>
#include <vector>

using ClientId = size_t;

class GameSession {
  public:
    GameSession();
    ~GameSession() = default;

    std::pair<bool, uint8_t> addClient(ClientId client_id);
    void removeClient(ClientId client_id);
    bool setClientReady(ClientId client_id);

    bool canStartGame() const;
    std::vector<ClientId> getReadyClients() const;
    std::vector<ClientId> getConnectedClients() const;
    size_t getClientCount() const { return _clients.size(); }
    bool isFull() const { return _clients.size() >= MAX_PLAYERS; }

  private:
    std::unordered_map<ClientId, ClientState> _clients;
    uint8_t _next_player_id;

    uint8_t getNextPlayerId();
};

#endif /* !GAMESESSION_HPP_ */
