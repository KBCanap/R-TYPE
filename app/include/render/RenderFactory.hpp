/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** RenderFactory
*/

#pragma once
#include "../../../ecs/include/render/IRenderAudio.hpp"
#include "../../../ecs/include/render/IRenderWindow.hpp"
#include <memory>

namespace render {

enum class RenderBackend {
    SFML,
    // Can add SDL, Raylib, etc. in the future
};

class RenderFactory {
  public:
    static std::unique_ptr<IRenderWindow>
    createWindow(RenderBackend backend, unsigned int width, unsigned int height,
                 const std::string &title);

    static std::unique_ptr<IRenderAudio> createAudio(RenderBackend backend);
};

} // namespace render
