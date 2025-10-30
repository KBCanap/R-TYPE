/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** main
*/

#include "../include/audio_manager.hpp"
#include "../include/connection_menu.hpp"
#include "../include/game.hpp"
#include "../include/key_bindings.hpp"
#include "../include/lobby_browser_menu.hpp"
#include "../include/lobby_menu.hpp"
#include "../include/menu.hpp"
#include "../include/network/NetworkManager.hpp"
#include "components.hpp"
#include "registery.hpp"
#include "render/RenderFactory.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

struct ClientConfig {
    std::string server_ip;
    uint16_t server_port;
    bool valid;
};

static void show_help() {
    std::cout << "USAGE:" << std::endl;
    std::cout << "./r-type_client [-i <server_ip>] [-p <tcp_port>] [-h]"
              << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "  -i <ip>      Server IP address (default: 127.0.0.1)"
              << std::endl;
    std::cout << "  -p <port>    Server TCP port (default: 8080)" << std::endl;
    std::cout << "  -h           Show this help message" << std::endl;
    std::cout << "\nExample: ./r-type_client -i 192.168.1.100 -p 8080"
              << std::endl;
}

static bool is_valid_port(const char *str) {
    if (str == nullptr || *str == '\0')
        return false;

    for (int i = 0; str[i]; ++i) {
        if (!std::isdigit(str[i]))
            return false;
    }

    int port = std::atoi(str);
    return port >= 1024 && port <= 65535;
}

static bool is_valid_ip(const std::string &ip) {
    if (ip.empty() || ip.length() > 253)
        return false;

    // Allow localhost variations
    if (ip == "localhost" || ip == "127.0.0.1")
        return true;

    // Basic IP format validation
    int dots = 0;
    bool has_digit = false;

    for (size_t i = 0; i < ip.length(); ++i) {
        char c = ip[i];
        if (c == '.') {
            if (!has_digit || i == 0 || i == ip.length() - 1)
                return false;
            dots++;
            has_digit = false;
        } else if (std::isdigit(c)) {
            has_digit = true;
        } else {
            return false;
        }
    }

    return dots == 3 && has_digit;
}

static ClientConfig parse_arguments(int argc, char **argv) {
    ClientConfig config = {"127.0.0.1", 8080, true}; // Valeurs par défaut

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-h") == 0) {
            show_help();
            config.valid = false;
            return config;
        } else if (std::strcmp(argv[i], "-i") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: Missing IP address after -i flag"
                          << std::endl;
                show_help();
                config.valid = false;
                return config;
            }

            std::string ip = argv[i + 1];
            if (!is_valid_ip(ip)) {
                std::cerr << "Error: Invalid IP address '" << ip << "'"
                          << std::endl;
                show_help();
                config.valid = false;
                return config;
            }

            config.server_ip = ip;
            i++;
        } else if (std::strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc || !is_valid_port(argv[i + 1])) {
                std::cerr << "Error: Invalid or missing TCP port after -p flag"
                          << std::endl;
                show_help();
                config.valid = false;
                return config;
            }

            config.server_port = static_cast<uint16_t>(std::atoi(argv[i + 1]));
            i++;
        } else {
            std::cerr << "Error: Unknown flag '" << argv[i] << "'" << std::endl;
            show_help();
            config.valid = false;
            return config;
        }
    }

    return config;
}

