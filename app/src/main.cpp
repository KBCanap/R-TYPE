#include "../include/menu.hpp"
#include "../include/game.hpp"
#include "registery.hpp"
#include "components.hpp"
#include "../include/audio_manager.hpp"
#include "render/RenderFactory.hpp"
#include "render/sfml/SFMLRenderWindow.hpp"
#include <memory>

int main() {
    // Create render and audio systems using factory
    auto window = render::RenderFactory::createWindow(
        render::RenderBackend::SFML,
        800, 600,
        "BS_R-TYPE"
    );

    auto audioSystem = render::RenderFactory::createAudio(
        render::RenderBackend::SFML
    );

    registry reg;
    AudioManager audioManager(*audioSystem);

    // Enregistre les composants
    reg.register_component<component::position>();
    reg.register_component<component::velocity>();
    reg.register_component<component::drawable>();
    reg.register_component<component::controllable>();

    // Get native SFML window for compatibility during transition
    auto& sfmlWindow = dynamic_cast<render::sfml::SFMLRenderWindow&>(*window);

    // Lance le menu
    Menu menu(reg, sfmlWindow.getNativeWindow(), audioManager);
    MenuResult result = menu.run();

    if (result == MenuResult::Play) {
        // Lance le jeu
        Game game(reg, sfmlWindow.getNativeWindow(), audioManager);
        game.run();
    }
    else if (result == MenuResult::Quit) {
        window->close();
    }

    return 0;
}
