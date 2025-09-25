#include "../include/systems.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <memory>

namespace systems {


void position_system(registry& r,
                     sparse_array<component::position>& positions,
                     sparse_array<component::velocity>& velocities,
                     sparse_array<component::input>& inputs,
                     sf::RenderWindow& window,
                     float current_time,
                     float dt) {
    auto& weapons = r.get_components<component::weapon>();
    auto& projectiles = r.get_components<component::projectile>();
    auto& ai_inputs = r.get_components<component::ai_input>();
    sf::Vector2u window_size = window.getSize();

    // Handle weapon firing and projectile creation
    for (size_t i = 0; i < std::min({weapons.size(), positions.size()}); ++i) {
        auto& weapon = weapons[i];
        auto& pos = positions[i];

        // Check if entity can fire (either player input or AI input)
        bool can_fire_input = false;
        if (i < inputs.size() && inputs[i] && inputs[i]->fire) {
            can_fire_input = true;
        } else if (i < ai_inputs.size() && ai_inputs[i] && ai_inputs[i]->fire) {
            can_fire_input = true;
        }

        if (weapon && pos && can_fire_input) {
            bool can_fire = false;

            if (weapon->is_burst) {
                float time_since_last_burst = current_time - weapon->last_burst_time;
                if (weapon->current_burst == 0) {
                    // Start new burst
                    float time_since_last_shot = current_time - weapon->last_shot_time;
                    float min_interval = 1.0f / weapon->fire_rate;
                    if (time_since_last_shot >= min_interval) {
                        can_fire = true;
                        weapon->last_shot_time = current_time;
                        weapon->current_burst = 1;
                        weapon->last_burst_time = current_time;
                    }
                } else if (weapon->current_burst < weapon->burst_count) {
                    // Continue burst
                    if (time_since_last_burst >= weapon->burst_interval) {
                        can_fire = true;
                        weapon->current_burst++;
                        weapon->last_burst_time = current_time;
                    }
                } else {
                    // End burst
                    weapon->current_burst = 0;
                }
            } else {
                // Standard fire rate check for non-burst weapons
                float time_since_last_shot = current_time - weapon->last_shot_time;
                float min_interval = 1.0f / weapon->fire_rate;
                if (time_since_last_shot >= min_interval) {
                    can_fire = true;
                    weapon->last_shot_time = current_time;
                }
            }

            if (can_fire) {
                // Création de projectiles basiques - le comportement est géré par projectile_behavior
                float offset_x = weapon->friendly ? 40.0f : -10.0f;
                float start_angle = 0.0f;
                if (weapon->projectile_count > 1) {
                    start_angle = -weapon->spread_angle * (weapon->projectile_count - 1) / 2.0f;
                }

                for (int i = 0; i < weapon->projectile_count; ++i) {
                    auto projectile_entity = r.spawn_entity();

                    // Créer le projectile avec les paramètres de l'arme
                    r.add_component<component::projectile>(projectile_entity,
                        component::projectile(weapon->projectile_damage, weapon->projectile_speed,
                                            weapon->friendly, "bullet", weapon->projectile_lifetime,
                                            weapon->projectile_piercing, weapon->projectile_max_hits));

                    r.add_component<component::position>(projectile_entity,
                        component::position(pos->x + offset_x, pos->y + 20.0f));

                    // Vélocité de base - le comportement sera modifié par projectile_behavior
                    float base_direction = weapon->friendly ? 1.0f : -1.0f;
                    float vx = base_direction * weapon->projectile_speed;
                    float vy = 0.0f;

                    // Spread uniquement pour les tirs multiples
                    if (weapon->projectile_count > 1) {
                        float angle_deg = start_angle + i * weapon->spread_angle;
                        float angle_rad = angle_deg * M_PI / 180.0f;
                        vx = base_direction * weapon->projectile_speed * std::cos(angle_rad);
                        vy = base_direction * weapon->projectile_speed * std::sin(angle_rad);
                    }

                    r.add_component<component::velocity>(projectile_entity,
                        component::velocity(vx, vy));

                    // Le projectile_behavior gère le pattern de mouvement
                    r.add_component<component::projectile_behavior>(projectile_entity,
                        component::projectile_behavior(weapon->movement_pattern));

                    r.add_component<component::drawable>(projectile_entity,
                        component::drawable("assets/sprites/r-typesheet1.gif",
                            weapon->projectile_sprite_rect, 1.0f, "projectile"));
                }
            }
        }
    }

    // Handle boss bounce movement with drawable tag check
    auto& drawables = r.get_components<component::drawable>();
    for (size_t i = 0; i < std::min({drawables.size(), positions.size(), velocities.size()}); ++i) {
        auto& drawable = drawables[i];
        auto& pos = positions[i];
        auto& vel = velocities[i];

        if (drawable && drawable->tag == "boss" && pos && vel) {
            // Check bounds and reverse direction
            if (pos->y <= 50.0f) {
                vel->vy = std::abs(vel->vy);
            } else if (pos->y >= static_cast<float>(window_size.y) - 100.0f) {
                vel->vy = -std::abs(vel->vy);
            }
        }
    }

    // Handle position updates and projectile cleanup
    for (size_t i = 0; i < std::min(positions.size(), velocities.size()); ++i) {
        auto& pos = positions[i];
        auto& vel = velocities[i];
        if (pos && vel) {
            // Wave movement for enemies is now handled by projectile_behavior component

            pos->x += vel->vx * dt;
            pos->y += vel->vy * dt;

            // Remove projectiles that are off-screen (now handled by projectile_system)
            if (i < projectiles.size() && projectiles[i]) {
                if (pos->x < -50.0f || pos->x > static_cast<float>(window_size.x) + 50.0f ||
                    pos->y < -50.0f || pos->y > static_cast<float>(window_size.y) + 50.0f) {
                    r.kill_entity(entity(i));
                }
            }
        }
    }
}

void control_system(registry& r,
                    sparse_array<component::controllable>& controllables,
                    sparse_array<component::velocity>& velocities,
                    sparse_array<component::input>& inputs,
                    float /*dt*/) {
    auto& animations = r.get_components<component::animation>();
    auto& drawables = r.get_components<component::drawable>();

    for (size_t i = 0; i < std::min({controllables.size(), velocities.size(), inputs.size()}); ++i) {
        auto& ctrl = controllables[i];
        auto& vel = velocities[i];
        auto& input = inputs[i];
        if (ctrl && vel && input) {
            vel->vx = vel->vy = 0;
            if (input->left) vel->vx = -ctrl->speed;
            if (input->right) vel->vx = ctrl->speed;
            if (input->up) vel->vy = -ctrl->speed;
            if (input->down) vel->vy = ctrl->speed;

            // Update player animation based on vertical movement direction
            if (i < animations.size() && animations[i] && i < drawables.size() && drawables[i] && drawables[i]->tag == "player") {
                auto& anim = animations[i];

                if (vel->vy < 0) {  // Moving up - play anim 3, then 4, stay on 4
                    if (ctrl->last_vy >= 0) {  // Just started moving up
                        anim->reverse = false;
                        anim->current_frame = 3;  // Start at frame 3
                        anim->playing = true;
                        anim->current_time = 0.0f;
                        anim->loop = false;  // Don't loop when moving up
                    } else if (anim->current_frame >= 4) {  // Reached frame 4, stay there
                        anim->playing = false;
                        anim->current_frame = 4;
                    }
                } else if (vel->vy > 0) {  // Moving down - just show frame 3
                    anim->playing = false;
                    anim->current_frame = 3;  // Frame 3: slight down
                } else {  // Not moving vertically - stop animation at middle frame
                    anim->playing = false;
                    anim->current_frame = 2;  // Middle frame (neutral position)
                    anim->loop = true;  // Reset loop for next movement
                }
                ctrl->last_vy = vel->vy;
            }
        }
    }
}

void draw_system(registry& r,
                 sparse_array<component::position>& positions,
                 sparse_array<component::drawable>& drawables,
                 sf::RenderWindow& window,
                 float dt) {
    auto& animations = r.get_components<component::animation>();
    auto& backgrounds = r.get_components<component::background>();

    // Handle background rendering
    for (size_t i = 0; i < backgrounds.size(); ++i) {
        auto& bg = backgrounds[i];
        if (bg) {
            bg->offset_x -= bg->scroll_speed * dt;

            sf::Vector2u window_size = window.getSize();
            sf::Vector2u texture_size = bg->texture.getSize();

            if (bg->offset_x <= -static_cast<float>(texture_size.x)) {
                bg->offset_x = 0.0f;
            }

            sf::Sprite sprite1(bg->texture);
            sprite1.setPosition(bg->offset_x, 0);
            sprite1.setScale(
                static_cast<float>(window_size.x) / texture_size.x,
                static_cast<float>(window_size.y) / texture_size.y
            );

            sf::Sprite sprite2(bg->texture);
            sprite2.setPosition(bg->offset_x + static_cast<float>(window_size.x), 0);
            sprite2.setScale(
                static_cast<float>(window_size.x) / texture_size.x,
                static_cast<float>(window_size.y) / texture_size.y
            );

            window.draw(sprite1);
            window.draw(sprite2);
        }
    }

    // Handle animation updates
    for (size_t i = 0; i < std::min(animations.size(), drawables.size()); ++i) {
        auto& anim = animations[i];
        auto& drawable = drawables[i];

        if (anim && drawable && anim->playing && !anim->frames.empty()) {
            anim->current_time += dt;

            if (anim->current_time >= anim->frame_duration) {
                anim->current_time = 0.0f;

                if (anim->reverse) {
                    if (anim->current_frame == 0) {
                        if (anim->loop) {
                            anim->current_frame = anim->frames.size() - 1;
                        } else {
                            anim->playing = false;
                            if (anim->destroy_on_finish) {
                                r.kill_entity(entity(i));
                            }
                        }
                    } else {
                        anim->current_frame--;
                    }
                } else {
                    anim->current_frame++;
                    if (anim->current_frame >= anim->frames.size()) {
                        if (anim->loop) {
                            anim->current_frame = 0;
                        } else {
                            anim->current_frame = anim->frames.size() - 1;
                            anim->playing = false;
                            if (anim->destroy_on_finish) {
                                r.kill_entity(entity(i));
                            }
                        }
                    }
                }
            }
        }
    }


    // Handle entity rendering
    for (size_t i = 0; i < std::min(positions.size(), drawables.size()); ++i) {
        auto& pos = positions[i];
        auto& draw = drawables[i];

        if (pos && draw) {
            if (draw->use_sprite && draw->texture.getSize().x > 0) {
                sf::Sprite sprite(draw->texture);

                // Position sprite at entity position (not centered in hitbox)
                sprite.setPosition(pos->x, pos->y);

                sprite.setScale(draw->scale, draw->scale);

                // Use animation frame if available, otherwise use default sprite_rect
                if (i < animations.size() && animations[i] && !animations[i]->frames.empty()) {
                    sprite.setTextureRect(animations[i]->frames[animations[i]->current_frame]);
                } else if (draw->sprite_rect.width > 0 && draw->sprite_rect.height > 0) {
                    sprite.setTextureRect(draw->sprite_rect);
                }

                window.draw(sprite);
            } else {
                sf::RectangleShape shape(sf::Vector2f(draw->size, draw->size));
                shape.setPosition(pos->x, pos->y);
                shape.setFillColor(draw->color);
                window.draw(shape);
            }
        }
    }
}


void collision_system(registry& r,
                      sparse_array<component::position>& positions,
                      sparse_array<component::drawable>& drawables,
                      sparse_array<component::projectile>& projectiles,
                      sparse_array<component::hitbox>& hitboxes) {
    std::vector<size_t> entities_to_kill;
    std::vector<std::pair<float, float>> explosion_positions;
    auto& scores = r.get_components<component::score>();
    auto& healths = r.get_components<component::health>();

    for (size_t proj_idx = 0; proj_idx < projectiles.size(); ++proj_idx) {
        auto& projectile = projectiles[proj_idx];
        auto& proj_pos = positions[proj_idx];

        if (!projectile || !proj_pos) continue;

        for (size_t target_idx = 0; target_idx < std::min(positions.size(), drawables.size()); ++target_idx) {
            auto& target_pos = positions[target_idx];
            auto& target_drawable = drawables[target_idx];

            if (!target_pos || !target_drawable || target_idx == proj_idx) continue;

            // Skip collision if it's a projectile hitting another projectile
            if (projectiles[target_idx]) continue;

            // Use hitboxes for collision detection if available
            float proj_width = 13.0f;
            float proj_height = 8.0f;
            float proj_left = proj_pos->x;
            float proj_top = proj_pos->y;

            float target_width, target_height, target_left, target_top;

            // Check if target has hitbox component for more precise collision
            if (target_idx < hitboxes.size() && hitboxes[target_idx]) {
                auto& target_hitbox = hitboxes[target_idx];
                target_width = target_hitbox->width;
                target_height = target_hitbox->height;
                // Position hitbox to the right and down from sprite center
                target_left = target_pos->x + target_hitbox->offset_x;
                target_top = target_pos->y + target_hitbox->offset_y;
            } else {
                // Fallback to drawable size
                if (target_drawable->use_sprite && target_drawable->sprite_rect.width > 0) {
                    target_width = target_drawable->sprite_rect.width * target_drawable->scale;
                    target_height = target_drawable->sprite_rect.height * target_drawable->scale;
                } else {
                    target_width = target_drawable->size;
                    target_height = target_drawable->size;
                }
                target_left = target_pos->x;
                target_top = target_pos->y;
            }

            // AABB collision detection
            bool collision = (proj_left < target_left + target_width &&
                             proj_left + proj_width > target_left &&
                             proj_top < target_top + target_height &&
                             proj_top + proj_height > target_top);

            if (collision) {
                // Check if this is a valid collision (friendly fire rules)
                bool is_player = target_drawable->tag == "player";
                bool is_enemy = target_drawable->tag == "enemy" || target_drawable->tag == "enemy_zigzag" || target_drawable->tag == "boss";

                if ((projectile->friendly && is_enemy) || (!projectile->friendly && is_player)) {
                    // Apply damage if target has health component
                    if (target_idx < healths.size() && healths[target_idx]) {
                        auto& target_health = healths[target_idx];
                        // Mark damage to be applied by health_system
                        target_health->pending_damage += static_cast<int>(projectile->damage);

                        // Create explosion effect when player is hit
                        if (is_player) {
                            explosion_positions.push_back({target_pos->x, target_pos->y});
                        }
                        // Create explosion effect when player projectile hits boss
                        else if (projectile->friendly && target_drawable->tag == "boss") {
                            explosion_positions.push_back({target_pos->x, target_pos->y});
                        }
                    } else {
                        // No health component, kill immediately (old behavior)
                        if (projectile->friendly && is_enemy) {
                            for (size_t score_idx = 0; score_idx < scores.size(); ++score_idx) {
                                auto& score = scores[score_idx];
                                if (score && score_idx < drawables.size() && drawables[score_idx] && drawables[score_idx]->tag == "player") {
                                    score->current_score += 5;
                                    score->enemies_killed += 1;
                                    break;
                                }
                            }
                        }
                        entities_to_kill.push_back(target_idx);
                        explosion_positions.push_back({target_pos->x, target_pos->y});
                    }

                    // Handle piercing projectiles
                    if (projectile->piercing) {
                        projectile->hits++;
                        // Don't break - piercing projectiles can hit multiple targets in one frame
                    } else {
                        // Non-piercing projectiles are destroyed on impact
                        entities_to_kill.push_back(proj_idx);
                        break;
                    }
                }
            }
        }
    }

    // Check for player-enemy collisions
    for (size_t player_idx = 0; player_idx < std::min(positions.size(), drawables.size()); ++player_idx) {
        auto& player_pos = positions[player_idx];
        auto& player_drawable = drawables[player_idx];
        auto& player_hitbox = hitboxes[player_idx];

        if (!player_pos || !player_drawable || !player_hitbox || player_drawable->tag != "player") continue;

        for (size_t enemy_idx = 0; enemy_idx < std::min(positions.size(), drawables.size()); ++enemy_idx) {
            auto& enemy_pos = positions[enemy_idx];
            auto& enemy_drawable = drawables[enemy_idx];
            auto& enemy_hitbox = hitboxes[enemy_idx];

            if (!enemy_pos || !enemy_drawable || !enemy_hitbox ||
                (enemy_drawable->tag != "enemy" && enemy_drawable->tag != "enemy_zigzag" && enemy_drawable->tag != "boss")) continue;
            if (enemy_idx == player_idx) continue;

            // AABB collision detection using hitboxes
            float player_left = player_pos->x + player_hitbox->offset_x;
            float player_right = player_left + player_hitbox->width;
            float player_top = player_pos->y + player_hitbox->offset_y;
            float player_bottom = player_top + player_hitbox->height;

            float enemy_left = enemy_pos->x + enemy_hitbox->offset_x;
            float enemy_right = enemy_left + enemy_hitbox->width;
            float enemy_top = enemy_pos->y + enemy_hitbox->offset_y;
            float enemy_bottom = enemy_top + enemy_hitbox->height;

            bool collision = (player_left < enemy_right &&
                             player_right > enemy_left &&
                             player_top < enemy_bottom &&
                             player_bottom > enemy_top);

            if (collision) {
                // Apply damage to both entities if they have health
                if (player_idx < healths.size() && healths[player_idx]) {
                    auto& player_health = healths[player_idx];
                    // Mark damage to be applied by health_system
                    player_health->pending_damage += 50;
                } else {
                    entities_to_kill.push_back(player_idx);
                    explosion_positions.push_back({player_pos->x, player_pos->y});
                }

                if (enemy_idx < healths.size() && healths[enemy_idx]) {
                    auto& enemy_health = healths[enemy_idx];
                    // Mark damage to be applied by health_system
                    enemy_health->pending_damage += 50;
                } else {
                    entities_to_kill.push_back(enemy_idx);
                    explosion_positions.push_back({enemy_pos->x, enemy_pos->y});
                }
                break;
            }
        }
    }

    // Remove duplicates and kill entities
    std::sort(entities_to_kill.begin(), entities_to_kill.end());
    entities_to_kill.erase(std::unique(entities_to_kill.begin(), entities_to_kill.end()), entities_to_kill.end());

    for (auto entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }

    // Create explosion effects
    for (const auto& explosion_pos : explosion_positions) {
        auto explosion_entity = r.spawn_entity();
        r.add_component<component::position>(explosion_entity, component::position(explosion_pos.first, explosion_pos.second));
        r.add_component<component::drawable>(explosion_entity, component::drawable("assets/sprites/r-typesheet1.gif", sf::IntRect(70, 290, 36, 32), 2.0f, "explosion"));

        // Set up explosion animation frames with auto-destruction
        auto& anim = r.add_component<component::animation>(explosion_entity, component::animation(0.1f, false, true));
        anim->frames.push_back(sf::IntRect(70, 290, 36, 32));
        anim->frames.push_back(sf::IntRect(106, 290, 36, 32));
        anim->frames.push_back(sf::IntRect(142, 290, 36, 32));
        anim->frames.push_back(sf::IntRect(178, 290, 35, 32));
        anim->playing = true;
        anim->current_frame = 0;
    }
}

void audio_system(registry& /*r*/,
                  sparse_array<component::sound_effect>& sound_effects,
                  sparse_array<component::music>& musics,
                  sparse_array<component::audio_trigger>& triggers) {

    static std::unordered_map<std::string, sf::SoundBuffer> sound_buffers;
    static std::unordered_map<std::string, sf::Sound> sounds;
    static std::unordered_map<std::string, std::unique_ptr<sf::Music>> music_tracks;

    // Handle sound effects
    for (size_t i = 0; i < std::min(sound_effects.size(), triggers.size()); ++i) {
        auto& sound_effect = sound_effects[i];
        auto& trigger = triggers[i];

        if (sound_effect && trigger && !trigger->triggered) {
            // Load sound buffer if not already loaded
            if (sound_buffers.find(sound_effect->sound_path) == sound_buffers.end()) {
                sf::SoundBuffer buffer;
                if (buffer.loadFromFile(sound_effect->sound_path)) {
                    sound_buffers[sound_effect->sound_path] = buffer;
                    sounds[sound_effect->sound_path] = sf::Sound();
                    sounds[sound_effect->sound_path].setBuffer(sound_buffers[sound_effect->sound_path]);
                }
            }

            // Play sound if buffer exists
            if (sound_buffers.find(sound_effect->sound_path) != sound_buffers.end()) {
                auto& sound = sounds[sound_effect->sound_path];
                sound.setVolume(sound_effect->volume);
                sound.play();
                sound_effect->is_playing = true;

                if (sound_effect->play_once) {
                    trigger->triggered = true;
                }
            }
        }

        // Update playing status
        if (sound_effect && sound_effect->is_playing) {
            if (sounds.find(sound_effect->sound_path) != sounds.end()) {
                auto& sound = sounds[sound_effect->sound_path];
                if (sound.getStatus() != sf::Sound::Playing) {
                    sound_effect->is_playing = false;
                }
            }
        }
    }

    // Handle music
    for (size_t i = 0; i < std::min(musics.size(), triggers.size()); ++i) {
        auto& music = musics[i];
        auto& trigger = triggers[i];

        if (music && trigger && !trigger->triggered && !music->is_playing) {
            // Load music if not already loaded
            if (music_tracks.find(music->music_path) == music_tracks.end()) {
                music_tracks[music->music_path] = std::make_unique<sf::Music>();
            }

            auto& track = music_tracks[music->music_path];
            if (track->openFromFile(music->music_path)) {
                track->setVolume(music->volume);
                track->setLoop(music->loop);
                track->play();
                music->is_playing = true;
                trigger->triggered = true;
            }
        }

        // Update playing status
        if (music && music->is_playing) {
            if (music_tracks.find(music->music_path) != music_tracks.end()) {
                auto& track = music_tracks[music->music_path];
                if (track->getStatus() != sf::Music::Playing) {
                    music->is_playing = false;
                }
            }
        }
    }
}

void input_system(registry& /*r*/,
                  sparse_array<component::input>& inputs) {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& input = inputs[i];
        if (input) {
            // Sauvegarder l'état précédent pour détecter les pressions
            bool prev_left = input->left;
            bool prev_right = input->right;
            bool prev_up = input->up;
            bool prev_down = input->down;
            bool prev_fire = input->fire;

            // Mettre à jour l'état actuel des touches
            input->left = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
            input->right = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
            input->up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
            input->down = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
            input->fire = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

            // Détecter les moments où les touches sont pressées (transition de false à true)
            input->left_pressed = !prev_left && input->left;
            input->right_pressed = !prev_right && input->right;
            input->up_pressed = !prev_up && input->up;
            input->down_pressed = !prev_down && input->down;
            input->fire_pressed = !prev_fire && input->fire;
        }
    }
}

