#include "render/RenderFactory.hpp"
#include "render/sfml/SFMLRenderWindow.hpp"
#include "render/sfml/SFMLRenderAudio.hpp"
#include <stdexcept>

namespace render {

std::unique_ptr<IRenderWindow> RenderFactory::createWindow(
    RenderBackend backend,
    unsigned int width,
    unsigned int height,
    const std::string& title
) {
    switch (backend) {
        case RenderBackend::SFML:
            return std::make_unique<sfml::SFMLRenderWindow>(width, height, title);
        default:
            throw std::runtime_error("Unsupported render backend");
    }
}

std::unique_ptr<IRenderAudio> RenderFactory::createAudio(RenderBackend backend) {
    switch (backend) {
        case RenderBackend::SFML:
            return std::make_unique<sfml::SFMLRenderAudio>();
        default:
            throw std::runtime_error("Unsupported audio backend");
    }
}

} // namespace render
