/*
** EPITECH PROJECT, 2025
** UdpServer.cpp
** File description:
** created by dylan adg
*/

#include "UdpServer.hpp"
#include <iostream>
#include <sstream>

UdpServer::UdpServer(asio::io_context &context, int port)
    : _io_context(context),
      _socket(context, asio::ip::udp::endpoint(asio::ip::udp::v4(), static_cast<unsigned short>(port))),
      _game_timer(context)
{
    std::cout << "UDP Server initialized on port " << port << std::endl;

    _game_manager.setGameStartCallback([this]() {
        start_game_loop();
    });
}

UdpServer::~UdpServer()
{
    std::cout << "UDP Server destroyed" << std::endl;
}

void UdpServer::start_receive()
{
    _socket.async_receive_from(
        asio::buffer(_recv_buffer), _remote_endpoint,
        [this](std::error_code ec, std::size_t bytes_transferred) {
            handle_receive(ec, bytes_transferred);
        });
}

void UdpServer::handle_receive(const std::error_code& error, std::size_t bytes_transferred)
{
    if (!error && bytes_transferred > 0) {
        std::string received_message(_recv_buffer.data(), bytes_transferred);
        std::string client_id = endpoint_to_string(_remote_endpoint);
        
        std::cout << "Message reçu de " << client_id << ": " << received_message << std::endl;
        std::string command = received_message;
        
        if (command == "CONNECT\n") {
            handle_connect_request(_remote_endpoint);
        } else if (command == "READY\n") {
            handle_ready_request(client_id);
        } else {
            send_to_client("ERROR:Unknown command\n", _remote_endpoint);
        }
    } else if (error) {
        std::cerr << "Erreur de réception: " << error.message() << std::endl;
    }
    
    start_receive();
}

void UdpServer::handle_connect_request(const asio::ip::udp::endpoint& client_endpoint)
{
    std::string client_id = endpoint_to_string(client_endpoint);
    
    if (_game_manager.canAcceptNewPlayer()) {
        if (_game_manager.addPlayer(client_id, client_endpoint)) {
            send_to_client("CONNECTED:Welcome to R-Type server!", client_endpoint);
            std::string notification = "PLAYER_JOINED:" + std::to_string(_game_manager.getPlayerCount()) + 
                                     "/" + std::to_string(GameManager::MAX_PLAYERS);
            broadcast_to_all_players(notification);
        } else {
            send_to_client("ERROR:Already connected\n", client_endpoint);
        }
    } else {
        send_to_client("ERROR:Server full\n", client_endpoint);
    }
}

void UdpServer::handle_ready_request(const std::string& playerId)
{
    if (_game_manager.setPlayerReady(playerId, true)) {
        auto player = _game_manager.getPlayer(playerId);
        if (player) {
            send_to_client("READY:You are ready!", player->getEndpoint());
            std::string notification = "PLAYER_READY:" + playerId;
            broadcast_to_all_players(notification);
        }
    } else {
        auto player = _game_manager.getPlayer(playerId);
        if (player) {
            send_to_client("ERROR:Cannot set ready", player->getEndpoint());
        }
    }
}

void UdpServer::handle_game_message(const std::string& playerId, const std::string& message)
{
    auto player = _game_manager.getPlayer(playerId);
    if (!player) {
        return;
    }
    
    if (_game_manager.getGameState() == GameState::IN_GAME) {
        std::cout << "Message de jeu de " << playerId << ": " << message << std::endl;

        std::string response = "GAME_RESPONSE:" + playerId + ":" + message;
        send_to_client(response, player->getEndpoint());
    } else {
        send_to_client("ERROR:Game not started", player->getEndpoint());
    }
}

void UdpServer::start_game_loop()
{
    std::cout << "Démarrage de la boucle de jeu..." << std::endl;

    broadcast_to_all_players("GAME_START:Game is starting!");

    _game_timer.expires_after(std::chrono::milliseconds(16));
    _game_timer.async_wait([this](std::error_code ec) {
        if (!ec) {
            handle_game_tick();
        }
    });
}

void UdpServer::handle_game_tick()
{
    if (_game_manager.getGameState() != GameState::IN_GAME) {
        return;
    }

    _game_timer.expires_after(std::chrono::milliseconds(16));
    _game_timer.async_wait([this](std::error_code ec) {
        if (!ec) {
            handle_game_tick();
        }
    });
}

void UdpServer::send_to_client(const std::string& message, const asio::ip::udp::endpoint& client_endpoint)
{
    auto send_buffer = std::make_shared<std::string>(message);
    
    _socket.async_send_to(
        asio::buffer(*send_buffer), client_endpoint,
        [this, send_buffer](std::error_code ec, std::size_t bytes_transferred) {
            handle_send(ec, bytes_transferred);
        });
}

void UdpServer::broadcast_to_all_players(const std::string& message)
{
    for (const auto& player : _game_manager.getPlayers()) {
        send_to_client(message, player->getEndpoint());
    }
}

void UdpServer::handle_send(const std::error_code& error, std::size_t bytes_transferred)
{
    if (error) {
        std::cerr << "Erreur d'envoi: " << error.message() << std::endl;
    } else {
        std::cout << "Message envoyé (" << bytes_transferred << " bytes)" << std::endl;
    }
}

std::string UdpServer::endpoint_to_string(const asio::ip::udp::endpoint& endpoint)
{
    return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
}

std::vector<std::string> UdpServer::split_message(const std::string& message, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(message);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}