void projectile_system(registry& r,
                       sparse_array<component::projectile>& projectiles,
                       sparse_array<component::position>& positions,
                       float dt) {
    auto& velocities = r.get_components<component::velocity>();
    auto& behaviors = r.get_components<component::projectile_behavior>();
    std::vector<size_t> entities_to_kill;

    for (size_t i = 0; i < std::min({projectiles.size(), positions.size(), velocities.size()}); ++i) {
        auto& projectile = projectiles[i];
        auto& pos = positions[i];
        auto& vel = velocities[i];

        if (projectile && pos && vel) {
            // Update projectile age
            projectile->age += dt;

            // Check if projectile has exceeded its lifetime
            if (projectile->age >= projectile->lifetime) {
                entities_to_kill.push_back(i);
                continue;
            }

            // Check if piercing projectile has hit maximum targets
            if (projectile->piercing && projectile->hits >= projectile->max_hits) {
                entities_to_kill.push_back(i);
                continue;
            }

            // Apply movement behavior - délègue tout au composant
            if (i < behaviors.size() && behaviors[i]) {
                auto& behavior = behaviors[i];
                behavior->pattern.apply_pattern(vel->vx, vel->vy, pos->x, pos->y,
                                              projectile->age, projectile->speed, projectile->friendly);
            }
        }
    }

    // Kill expired projectiles
    for (auto entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }
}

