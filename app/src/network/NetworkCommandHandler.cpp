#include "network/NetworkCommandHandler.hpp"
#include "systems.hpp"
#include <atomic>
#include <cmath>
#include <iostream>
#include <mutex>
#include <set>

NetworkCommandHandler::NetworkCommandHandler(registry &registry,
                                             render::IRenderWindow &window,
                                             PlayerManager &player_mgr,
                                             EnemyManager &enemy_mgr,
                                             BossManager &boss_mgr)
    : registry_(registry), window_(window), player_manager_(player_mgr),
      enemy_manager_(enemy_mgr), boss_manager_(boss_mgr) {}

void NetworkCommandHandler::onCreateEntity(
    const network::CreateEntityCommand &cmd) {
    entity new_entity(0);

    switch (cmd.entity_type) {
    case network::EntityType::PLAYER:
        new_entity = createPlayerEntity(cmd);
        break;

    case network::EntityType::ENEMY:
        new_entity = createEnemyEntity(cmd);
        break;

    case network::EntityType::ENEMY_LEVEL2:
        new_entity = createEnemyLevel2Entity(cmd);
        break;

    case network::EntityType::ENEMY_LEVEL2_SPREAD:
        new_entity = createEnemyLevel2SpreadEntity(cmd);
        break;

    case network::EntityType::BOSS:
        new_entity = createBossEntity(cmd);
        break;

    case network::EntityType::BOSS_LEVEL2_PART1:
        new_entity = createBossLevel2Part1Entity(cmd);
        break;

    case network::EntityType::BOSS_LEVEL2_PART2:
        new_entity = createBossLevel2Part2Entity(cmd);
        break;

    case network::EntityType::BOSS_LEVEL2_PART3:
        new_entity = createBossLevel2Part3Entity(cmd);
        break;

    case network::EntityType::PROJECTILE:
    case network::EntityType::ALLIED_PROJECTILE:
        new_entity = createProjectileEntity(cmd);
        break;

    case network::EntityType::POWERUP_SHIELD:
    case network::EntityType::POWERUP_SPREAD:
        new_entity = createPowerUpEntity(cmd);
        break;

    case network::EntityType::POWERUP_COMPANION:
        new_entity = createPowerUpCompanionEntity(cmd);
        break;

    case network::EntityType::COMPANION:
        new_entity = createActiveCompanionEntity(cmd);
        break;

    default:
        std::cerr << "[CLIENT] Unknown entity type: "
                  << static_cast<int>(cmd.entity_type) << std::endl;
        return;
    }

    if (new_entity == entity(0)) {
        return;
    }

    registry_.add_component<component::network_entity>(
        new_entity, component::network_entity(cmd.net_id, 0, false));

    registry_.add_component<component::network_state>(
        new_entity, component::network_state());

    {
        std::lock_guard<std::mutex> lock(net_id_mutex_);
        net_id_to_entity_.emplace(cmd.net_id, new_entity);
    }

    // Track if this is our player entity
    if (cmd.entity_type == network::EntityType::PLAYER &&
        cmd.net_id == assigned_player_net_id_.load()) {
        player_entity_created_.store(true);
    }
}

