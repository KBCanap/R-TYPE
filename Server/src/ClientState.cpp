/*
** EPITECH PROJECT, 2025
** ClientState.cpp
** File description:
** Implementation of client state management
*/

#include "ClientState.hpp"

ClientState::ClientState()
    : _state(ClientStateType::DISCONNECTED), _player_id(0) {}

void ClientState::connect(uint8_t player_id) {
    if (_state == ClientStateType::DISCONNECTED ||
        _state == ClientStateType::CONNECTING) {
        _state = ClientStateType::CONNECTED;
        _player_id = player_id;
    }
}

void ClientState::setReady() {
    if (_state == ClientStateType::CONNECTED) {
        _state = ClientStateType::READY;
    }
}

void ClientState::startGame() {
    if (_state == ClientStateType::READY) {
        _state = ClientStateType::IN_GAME;
    }
}

void ClientState::disconnect() {
    _state = ClientStateType::DISCONNECTED;
    _player_id = 0;
}