void ai_input_system(registry& r,
                     sparse_array<component::ai_input>& ai_inputs,
                     float dt) {
    auto& positions = r.get_components<component::position>();
    auto& velocities = r.get_components<component::velocity>();

    for (size_t i = 0; i < ai_inputs.size(); ++i) {
        auto& ai_input = ai_inputs[i];
        if (ai_input) {
            // Handle firing logic
            ai_input->fire_timer += dt;
            if (ai_input->fire_timer >= ai_input->fire_interval) {
                ai_input->fire = true;
                ai_input->fire_timer = 0.0f;
            } else {
                ai_input->fire = false;
            }

            // Apply movement pattern if entity has position and velocity
            if (i < positions.size() && positions[i] && i < velocities.size() && velocities[i]) {
                auto& pos = positions[i];
                auto& vel = velocities[i];

                // Only apply movement if base_speed is not zero (avoid overriding boss bounce)
                if (ai_input->movement_pattern.base_speed != 0.0f) {
                    ai_input->movement_pattern.apply_pattern(vel->vx, vel->vy, pos->x, pos->y, dt);
                }
            }
        }
    }
}

void score_system(registry& r,
                  sparse_array<component::score>& scores,
                  float dt) {
    for (size_t i = 0; i < scores.size(); ++i) {
        auto& score = scores[i];
        if (score) {
            // Update survival time
            score->survival_time += dt;

            // Award 1 point every second for survival
            if (score->survival_time - score->last_time_point_awarded >= 1.0f) {
                score->current_score += 1;
                score->last_time_point_awarded = score->survival_time;
            }
        }
    }
}