void NetworkCommandHandler::onUpdateEntity(
    const network::UpdateEntityCommand &cmd) {
    auto opt_entity = findEntityByNetId(cmd.net_id);
    if (!opt_entity) {
        // Entity doesn't exist yet, create it using the entity_type from the packet
        network::CreateEntityCommand create_cmd;
        create_cmd.net_id = cmd.net_id;
        create_cmd.entity_type = cmd.entity_type;
        create_cmd.health = cmd.health;
        create_cmd.shield = cmd.shield;
        create_cmd.position_x = cmd.position_x;
        create_cmd.position_y = cmd.position_y;

        onCreateEntity(create_cmd);
        opt_entity = findEntityByNetId(cmd.net_id);
        if (!opt_entity) {
            std::cerr << "[UPDATE] Failed to create entity NET_ID=" << cmd.net_id << std::endl;
            return;
        }
    }

    entity ent = *opt_entity;

    auto &positions = registry_.get_components<component::position>();
    auto &healths = registry_.get_components<component::health>();
    auto &network_states = registry_.get_components<component::network_state>();

    if (ent < positions.size() && positions[ent]) {
        render::Vector2u window_size = window_.getSize();
        float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
        float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

        positions[ent]->x = pixel_x;
        positions[ent]->y = pixel_y;
    }

    if (ent < healths.size() && healths[ent]) {
        uint32_t old_hp = healths[ent]->current_hp;
        uint32_t new_hp = cmd.health;

        // Create explosion effect if entity took damage
        if (new_hp < old_hp && ent < positions.size() && positions[ent]) {
            systems::create_explosion(registry_, positions[ent]->x, positions[ent]->y);
        }

        healths[ent]->current_hp = new_hp;
    }

    // Update shield from server data
    auto &shields = registry_.get_components<component::shield>();
    if (cmd.shield > 0) {
        // Entity has shield
        if (ent < shields.size() && shields[ent]) {
            uint32_t old_shield = shields[ent]->current_shield;
            uint32_t new_shield = cmd.shield;

            // Create explosion effect if shield took damage
            if (new_shield < old_shield && ent < positions.size() && positions[ent]) {
                systems::create_explosion(registry_, positions[ent]->x, positions[ent]->y);
            }

            shields[ent]->current_shield = cmd.shield;
        } else {
            // Add shield component if it doesn't exist
            registry_.add_component<component::shield>(ent, component::shield(cmd.shield, 50));
        }
    } else {
        // No shield - remove component if it exists
        if (ent < shields.size() && shields[ent]) {
            shields.erase(ent);
        }
    }

    // Update score from server data (for players)
    auto &scores = registry_.get_components<component::score>();

    // Sanity check: score should be reasonable (0-100000)
    uint32_t safe_score = cmd.score;
    if (cmd.score > 100000) {
        safe_score = 0;
    }

    if (ent < scores.size() && scores[ent]) {
        scores[ent]->current_score = safe_score;

        // If this is the local player, update player_score_ for HUD display
        if (cmd.net_id == assigned_player_net_id_.load()) {
            player_score_.store(safe_score);
        }
    } else {
        // Score component doesn't exist - create it
        if (ent < scores.size()) {
            registry_.add_component<component::score>(ent, component::score(safe_score));

            if (cmd.net_id == assigned_player_net_id_.load()) {
                player_score_.store(safe_score);
            }
        }
    }

    if (ent < network_states.size() && network_states[ent]) {
        network_states[ent]->last_position.x = cmd.position_x;
        network_states[ent]->last_position.y = cmd.position_y;
        network_states[ent]->interpolation_time = 0.0f;
    }
}

