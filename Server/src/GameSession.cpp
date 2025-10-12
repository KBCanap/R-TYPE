/*
** EPITECH PROJECT, 2025
** GameSession.cpp
** File description:
** Implementation of game session management
*/

#include "GameSession.hpp"

GameSession::GameSession() : _next_player_id(1) {}

std::pair<bool, uint8_t> GameSession::addClient(ClientId client_id) {
    if (isFull()) {
        return {false, 0};
    }

    if (_clients.find(client_id) != _clients.end()) {
        return {false, 0};
    }

    uint8_t player_id = getNextPlayerId();
    ClientState client_state;
    client_state.connect(player_id);

    _clients[client_id] = client_state;

    return {true, player_id};
}

void GameSession::removeClient(ClientId client_id) {
    auto it = _clients.find(client_id);
    if (it != _clients.end()) {
        _clients.erase(it);
    }
}

bool GameSession::setClientReady(ClientId client_id) {
    auto it = _clients.find(client_id);

    if (it == _clients.end()) {
        return false;
    }

    it->second.setReady();
    return true;
}

bool GameSession::canStartGame() const {
    if (_clients.empty()) {
        return false;
    }

    for (const auto &[id, client] : _clients) {
        if (!client.isReady()) {
            return false;
        }
    }

    return true;
}

std::vector<ClientId> GameSession::getReadyClients() const {
    std::vector<ClientId> ready_clients;

    for (const auto &[id, client] : _clients) {
        if (client.isReady()) {
            ready_clients.push_back(id);
        }
    }

    return ready_clients;
}

std::vector<ClientId> GameSession::getConnectedClients() const {
    std::vector<ClientId> connected_clients;

    for (const auto &[id, client] : _clients) {
        if (client.isConnected()) {
            connected_clients.push_back(id);
        }
    }

    return connected_clients;
}

uint8_t GameSession::getNextPlayerId() { return _next_player_id++; }
