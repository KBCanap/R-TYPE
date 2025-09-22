/*
** EPITECH PROJECT, 2025
** GameManager.cpp
** File description:
** Game state management for R-Type server
*/

#include "GameManager.hpp"
#include <iostream>
#include <algorithm>

GameManager::GameManager() : _gameState(GameState::WAITING_FOR_PLAYERS)
{
}

bool GameManager::addPlayer(const std::string& playerId, const asio::ip::udp::endpoint& endpoint)
{
    if (!canAcceptNewPlayer()) {
        std::cout << "Serveur complet, impossible d'accepter le joueur " << playerId << std::endl;
        return false;
    }

    if (getPlayer(playerId) != nullptr) {
        std::cout << "Joueur " << playerId << " déjà connecté" << std::endl;
        return false;
    }
    
    auto player = std::make_shared<Player>(playerId, endpoint);
    _players.push_back(player);
    
    std::cout << "Nouveau joueur connecté: " << playerId 
              << " (" << _players.size() << "/" << MAX_PLAYERS << ")" << std::endl;
    
    return true;
}

bool GameManager::removePlayer(const std::string& playerId)
{
    auto it = std::find_if(_players.begin(), _players.end(),
        [&playerId](const std::shared_ptr<Player>& player) {
            return player->getId() == playerId;
        });
    
    if (it != _players.end()) {
        std::cout << "Joueur déconnecté: " << playerId << std::endl;
        _players.erase(it);
        

        if (_gameState == GameState::IN_GAME && _players.size() < 1) {
            stopGame();
        }
        
        return true;
    }
    
    return false;
}

bool GameManager::setPlayerReady(const std::string& playerId, bool ready)
{
    auto player = getPlayer(playerId);
    if (player == nullptr) {
        return false;
    }
    
    player->setState(ready ? PlayerState::READY : PlayerState::CONNECTED);
    std::cout << "Joueur " << playerId << (ready ? " prêt" : " pas prêt") << std::endl;
    
    checkGameStart();
    return true;
}

bool GameManager::canAcceptNewPlayer() const
{
    return _players.size() < MAX_PLAYERS && _gameState == GameState::WAITING_FOR_PLAYERS;
}

bool GameManager::allPlayersReady() const
{
    if (_players.empty()) {
        return false;
    }
    
    return std::all_of(_players.begin(), _players.end(),
        [](const std::shared_ptr<Player>& player) {
            return player->isReady();
        });
}

std::shared_ptr<Player> GameManager::getPlayer(const std::string& playerId)
{
    auto it = std::find_if(_players.begin(), _players.end(),
        [&playerId](const std::shared_ptr<Player>& player) {
            return player->getId() == playerId;
        });
    
    return (it != _players.end()) ? *it : nullptr;
}

void GameManager::startGame()
{
    if (_gameState == GameState::IN_GAME) {
        return;
    }
    
    _gameState = GameState::IN_GAME;

    for (auto& player : _players) {
        player->setState(PlayerState::IN_GAME);
    }
    
    std::cout << "=== JEU COMMENCE AVEC " << _players.size() << " JOUEURS ===" << std::endl;
    
    if (_gameStartCallback) {
        _gameStartCallback();
    }
}

void GameManager::stopGame()
{
    if (_gameState == GameState::WAITING_FOR_PLAYERS) {
        return;
    }
    
    _gameState = GameState::WAITING_FOR_PLAYERS;

    for (auto& player : _players) {
        player->setState(PlayerState::CONNECTED);
    }
    
    std::cout << "=== JEU ARRÊTÉ ===" << std::endl;
}

void GameManager::checkGameStart()
{
    if (_gameState == GameState::WAITING_FOR_PLAYERS && 
        _players.size() >= 1 &&
        allPlayersReady()) {
        startGame();
    }
}