void NetworkCommandHandler::onDestroyEntity(
    const network::DestroyEntityCommand &cmd) {
    auto opt_entity = findEntityByNetId(cmd.net_id);
    if (!opt_entity) {
        // Check if this was a tracked power-up (might have been destroyed before we saw it)
        powerup_net_id_to_type_.erase(cmd.net_id);
        return;
    }

    entity ent = *opt_entity;

    // Create explosion effect for destroyed entities
    auto &drawables = registry_.get_components<component::drawable>();
    auto &positions = registry_.get_components<component::position>();

    if (ent < drawables.size() && drawables[ent]) {
        const std::string &tag = drawables[ent]->tag;
        if (tag == "enemy" || tag == "enemy_zigzag" || tag == "boss") {
            // Server handles all score updates - just create visual effects
        } else if (tag == "powerup" || tag == "spread_powerup") {
            // Power-up was destroyed - check if our player collected it
            auto powerup_it = powerup_net_id_to_type_.find(cmd.net_id);
            if (powerup_it != powerup_net_id_to_type_.end()) {
                int powerup_type = powerup_it->second;

                // Check if power-up was near our player
                uint32_t my_net_id = assigned_player_net_id_.load();
                auto my_entity = findEntityByNetId(my_net_id);

                if (my_entity && ent < positions.size() && positions[ent] &&
                    *my_entity < positions.size() && positions[*my_entity]) {

                    float powerup_x = positions[ent]->x;
                    float powerup_y = positions[ent]->y;
                    float player_x = positions[*my_entity]->x;
                    float player_y = positions[*my_entity]->y;

                    // Check if within collection range (100 pixels)
                    float dx = powerup_x - player_x;
                    float dy = powerup_y - player_y;
                    float distance = std::sqrt(dx * dx + dy * dy);

                    if (distance < 100.0f) {
                        // Player collected this power-up!
                        auto &shields = registry_.get_components<component::shield>();
                        auto &weapons = registry_.get_components<component::weapon>();

                        if (powerup_type == 0) {
                            // Shield power-up - add or refresh shield
                            if (*my_entity < shields.size() && shields[*my_entity]) {
                                shields[*my_entity]->current_shield = 50;
                            } else {
                                registry_.add_component<component::shield>(
                                    *my_entity, component::shield(50, 50));
                            }
                        } else if (powerup_type == 1) {
                            // Spread power-up - upgrade weapon
                            if (*my_entity < weapons.size() && weapons[*my_entity]) {
                                weapons[*my_entity]->projectile_count = 3;
                                weapons[*my_entity]->spread_angle = 15.0f;
                            }
                        }
                    }
                }
                powerup_net_id_to_type_.erase(powerup_it);
            }
        }
    }

    registry_.kill_entity(ent);

    {
        std::lock_guard<std::mutex> lock(net_id_mutex_);
        net_id_to_entity_.erase(cmd.net_id);
    }
}

void NetworkCommandHandler::onFullStateSync(
    const network::FullStateSyncCommand &cmd) {
    std::set<uint32_t> received_net_ids;

    for (const auto &entity_cmd : cmd.entities) {
        received_net_ids.insert(entity_cmd.net_id);

        auto existing_entity = findEntityByNetId(entity_cmd.net_id);

        if (existing_entity) {
            network::UpdateEntityCommand update_cmd;
            update_cmd.net_id = entity_cmd.net_id;
            update_cmd.health = entity_cmd.health;
            update_cmd.shield = entity_cmd.shield;
            update_cmd.position_x = entity_cmd.position_x;
            update_cmd.position_y = entity_cmd.position_y;
            onUpdateEntity(update_cmd);
        } else {
            onCreateEntity(entity_cmd);
        }
    }
}

void NetworkCommandHandler::onConnectionStatusChanged(
    const network::ConnectionStatusCommand &) {
}

void NetworkCommandHandler::onPlayerAssignment(
    const network::PlayerAssignmentCommand &cmd) {
    assigned_player_net_id_.store(cmd.player_net_id);
}

std::optional<entity>
NetworkCommandHandler::findEntityByNetId(uint32_t net_id) const {
    auto it = net_id_to_entity_.find(net_id);
    if (it != net_id_to_entity_.end()) {
        return it->second;
    }
    return std::nullopt;
}

entity NetworkCommandHandler::createPlayerEntity(
    const network::CreateEntityCommand &cmd) {
    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    auto player_opt = player_manager_.createPlayer(pixel_x, pixel_y);

    if (!player_opt) {
        return entity(0);
    }

    entity player = *player_opt;

    auto &healths = registry_.get_components<component::health>();
    if (player < healths.size() && healths[player]) {
        healths[player]->current_hp = cmd.health;
        healths[player]->max_hp = cmd.health;
    }

    // Initialize shield if present
    if (cmd.shield > 0) {
        registry_.add_component<component::shield>(player, component::shield(cmd.shield, 50));
    }

    // Initialize score component (starts at 0, will be updated by server)
    registry_.add_component<component::score>(player, component::score(0));

    return player;
}

