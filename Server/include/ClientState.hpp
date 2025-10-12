/*
** EPITECH PROJECT, 2025
** ClientState.hpp
** File description:
** Client connection state management
*/

#ifndef CLIENTSTATE_HPP_
#define CLIENTSTATE_HPP_

#include "MessageType.hpp"
#include <chrono>

enum class ClientStateType {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    READY,
    IN_GAME
};

class ClientState {
  public:
    ClientState();
    ~ClientState() = default;

    void connect(uint8_t player_id);
    void setReady();
    void startGame();
    void disconnect();

    ClientStateType getState() const { return _state; }
    uint8_t getPlayerId() const { return _player_id; }
    bool isReady() const { return _state == ClientStateType::READY; }
    bool isConnected() const {
        return _state != ClientStateType::DISCONNECTED &&
               _state != ClientStateType::CONNECTING;
    }

  private:
    ClientStateType _state;
    uint8_t _player_id;
};

#endif /* !CLIENTSTATE_HPP_ */