int main(int argc, char **argv) {
    // Parse command line arguments
    ClientConfig config = parse_arguments(argc, argv);

    if (!config.valid) {
        return (argc == 2 && std::strcmp("-h", argv[1]) == 0) ? 0 : 84;
    }

    std::cout << "[Client] Configuration: " << config.server_ip << ":"
              << config.server_port << std::endl;

    // Create render and audio systems using factory
    auto window = render::RenderFactory::createWindow(
        render::RenderBackend::SFML, 800, 600, "R-TYPE");

    auto audioSystem =
        render::RenderFactory::createAudio(render::RenderBackend::SFML);

    registry reg;
    AudioManager audioManager(*audioSystem);
    KeyBindings keyBindings;

    reg.register_component<component::position>();
    reg.register_component<component::velocity>();
    reg.register_component<component::drawable>();
    reg.register_component<component::controllable>();

    bool running = true;
    while (running && window->isOpen()) {
        // Main menu
        Menu menu(reg, *window, audioManager, keyBindings);
        MenuResult menuResult = menu.run();

        if (menuResult == MenuResult::Quit) {
            running = false;
            break;
        }

        if (menuResult == MenuResult::Play) {
            ConnectionInfo connInfo;
            connInfo.serverHost = config.server_ip;
            connInfo.serverPort = config.server_port;
            connInfo.username = ""; // Will be set by ConnectionMenu

            ConnectionMenu connectionMenu(*window, audioManager);
            ConnectionMenuResult connResult = connectionMenu.run(connInfo);

            if (connResult == ConnectionMenuResult::Solo) {
                std::cout << "[Main] Starting SOLO game..." << std::endl;
                Game game(reg, *window, audioManager,
                          keyBindings); // nullptr = solo
                game.run();
                std::cout << "[Main] Solo game ended, returning to main menu..."
                          << std::endl;
            } else if (connResult == ConnectionMenuResult::Multiplayer) {
                // Use username from ConnectionMenu (or default if empty)
                std::string username = connInfo.username.empty()
                    ? "Player" + std::to_string(rand() % 10000)
                    : connInfo.username;

                std::cout << "[Main] Connecting to: "
                          << connInfo.serverHost << ":" << connInfo.serverPort
                          << " as " << username << std::endl;

                auto networkManager =
                    std::make_unique<network::NetworkManager>();

                auto connectionResult = networkManager->connectTCP(
                    connInfo.serverHost, connInfo.serverPort, username);

                if (connectionResult.success) {
                    std::cout << "[Main] TCP connection successful!"
                              << std::endl;

                    // Show lobby browser menu
                    LobbyBrowserMenu lobbyBrowserMenu(*window, audioManager,
                                                      *networkManager);
                    LobbyBrowserResult browserResult = lobbyBrowserMenu.run();

                    if (browserResult == LobbyBrowserResult::JoinedLobby) {
                        std::cout << "[Main] Joined lobby, entering lobby menu..."
                                  << std::endl;

                        // Show lobby menu (waiting room)
                        LobbyMenu lobbyMenu(*window, audioManager,
                                            *networkManager);
                        LobbyResult lobbyResult = lobbyMenu.run();

                        uint8_t playerId = networkManager->getPlayerID();
                        uint16_t udpPort = networkManager->getUDPPort();

                        std::cout << "[Main] Player ID: "
                                  << static_cast<int>(playerId) << std::endl;
                        std::cout << "[Main] UDP Port: " << udpPort << std::endl;

                        if (lobbyResult == LobbyResult::GameStarting) {
                            std::cout << "[Main] 🎮 Starting MULTIPLAYER game..."
                                      << std::endl;

                            // Launch multiplayer game with network manager
                            Game game(reg, *window, audioManager, keyBindings,
                                      networkManager.get());
                            game.run();

                            std::cout
                                << "[Main] Game ended, returning to main menu..."
                                << std::endl;
                        } else {
                            std::cout << "[Main] Lobby exited" << std::endl;
                        }
                    } else {
                        std::cout << "[Main] Lobby browser exited" << std::endl;
                    }

                    // Disconnect gracefully
                    networkManager->disconnect();
                } else {
                    std::cerr << "[Main] ❌ Connection failed: "
                              << connectionResult.error_message << std::endl;
                    std::cout << "[Main] Returning to main menu..."
                              << std::endl;
                }
            }
            // Si "Back" est pressé, retourner au menu principal
        }
    }

    window->close();
    return 0;
}