entity NetworkCommandHandler::createEnemyEntity(
    const network::CreateEntityCommand &cmd) {
    auto enemy = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        enemy, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        enemy, component::velocity(0.0f, 0.0f));

    registry_.add_component<component::drawable>(
        enemy, component::drawable("assets/sprites/r-typesheet9.gif",
                                   render::IntRect(), 1.0f, "enemy"));

    registry_.add_component<component::hitbox>(
        enemy, component::hitbox(50.0f, 58.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(enemy,
                                               component::health(cmd.health));

    float fire_interval = 1.5f;
    component::ai_movement_pattern movement_pattern =
        component::ai_movement_pattern::wave(50.0f, 0.01f, 120.0f);
    registry_.add_component<component::ai_input>(
        enemy, component::ai_input(true, fire_interval, movement_pattern));

    auto &anim = registry_.add_component<component::animation>(
        enemy, component::animation(0.5f, true));
    anim->frames.push_back(render::IntRect(0, 0, 50, 58));
    anim->frames.push_back(render::IntRect(51, 0, 57, 58));
    anim->frames.push_back(render::IntRect(116, 0, 49, 58));

    return enemy;
}

entity NetworkCommandHandler::createEnemyLevel2Entity(
    const network::CreateEntityCommand &cmd) {
    auto enemy = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        enemy, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        enemy, component::velocity(0.0f, 0.0f));

    // Level 2 enemy sprite (r-typesheet5.gif)
    registry_.add_component<component::drawable>(
        enemy, component::drawable("assets/sprites/r-typesheet5.gif",
                                   render::IntRect(6, 6, 22, 23), 2.0f,
                                   "enemy_level2"));

    registry_.add_component<component::hitbox>(
        enemy, component::hitbox(44.0f, 46.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(enemy,
                                               component::health(cmd.health));

    float fire_interval = 1.0f;
    component::ai_movement_pattern movement_pattern =
        component::ai_movement_pattern::sine_wave(80.0f, 0.02f, 110.0f);
    registry_.add_component<component::ai_input>(
        enemy, component::ai_input(true, fire_interval, movement_pattern));

    auto &anim = registry_.add_component<component::animation>(
        enemy, component::animation(0.2f, true));
    int frame_width = 22;
    int frame_height = 23;
    int start_x = 6;
    int start_y = 6;
    int spacing = 10;
    for (int i = 0; i < 8; ++i) {
        int x_pos = start_x + i * (frame_width + spacing);
        anim->frames.push_back(
            render::IntRect(x_pos, start_y, frame_width, frame_height));
    }

    return enemy;
}

entity NetworkCommandHandler::createEnemyLevel2SpreadEntity(
    const network::CreateEntityCommand &cmd) {
    auto enemy = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        enemy, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        enemy, component::velocity(0.0f, 0.0f));

    // Level 2 spread enemy (r-typesheet11.gif)
    registry_.add_component<component::drawable>(
        enemy, component::drawable("assets/sprites/r-typesheet11.gif",
                                   render::IntRect(0, 0, 34, 31), 2.0f,
                                   "enemy_spread_level2"));

    registry_.add_component<component::hitbox>(
        enemy, component::hitbox(66.0f, 62.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(enemy,
                                               component::health(cmd.health));

    float fire_interval = 0.8f;
    component::ai_movement_pattern movement_pattern =
        component::ai_movement_pattern::zigzag(70.0f, 0.018f, 150.0f);
    registry_.add_component<component::ai_input>(
        enemy, component::ai_input(true, fire_interval, movement_pattern));

    auto &anim = registry_.add_component<component::animation>(
        enemy, component::animation(0.8f, true));
    anim->frames.push_back(render::IntRect(0, 0, 34, 31));
    anim->frames.push_back(render::IntRect(34, 0, 32, 31));
    anim->frames.push_back(render::IntRect(66, 0, 33, 31));

    return enemy;
}

entity NetworkCommandHandler::createBossEntity(
    const network::CreateEntityCommand &cmd) {
    auto boss = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        boss, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        boss, component::velocity(0.0f, 100.0f));

    registry_.add_component<component::drawable>(
        boss, component::drawable("assets/sprites/r-typesheet17.gif",
                                  render::IntRect(), 2.0f, "boss"));

    registry_.add_component<component::hitbox>(
        boss, component::hitbox(130.0f, 220.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(boss,
                                               component::health(cmd.health));

    auto boss_ai = registry_.add_component<component::ai_input>(
        boss, component::ai_input(
                  true, 0.5f, component::ai_movement_pattern::straight(0.0f)));
    boss_ai->movement_pattern.base_speed = 0.0f;

    auto &boss_anim = registry_.add_component<component::animation>(
        boss, component::animation(0.1f, true));
    for (int i = 0; i < 8; ++i) {
        boss_anim->frames.push_back(render::IntRect(i * 65, 0, 65, 132));
    }

    return boss;
}

entity NetworkCommandHandler::createBossLevel2Part1Entity(
    const network::CreateEntityCommand &cmd) {
    auto boss_part = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        boss_part, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        boss_part, component::velocity(0.0f, 0.0f));

    // Part 1 (Left): sprite (24, 188, 116, 69) from r-typesheet38.gif
    registry_.add_component<component::drawable>(
        boss_part, component::drawable("assets/sprites/r-typesheet38.gif",
                                       render::IntRect(24, 188, 116, 69), 1.0f, "boss"));

    registry_.add_component<component::hitbox>(
        boss_part, component::hitbox(116.0f, 69.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(boss_part,
                                               component::health(cmd.health));

    auto boss_ai = registry_.add_component<component::ai_input>(
        boss_part, component::ai_input(false, 0.0f,
                                       component::ai_movement_pattern::sine_wave(
                                           50.0f, 1.0f, 0.0f)));
    boss_ai->movement_pattern.base_speed = 0.0f;

    return boss_part;
}

entity NetworkCommandHandler::createBossLevel2Part2Entity(
    const network::CreateEntityCommand &cmd) {
    auto boss_part = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        boss_part, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        boss_part, component::velocity(0.0f, 0.0f));

    // Part 2 (Center): sprite (141, 158, 98, 100) from r-typesheet38.gif
    registry_.add_component<component::drawable>(
        boss_part, component::drawable("assets/sprites/r-typesheet38.gif",
                                       render::IntRect(141, 158, 98, 100), 1.0f, "boss"));

    registry_.add_component<component::hitbox>(
        boss_part, component::hitbox(98.0f, 100.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(boss_part,
                                               component::health(cmd.health));

    auto boss_ai = registry_.add_component<component::ai_input>(
        boss_part, component::ai_input(false, 0.0f,
                                       component::ai_movement_pattern::sine_wave(
                                           60.0f, 1.2f, 0.0f)));
    boss_ai->movement_pattern.base_speed = 0.0f;

    return boss_part;
}

entity NetworkCommandHandler::createBossLevel2Part3Entity(
    const network::CreateEntityCommand &cmd) {
    auto boss_part = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        boss_part, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        boss_part, component::velocity(0.0f, 0.0f));

    // Part 3 (Right): sprite (240, 175, 99, 83) from r-typesheet38.gif
    registry_.add_component<component::drawable>(
        boss_part, component::drawable("assets/sprites/r-typesheet38.gif",
                                       render::IntRect(240, 175, 99, 83), 1.0f, "boss"));

    registry_.add_component<component::hitbox>(
        boss_part, component::hitbox(99.0f, 83.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(boss_part,
                                               component::health(cmd.health));

    auto boss_ai = registry_.add_component<component::ai_input>(
        boss_part, component::ai_input(false, 0.0f,
                                       component::ai_movement_pattern::sine_wave(
                                           40.0f, 0.8f, 0.0f)));
    boss_ai->movement_pattern.base_speed = 0.0f;

    return boss_part;
}

entity NetworkCommandHandler::createProjectileEntity(
    const network::CreateEntityCommand &cmd) {
    auto projectile = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    bool is_friendly =
        (cmd.entity_type == network::EntityType::ALLIED_PROJECTILE);

    registry_.add_component<component::position>(
        projectile, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        projectile, component::velocity(0.0f, 0.0f));

    std::string texture_path = "assets/sprites/r-typesheet1.gif";
    render::IntRect sprite_rect;
    std::string tag;
    float damage;

    if (is_friendly) {
        // Player projectile: same sprite as solo mode (scale 1.0)
        sprite_rect = render::IntRect(60, 353, 12, 12);
        tag = "allied_projectile";
        damage = 20.0f;  // Same as solo mode
    } else {
        // Enemy projectile: sprite from enemy_manager.cpp (solo mode)
        sprite_rect = render::IntRect(249, 103, 16, 12);
        tag = "projectile";
        damage = 25.0f;  // Same as solo mode enemy damage
    }

    // Scale 1.0 like solo mode
    registry_.add_component<component::drawable>(
        projectile, component::drawable(texture_path, sprite_rect, 1.0f, tag));

    registry_.add_component<component::projectile>(
        projectile, component::projectile(damage, 500.0f, is_friendly, "bullet",
                                          5.0f, false, 1));

    registry_.add_component<component::health>(projectile,
                                               component::health(cmd.health));

    return projectile;
}

entity NetworkCommandHandler::createPowerUpEntity(
    const network::CreateEntityCommand &cmd) {
    auto powerup = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    int powerup_type = (cmd.entity_type == network::EntityType::POWERUP_SHIELD) ? 0 : 1;

    registry_.add_component<component::position>(
        powerup, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        powerup, component::velocity(-100.0f, 0.0f));  // Moves left like solo mode

    std::string texture_path = "assets/sprites/r-typesheet2.gif";
    render::IntRect sprite_rect;
    std::string tag;

    // Track power-up for collection detection
    powerup_net_id_to_type_[cmd.net_id] = powerup_type;

    if (powerup_type == 0) {
        // Shield power-up - exact frames from solo powerup_manager.cpp
        sprite_rect = render::IntRect(159, 34, 19, 17);
        tag = "powerup";  // Must match solo mode tag for collision detection
        registry_.add_component<component::hitbox>(
            powerup, component::hitbox(42.0f, 34.0f, 0.0f, 0.0f));

        registry_.add_component<component::drawable>(
            powerup, component::drawable(texture_path, sprite_rect, 2.0f, tag));

        // Animation with exact frames from solo mode (12 frames, 0.2s per frame)
        auto &powerup_anim = registry_.add_component<component::animation>(
            powerup, component::animation(0.2f, true));
        powerup_anim->frames.push_back(render::IntRect(159, 34, 19, 17));
        powerup_anim->frames.push_back(render::IntRect(182, 34, 21, 17));
        powerup_anim->frames.push_back(render::IntRect(207, 34, 19, 17));
        powerup_anim->frames.push_back(render::IntRect(230, 34, 21, 17));
        powerup_anim->frames.push_back(render::IntRect(254, 34, 21, 17));
        powerup_anim->frames.push_back(render::IntRect(279, 34, 19, 17));
        powerup_anim->frames.push_back(render::IntRect(301, 34, 19, 17));
        powerup_anim->frames.push_back(render::IntRect(324, 34, 21, 17));
        powerup_anim->frames.push_back(render::IntRect(348, 34, 21, 17));
        powerup_anim->frames.push_back(render::IntRect(373, 34, 19, 17));
        powerup_anim->frames.push_back(render::IntRect(396, 34, 21, 17));
        powerup_anim->frames.push_back(render::IntRect(421, 34, 19, 17));
    } else {
        // Spread power-up - exact frames from solo powerup_manager.cpp
        sprite_rect = render::IntRect(119, 68, 28, 23);
        tag = "spread_powerup";  // Must match solo mode tag for collision detection
        registry_.add_component<component::hitbox>(
            powerup, component::hitbox(56.0f, 46.0f, 0.0f, 0.0f));

        registry_.add_component<component::drawable>(
            powerup, component::drawable(texture_path, sprite_rect, 2.0f, tag));

        // Animation with exact frames from solo mode (6 frames, 0.15s per frame)
        auto &powerup_anim = registry_.add_component<component::animation>(
            powerup, component::animation(0.15f, true));
        int frame_width = 28;
        int separation = 2;
        int start_x = 119;
        for (int i = 0; i < 6; ++i) {
            int x_pos = start_x + i * (frame_width + separation);
            powerup_anim->frames.push_back(render::IntRect(x_pos, 68, 28, 23));
        }
    }

    return powerup;
}

entity NetworkCommandHandler::createPowerUpCompanionEntity(
    const network::CreateEntityCommand &cmd) {
    auto pickup = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        pickup, component::position(pixel_x, pixel_y));
    registry_.add_component<component::velocity>(
        pickup, component::velocity(-100.0f, 0.0f));
    registry_.add_component<component::hitbox>(
        pickup, component::hitbox(42.0f, 34.0f, 0.0f, 0.0f));

    // Left sprite from r-typesheet27.gif (68x34, two sprites of 34x34 each)
    render::IntRect sprite_rect(0, 0, 34, 34);
    registry_.add_component<component::drawable>(
        pickup, component::drawable("assets/sprites/r-typesheet27.gif",
                                   sprite_rect, 1.5f, "companion_powerup"));

    // Track for collection detection (type 2 = companion)
    powerup_net_id_to_type_[cmd.net_id] = 2;

    return pickup;
}

entity NetworkCommandHandler::createActiveCompanionEntity(
    const network::CreateEntityCommand &cmd) {
    auto companion = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        companion, component::position(pixel_x, pixel_y));

    // Right sprite from r-typesheet27.gif
    render::IntRect sprite_rect(34, 0, 34, 34);
    registry_.add_component<component::drawable>(
        companion, component::drawable("assets/sprites/r-typesheet27.gif",
                                      sprite_rect, 1.5f, "companion"));

    return companion;
}

void NetworkCommandHandler::onRawTCPMessage(const network::TCPMessage &msg) {
    switch (msg.msg_type) {
    case network::MessageType::TCP_CONNECT_ACK:
        if (msg.data.size() >= 1) {
            network::ConnectionStatusCommand cmd;
            cmd.old_state = network::ConnectionState::CONNECTING;
            cmd.new_state = network::ConnectionState::CONNECTED;
            cmd.player_id = msg.data[0];
            cmd.udp_port = 0;
            onConnectionStatusChanged(cmd);
        } else {
            std::cerr << "Invalid TCP_CONNECT_ACK payload" << std::endl;
        }
        break;

    case network::MessageType::TCP_CONNECT_NAK: {
        network::ConnectionStatusCommand cmd;
        cmd.old_state = network::ConnectionState::CONNECTING;
        cmd.new_state = network::ConnectionState::ERROR;
        cmd.player_id = 0;
        cmd.udp_port = 0;
        onConnectionStatusChanged(cmd);
    } break;

    case network::MessageType::TCP_GAME_START:
        if (msg.data.size() >= 2) {
            uint16_t udp_port =
                (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
            network::ConnectionStatusCommand cmd;
            cmd.old_state = network::ConnectionState::WAITING;
            cmd.new_state = network::ConnectionState::GAME_STARTING;
            cmd.player_id = 0;
            cmd.udp_port = udp_port;
            onConnectionStatusChanged(cmd);
        } else {
            std::cerr << "Invalid TCP_GAME_START payload" << std::endl;
        }
        break;

    case network::MessageType::TCP_ERROR: {
        network::ConnectionStatusCommand cmd;
        cmd.old_state = network::ConnectionState::CONNECTED;
        cmd.new_state = network::ConnectionState::ERROR;
        cmd.player_id = 0;
        cmd.udp_port = 0;
        onConnectionStatusChanged(cmd);
    } break;

    default:
        break;
    }
}

void NetworkCommandHandler::onRawUDPPacket(const network::UDPPacket &packet) {
    switch (packet.msg_type) {
    case network::UDPMessageType::CLIENT_PING:
        std::cerr << "Received CLIENT_PING (unexpected on client)" << std::endl;
        break;

    case network::UDPMessageType::PLAYER_ASSIGNMENT: {
        if (packet.payload.size() != 4) {
            std::cerr << "Invalid PLAYER_ASSIGNMENT size: "
                      << packet.payload.size() << " (expected 4)" << std::endl;
            break;
        }

        uint32_t player_net_id =
            network::PacketProcessor::parsePlayerAssignment(packet.payload);
        network::PlayerAssignmentCommand cmd;
        cmd.player_net_id = player_net_id;
        onPlayerAssignment(cmd);
        break;
    }

    case network::UDPMessageType::ENTITY_CREATE: {
        if (packet.payload.size() != 21) {
            std::cerr << "Invalid ENTITY_CREATE size: " << packet.payload.size()
                      << " (expected 21)" << std::endl;
            break;
        }

        auto entity_data =
            network::PacketProcessor::parseEntityCreate(packet.payload);

        network::CreateEntityCommand cmd;
        cmd.net_id = entity_data.net_id;
        cmd.entity_type = entity_data.entity_type;
        cmd.health = entity_data.health;
        cmd.shield = entity_data.shield;
        cmd.position_x = entity_data.position_x;
        cmd.position_y = entity_data.position_y;

        onCreateEntity(cmd);
        break;
    }

    case network::UDPMessageType::ENTITY_UPDATE: {
        if (packet.payload.size() == 0 || packet.payload.size() % 25 != 0) {
            std::cerr << "Invalid ENTITY_UPDATE size: " << packet.payload.size()
                      << " (must be multiple of 25)" << std::endl;
            break;
        }

        auto updates =
            network::PacketProcessor::parseEntityUpdate(packet.payload);

        for (const auto &update : updates) {
            network::UpdateEntityCommand cmd;
            cmd.net_id = update.net_id;
            cmd.entity_type = update.entity_type;
            cmd.health = update.health;
            cmd.shield = update.shield;
            cmd.position_x = update.position_x;
            cmd.position_y = update.position_y;
            cmd.score = update.score;

            onUpdateEntity(cmd);
        }
        break;
    }

    case network::UDPMessageType::ENTITY_DESTROY: {
        if (packet.payload.size() == 0 || packet.payload.size() % 4 != 0) {
            std::cerr << "Invalid ENTITY_DESTROY size: "
                      << packet.payload.size() << " (must be multiple of 4)"
                      << std::endl;
            break;
        }

        auto net_ids =
            network::PacketProcessor::parseEntityDestroy(packet.payload);

        for (uint32_t net_id : net_ids) {
            network::DestroyEntityCommand cmd;
            cmd.net_id = net_id;

            onDestroyEntity(cmd);
        }
        break;
    }

    case network::UDPMessageType::GAME_STATE: {
        if (packet.payload.size() < 4) {
            std::cerr << "Invalid GAME_STATE size: " << packet.payload.size()
                      << " (minimum 4)" << std::endl;
            break;
        }

        if ((packet.payload.size() - 4) % 21 != 0) {
            std::cerr << "Invalid GAME_STATE size: " << packet.payload.size()
                      << " (should be 4 + N*21)" << std::endl;
            break;
        }

        auto entities =
            network::PacketProcessor::parseGameState(packet.payload);

        network::FullStateSyncCommand cmd;
        for (const auto &entity_data : entities) {
            network::CreateEntityCommand entity_cmd;
            entity_cmd.net_id = entity_data.net_id;
            entity_cmd.entity_type = entity_data.entity_type;
            entity_cmd.health = entity_data.health;
            entity_cmd.shield = entity_data.shield;
            entity_cmd.position_x = entity_data.position_x;
            entity_cmd.position_y = entity_data.position_y;

            cmd.entities.push_back(entity_cmd);
        }

        onFullStateSync(cmd);
        break;
    }

    case network::UDPMessageType::PLAYER_INPUT:
        break;

    case network::UDPMessageType::VICTORY: {
        victory_received_ = true;
        break;
    }

    default:
        std::cerr << "[NetworkCommandHandler] Unknown UDP message type: "
                  << static_cast<int>(packet.msg_type) << std::endl;
        break;
    }
}