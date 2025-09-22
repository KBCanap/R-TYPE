/*
** EPITECH PROJECT, 2025
** Player.cpp
** File description:
** Player management for R-Type server
*/

#include "Player.hpp"

Player::Player(const std::string& id, const asio::ip::udp::endpoint& endpoint)
    : _id(id), _endpoint(endpoint), _state(PlayerState::CONNECTED)
{
}
