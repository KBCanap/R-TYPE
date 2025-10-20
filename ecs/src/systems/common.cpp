/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** Common helpers for systems
*/

#include "../../include/systems.hpp"
#include <string>

namespace systems {

static const char* ENEMY_TAGS[] = {
    "enemy",
    "enemy_zigzag",
    "enemy_spread",
    "boss"
};

static const size_t ENEMY_TAGS_COUNT = sizeof(ENEMY_TAGS) / sizeof(ENEMY_TAGS[0]);

bool is_enemy_tag(const std::string& tag) {
    for (size_t i = 0; i < ENEMY_TAGS_COUNT; ++i) {
        if (tag == ENEMY_TAGS[i]) {
            return true;
        }
    }
    return false;
}

static const render::IntRect EXPLOSION_FRAMES[] = {
    render::IntRect(70, 290, 36, 32),
    render::IntRect(106, 290, 36, 32),
    render::IntRect(142, 290, 36, 32),
    render::IntRect(178, 290, 35, 32)
};

static const size_t EXPLOSION_FRAMES_COUNT = sizeof(EXPLOSION_FRAMES) / sizeof(EXPLOSION_FRAMES[0]);

void create_explosion(registry &r, float x, float y) {
    auto explosion_entity = r.spawn_entity();
    r.add_component<component::position>(
        explosion_entity,
        component::position(x, y));
    r.add_component<component::drawable>(
        explosion_entity,
        component::drawable("assets/sprites/r-typesheet1.gif",
                            EXPLOSION_FRAMES[0], 2.0f,
                            "explosion"));

    auto &anim = r.add_component<component::animation>(
        explosion_entity, component::animation(0.1f, false, true));
    for (size_t i = 0; i < EXPLOSION_FRAMES_COUNT; ++i) {
        anim->frames.push_back(EXPLOSION_FRAMES[i]);
    }
    anim->playing = true;
    anim->current_frame = 0;
}

} // namespace systems
