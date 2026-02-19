/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** beam_system - continuous laser beam powerup
*/

#include "../../include/systems.hpp"
#include "../../include/GameConstants.hpp"
#include <algorithm>

namespace systems {

void beam_system(registry &r, render::IRenderWindow &window, float dt) {
    auto &beams = r.get_components<component::beam>();
    auto &positions = r.get_components<component::position>();
    auto &drawables = r.get_components<component::drawable>();
    auto &healths = r.get_components<component::health>();
    auto &weapons = r.get_components<component::weapon>();
    auto &hitboxes = r.get_components<component::hitbox>();

    render::Vector2u win_size = window.getSize();
    float window_width = static_cast<float>(win_size.x);

    std::vector<size_t> expired_beams;

    for (size_t i = 0; i < beams.size(); ++i) {
        if (!beams[i] || !positions[i])
            continue;

        component::beam &b = *beams[i];
        b.elapsed += dt;

        if (b.elapsed >= b.duration) {
            expired_beams.push_back(i);

            size_t owner = b.owner_idx;
            if (owner < weapons.size() && weapons[owner]) {
                weapons[owner]->fire_function = nullptr;
                weapons[owner]->projectile_damage = weapons[owner]->base_projectile_damage;
                weapons[owner]->damage_boost_timer = 0.0f;
            }
            continue;
        }

        size_t owner = b.owner_idx;
        if (owner < positions.size() && positions[owner]) {
            positions[i]->x = positions[owner]->x;
            positions[i]->y = positions[owner]->y;
        }

        float player_x = positions[i]->x;
        float player_y = positions[i]->y;
        float beam_x = player_x + 60.0f;
        float beam_y = player_y + 17.0f - b.height / 2.0f;
        float beam_w = window_width - beam_x;
        float beam_h = b.height;

        if (beam_w <= 0.0f)
            continue;

        size_t max_ents = std::min({drawables.size(), positions.size(), healths.size()});
        for (size_t e = 0; e < max_ents; ++e) {
            if (!drawables[e] || !positions[e] || !healths[e])
                continue;
            if (!is_enemy_tag(drawables[e]->tag))
                continue;

            float ex = positions[e]->x;
            float ey = positions[e]->y;
            float ew = drawables[e]->size;
            float eh = ew;

            if (e < hitboxes.size() && hitboxes[e]) {
                ex += hitboxes[e]->offset_x;
                ey += hitboxes[e]->offset_y;
                ew = hitboxes[e]->width;
                eh = hitboxes[e]->height;
            } else if (drawables[e]->use_sprite && drawables[e]->sprite_rect.width > 0) {
                ew = drawables[e]->sprite_rect.width * drawables[e]->scale;
                eh = drawables[e]->sprite_rect.height * drawables[e]->scale;
            }

            bool overlap = (beam_x < ex + ew) && (beam_x + beam_w > ex) &&
                           (beam_y < ey + eh) && (beam_y + beam_h > ey);

            if (overlap) {
                healths[e]->pending_damage += static_cast<int>(b.damage_per_sec * dt);
            }
        }
    }

    for (size_t idx : expired_beams) {
        r.kill_entity(entity(idx));
    }
}

} // namespace systems
