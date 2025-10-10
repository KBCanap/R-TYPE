#include "../include/menu.hpp"
#include "../include/connection_menu.hpp"
#include "../include/lobby_menu.hpp"
#include "../include/game.hpp"
#include "../include/network/NetworkManager.hpp"
#include "registery.hpp"
#include "components.hpp"
#include "../include/audio_manager.hpp"
#include "../include/key_bindings.hpp"
#include "render/RenderFactory.hpp"
#include <memory>
#include <iostream>

int main() {
    // Create render and audio systems using factory
    auto window = render::RenderFactory::createWindow(
        render::RenderBackend::SFML,
        800, 600,
        "R-TYPE"
    );

    auto audioSystem = render::RenderFactory::createAudio(
        render::RenderBackend::SFML
    );

    registry reg;
    AudioManager audioManager(*audioSystem);
    KeyBindings keyBindings;

    // Enregistre les composants
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
            // Show connection menu to choose between Solo and Multiplayer
            ConnectionInfo connInfo;
            ConnectionMenu connectionMenu(*window, audioManager);
            ConnectionMenuResult connResult = connectionMenu.run(connInfo);

            if (connResult == ConnectionMenuResult::Solo) {
                // Start solo game
                std::cout << "[Main] Starting SOLO game..." << std::endl;
                Game game(reg, *window, audioManager, keyBindings);
                game.run();
                std::cout << "[Main] Solo game ended, returning to main menu..." << std::endl;
            }
            else if (connResult == ConnectionMenuResult::Multiplayer) {
                // Create network manager and connect
                auto networkManager = std::make_unique<network::NetworkManager>();

                std::cout << "[Main] Connecting to " << connInfo.serverHost
                          << ":" << connInfo.serverPort << "..." << std::endl;

                auto connResult = networkManager->connectTCP(connInfo.serverHost, connInfo.serverPort);

                if (connResult.success) {
                    std::cout << "[Main] Connected successfully!" << std::endl;

                    // Show lobby menu
                    LobbyMenu lobbyMenu(*window, audioManager, *networkManager);
                    LobbyResult lobbyResult = lobbyMenu.run();

                    uint8_t playerId = networkManager->getPlayerID();
                    std::cout << "[Main] Final Player ID: " << static_cast<int>(playerId) << std::endl;

                    if (lobbyResult == LobbyResult::StartGame) {
                        std::cout << "[Main] Starting MULTIPLAYER game..." << std::endl;
                        std::cout << "[Main] Multiplayer game implementation coming soon!" << std::endl;
                        std::cout << "[Main] Returning to main menu..." << std::endl;
                        // TODO: Launch multiplayer game with network manager
                        // The Game class needs to be modified to accept NetworkManager
                        // Game game(reg, *window, audioManager, networkManager);
                        // game.run();

                        // Disconnect gracefully
                        networkManager->disconnect();
                    } else {
                        std::cout << "[Main] Disconnected from lobby" << std::endl;
                        networkManager->disconnect();
                    }
                } else {
                    std::cerr << "[Main] Connection failed: " << connResult.error_message << std::endl;
                }
            }
            // If Back was pressed, loop back to main menu
        }
    }

    window->close();
    return 0;
}
