/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** mario_systems - Systems specific to Mario platformer game
*/

#include "systems.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace systems {

// Helper: Check if an enemy is standing on a specific platform
static bool is_entity_on_platform(
    const component::position &entity_pos,
    const component::hitbox &entity_hitbox,
    const component::position &plat_pos,
    const component::hitbox &plat_hitbox,
    const component::gravity *entity_grav) {

    if (!entity_grav || !entity_grav->on_ground)
        return false;

    float entity_left = entity_pos.x + entity_hitbox.offset_x;
    float entity_right = entity_left + entity_hitbox.width;
    float entity_bottom = entity_pos.y + entity_hitbox.offset_y + entity_hitbox.height;

    float plat_left = plat_pos.x + plat_hitbox.offset_x;
    float plat_right = plat_left + plat_hitbox.width;
    float plat_top = plat_pos.y + plat_hitbox.offset_y;

    // Check if entity bottom is near platform top (within 5 pixels)
    bool on_top = std::abs(entity_bottom - plat_top) < 5.0f;

    // Check horizontal overlap
    bool x_overlap = entity_right > plat_left && entity_left < plat_right;

    return on_top && x_overlap;
}

// Helper: Stun ALL enemies on the screen (for POW block)
static void stun_all_enemies(
    registry &r,
    sparse_array<component::position> &positions,
    sparse_array<component::velocity> &velocities) {

    auto &drawables = r.get_components<component::drawable>();
    auto &stunneds = r.get_components<component::enemy_stunned>();
    auto &gravities = r.get_components<component::gravity>();

    for (size_t i = 0; i < positions.size(); ++i) {
        auto &pos = positions[i];
        auto &vel = velocities[i];
        auto &drawable = drawables[i];
        auto &stunned = stunneds[i];
        auto &grav = gravities[i];

        if (!pos || !vel || !drawable)
            continue;

        // Only affect enemies
        if (drawable->tag != "enemy")
            continue;

        // Already stunned - skip
        if (stunned && stunned->stunned)
            continue;

        // Stun the enemy!
        if (stunned) {
            stunned->stunned = true;
            stunned->knockback_velocity = 0.0f;
            stunned->stun_timer = 0.0f;
        }

        // Stop enemy movement
        vel->vx = 0.0f;

        // Bounce enemies on the ground
        if (grav && grav->on_ground) {
            vel->vy = -300.0f;
        }
    }

    std::cout << "[Mario] POW! All enemies stunned!" << std::endl;
}

// Helper: Stun enemies on a platform within range of player hit
static void stun_enemies_on_platform(
    registry &r,
    size_t platform_idx,
    float player_hit_x,
    sparse_array<component::position> &positions,
    sparse_array<component::hitbox> &hitboxes,
    sparse_array<component::gravity> &gravities,
    sparse_array<component::velocity> &velocities) {

    auto &drawables = r.get_components<component::drawable>();
    auto &stunneds = r.get_components<component::enemy_stunned>();

    auto &plat_pos = positions[platform_idx];
    auto &plat_hitbox = hitboxes[platform_idx];

    if (!plat_pos || !plat_hitbox)
        return;

    // Stun radius: only enemies within 80 pixels of where player hit
    const float STUN_RADIUS = 80.0f;

    for (size_t i = 0; i < positions.size(); ++i) {
        auto &pos = positions[i];
        auto &hitbox = hitboxes[i];
        auto &drawable = drawables[i];
        auto &grav = gravities[i];
        auto &vel = velocities[i];
        auto &stunned = stunneds[i];

        if (!pos || !hitbox || !drawable || !grav || !vel)
            continue;

        // Only stun enemies
        if (drawable->tag != "enemy")
            continue;

        // Check if already stunned
        if (stunned && stunned->stunned)
            continue;

        // Check if enemy is on this platform
        if (!is_entity_on_platform(*pos, *hitbox, *plat_pos, *plat_hitbox, &(*grav)))
            continue;

        // Check if enemy is within stun radius of where player hit
        float enemy_center_x = pos->x + hitbox->offset_x + hitbox->width / 2.0f;
        float distance = std::abs(enemy_center_x - player_hit_x);

        if (distance > STUN_RADIUS)
            continue;

        // Stun the enemy!
        if (stunned) {
            stunned->stunned = true;
            stunned->knockback_velocity = 0.0f;
            stunned->stun_timer = 0.0f;
        }

        // Stop enemy movement
        vel->vx = 0.0f;

        // Give a small bounce effect
        vel->vy = -200.0f;

        std::cout << "[Mario] Enemy stunned by platform hit from below!" << std::endl;
    }
}

// Gravity system - applies gravity acceleration to entities
void gravity_system(registry &r, sparse_array<component::gravity> &gravities,
                    sparse_array<component::velocity> &velocities, float dt) {
    auto &deads = r.get_components<component::dead>();

    for (size_t i = 0; i < gravities.size() && i < velocities.size(); ++i) {
        auto &gravity = gravities[i];
        auto &velocity = velocities[i];

        if (!gravity || !velocity)
            continue;

        // Check if entity is dead
        auto dead = (i < deads.size()) ? deads[i] : std::nullopt;

        // Apply gravity acceleration
        velocity->vy += gravity->acceleration * dt;

        // Cap falling speed (unless dead - let them fall through)
        if (!dead || !dead->ignore_platforms) {
            if (velocity->vy > gravity->max_velocity) {
                velocity->vy = gravity->max_velocity;
            }
        }
    }
}