void health_system(registry& r,
                   sparse_array<component::health>& healths,
                   float dt) {
    std::vector<size_t> entities_to_kill;
    auto& positions = r.get_components<component::position>();
    auto& drawables = r.get_components<component::drawable>();
    auto& scores = r.get_components<component::score>();

    for (size_t i = 0; i < healths.size(); ++i) {
        auto& health = healths[i];
        if (health) {
            // Apply pending damage
            if (health->pending_damage > 0) {
                health->current_hp -= health->pending_damage;
            }
            health->pending_damage = 0; // Reset pending damage

            // Check if entity should die
            if (health->current_hp <= 0) {
                entities_to_kill.push_back(i);

                // Create explosion effect
                if (i < positions.size() && positions[i]) {
                    auto explosion_entity = r.spawn_entity();
                    r.add_component<component::position>(explosion_entity,
                        component::position(positions[i]->x, positions[i]->y));
                    r.add_component<component::drawable>(explosion_entity,
                        component::drawable("assets/sprites/r-typesheet1.gif",
                                          sf::IntRect(70, 290, 36, 32), 2.0f, "explosion"));
                    auto& anim = r.add_component<component::animation>(explosion_entity,
                        component::animation(0.1f, false, true));
                    anim->frames.push_back(sf::IntRect(70, 290, 36, 32));
                    anim->frames.push_back(sf::IntRect(106, 290, 36, 32));
                    anim->frames.push_back(sf::IntRect(142, 290, 36, 32));
                    anim->frames.push_back(sf::IntRect(178, 290, 35, 32));
                    anim->playing = true;
                    anim->current_frame = 0;
                }

                // Award points if an enemy died and there's a player
                if (i < drawables.size() && drawables[i] &&
                    (drawables[i]->tag == "enemy" || drawables[i]->tag == "enemy_zigzag" || drawables[i]->tag == "boss")) {
                    for (size_t score_idx = 0; score_idx < scores.size(); ++score_idx) {
                        auto& score = scores[score_idx];
                        if (score && score_idx < drawables.size() && drawables[score_idx] &&
                            drawables[score_idx]->tag == "player") {
                            score->current_score += 5;
                            score->enemies_killed += 1;
                            break;
                        }
                    }
                }
            }

            // Clamp health between 0 and max_hp
            health->current_hp = std::max(0, std::min(health->current_hp, health->max_hp));
        }
    }

    // Kill entities that should die
    for (auto entity_idx : entities_to_kill) {
        r.kill_entity(entity(entity_idx));
    }
}

}
