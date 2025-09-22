/*
** EPITECH PROJECT, 2025
** Player.hpp
** File description:
** Player management for R-Type server
*/

#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <asio.hpp>
#include <string>

enum class PlayerState {
    CONNECTED,
    READY,
    IN_GAME
};

class Player {
    public:
        Player(const std::string& id, const asio::ip::udp::endpoint& endpoint);
        ~Player() = default;
    
        const std::string& getId() const { return _id; }
        const asio::ip::udp::endpoint& getEndpoint() const { return _endpoint; }
        PlayerState getState() const { return _state; }
        
        void setState(PlayerState state) { _state = state; }
        bool isReady() const { return _state == PlayerState::READY; }
    
    private:
        std::string _id;
        asio::ip::udp::endpoint _endpoint;
        PlayerState _state;
};

#endif /* !PLAYER_HPP_ */
