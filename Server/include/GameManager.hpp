/*
** EPITECH PROJECT, 2025
** GameManager.hpp
** File description:
** Game state management for R-Type server
*/

#ifndef GAMEMANAGER_HPP_
#define GAMEMANAGER_HPP_

#include "Player.hpp"
#include <asio.hpp>
#include <vector>
#include <memory>
#include <functional>

enum class GameState {
    WAITING_FOR_PLAYERS,
    IN_GAME
};

class GameManager {
    public:
        static const size_t MAX_PLAYERS = 4;
        
        GameManager();
        ~GameManager() = default;
    
        bool addPlayer(const std::string& playerId, const asio::ip::udp::endpoint& endpoint);
        bool removePlayer(const std::string& playerId);
        bool setPlayerReady(const std::string& playerId, bool ready);
        
        bool canAcceptNewPlayer() const;
        bool allPlayersReady() const;
        size_t getPlayerCount() const { return _players.size(); }
        GameState getGameState() const { return _gameState; }
        
        const std::vector<std::shared_ptr<Player>>& getPlayers() const { return _players; }
        std::shared_ptr<Player> getPlayer(const std::string& playerId);
        
        void startGame();
        void stopGame();
    
        void setGameStartCallback(std::function<void()> callback) { _gameStartCallback = callback; }
    
    private:
        std::vector<std::shared_ptr<Player>> _players;
        GameState _gameState;
        std::function<void()> _gameStartCallback;
        
        void checkGameStart();
};

#endif /* !GAMEMANAGER_HPP_ */