// Platform collision system - handles collision detection and resolution
void platform_collision_system(
    registry &r, sparse_array<component::position> &positions,
    sparse_array<component::velocity> &velocities,
    sparse_array<component::gravity> &gravities,
    sparse_array<component::platform_tag> &platforms,
    sparse_array<component::hitbox> &hitboxes) {

    auto &deads = r.get_components<component::dead>();
    auto &drawables = r.get_components<component::drawable>();
    auto &pow_blocks = r.get_components<component::pow_block>();

    // Track platforms hit from below by player (for stunning enemies)
    // Pair: (platform_idx, player_hit_x)
    std::vector<std::pair<size_t, float>> platforms_hit_from_below;
    bool pow_block_hit = false;
    size_t pow_block_idx = 0;

    // For each entity with gravity (player/enemies)
    for (size_t i = 0; i < positions.size(); ++i) {
        auto &pos = positions[i];
        auto &vel = velocities[i];
        auto &grav = gravities[i];
        auto &hitbox = hitboxes[i];

        if (!pos || !vel || !grav || !hitbox)
            continue;

        // Check if entity is dead and should ignore platforms
        auto dead = (i < deads.size()) ? deads[i] : std::nullopt;
        if (dead && dead->ignore_platforms) {
            grav->on_ground = false;
            continue;
        }

        // Kicked enemies (stunned with knockback) should ignore platforms
        auto &stunneds = r.get_components<component::enemy_stunned>();
        if (i < stunneds.size() && stunneds[i] && stunneds[i]->stunned &&
            std::abs(stunneds[i]->knockback_velocity) > 1.0f) {
            grav->on_ground = false;
            continue;
        }

        // Check if this is the player
        bool is_player = (i < drawables.size()) && drawables[i] &&
                         (drawables[i]->tag == "player");

        // Reset on_ground status
        grav->on_ground = false;

        // Check collision with each platform
        for (size_t j = 0; j < positions.size(); ++j) {
            auto &plat_pos = positions[j];
            auto &plat_tag = platforms[j];
            auto &plat_hitbox = hitboxes[j];

            if (!plat_pos || !plat_tag || !plat_hitbox)
                continue;

            // Calculate entity bounds
            float entity_left = pos->x + hitbox->offset_x;
            float entity_right = entity_left + hitbox->width;
            float entity_top = pos->y + hitbox->offset_y;
            float entity_bottom = entity_top + hitbox->height;

            // Calculate platform bounds
            float plat_left = plat_pos->x + plat_hitbox->offset_x;
            float plat_right = plat_left + plat_hitbox->width;
            float plat_top = plat_pos->y + plat_hitbox->offset_y;
            float plat_bottom = plat_top + plat_hitbox->height;

            // Check for collision
            bool x_overlap = entity_right > plat_left && entity_left < plat_right;
            bool y_overlap = entity_bottom > plat_top && entity_top < plat_bottom;

            if (x_overlap && y_overlap) {
                // Calculate overlap amounts
                float overlap_left = entity_right - plat_left;
                float overlap_right = plat_right - entity_left;
                float overlap_top = entity_bottom - plat_top;
                float overlap_bottom = plat_bottom - entity_top;

                // Find smallest overlap
                float min_overlap = std::min({overlap_left, overlap_right,
                                              overlap_top, overlap_bottom});

                // Resolve collision based on smallest overlap
                if (min_overlap == overlap_top && vel->vy > 0) {
                    // Landing on platform from above
                    pos->y = plat_top - hitbox->height - hitbox->offset_y;
                    vel->vy = 0.0f;
                    grav->on_ground = true;
                } else if (min_overlap == overlap_bottom && vel->vy < 0) {
                    // Hit platform from below
                    pos->y = plat_bottom - hitbox->offset_y;
                    vel->vy = 0.0f;

                    // If player hit from below, track this platform and player position
                    if (is_player) {
                        // Check if it's a POW block
                        bool is_pow = (j < pow_blocks.size()) && pow_blocks[j];
                        if (is_pow && pow_blocks[j]->hits_remaining > 0) {
                            pow_block_hit = true;
                            pow_block_idx = j;
                        } else {
                            float player_center_x = pos->x + hitbox->offset_x + hitbox->width / 2.0f;
                            platforms_hit_from_below.push_back({j, player_center_x});
                        }
                    }
                } else if (min_overlap == overlap_left && vel->vx > 0) {
                    // Hit platform from left
                    pos->x = plat_left - hitbox->width - hitbox->offset_x;
                    vel->vx = 0.0f;
                } else if (min_overlap == overlap_right && vel->vx < 0) {
                    // Hit platform from right
                    pos->x = plat_right - hitbox->offset_x;
                    vel->vx = 0.0f;
                }
            }
        }
    }

    // Handle POW block hit - stun ALL enemies
    if (pow_block_hit) {
        pow_blocks[pow_block_idx]->hits_remaining--;
        stun_all_enemies(r, positions, velocities);

        // If POW block is depleted, mark it as inactive and hide it
        if (pow_blocks[pow_block_idx]->hits_remaining <= 0) {
            // Remove platform tag so player can't hit it anymore
            auto &plat_tags = r.get_components<component::platform_tag>();
            if (pow_block_idx < plat_tags.size() && plat_tags[pow_block_idx]) {
                r.remove_component<component::platform_tag>(entity(pow_block_idx));
            }
            // Mark POW block as depleted (will be rendered with background cover)
            pow_blocks[pow_block_idx]->hits_remaining = -1;
            std::cout << "[Mario] POW block depleted!" << std::endl;
        }
    }

    // Stun enemies on platforms that were hit from below
    for (const auto &hit : platforms_hit_from_below) {
        stun_enemies_on_platform(r, hit.first, hit.second, positions, hitboxes,
                                 gravities, velocities);
    }
}

} // namespace systems
