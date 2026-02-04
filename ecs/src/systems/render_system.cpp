/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** render_system
*/

#include "../../app/include/settings.hpp"
#include "../../include/systems.hpp"
#include "../include/render/IRenderWindow.hpp"
#include <iostream>
#include <memory>

namespace systems {

static void draw_sprite_with_shader(render::IRenderWindow &window,
                                    render::ISprite &sprite,
                                    render::IShader *shader) {
    if (shader) {
        window.draw(sprite, *shader);
    } else {
        window.draw(sprite);
    }
}

static void draw_shape_with_shader(render::IRenderWindow &window,
                                   render::IShape &shape,
                                   render::IShader *shader) {
    if (shader) {
        window.draw(shape, *shader);
    } else {
        window.draw(shape);
    }
}

static void render_background(component::background &bg,
                              render::IRenderWindow &window,
                              render::IShader *shader, float dt) {
    if (!bg.texture)
        return;

    bg.offset_x -= bg.scroll_speed * dt;

    render::Vector2u window_size = window.getSize();
    render::Vector2u texture_size = bg.texture->getSize();

    if (bg.offset_x <= -static_cast<float>(window_size.x)) {
        bg.offset_x += static_cast<float>(window_size.x);
    }

    // Calculate scale to maintain aspect ratio
    // For very wide textures (like level 2), scale by height only
    float scale_x, scale_y;

    float texture_ratio = static_cast<float>(texture_size.x) / texture_size.y;
    float window_ratio = static_cast<float>(window_size.x) / window_size.y;

    if (texture_ratio > window_ratio * 2.0f) {
        // Very wide texture (like spritesheet) - scale by height and maintain
        // aspect ratio
        scale_y = static_cast<float>(window_size.y) / texture_size.y;
        scale_x = scale_y;
    } else {
        // Normal texture - scale to fill window
        scale_x = static_cast<float>(window_size.x) / texture_size.x;
        scale_y = static_cast<float>(window_size.y) / texture_size.y;
    }

    std::unique_ptr<render::ISprite> sprite1 = window.createSprite();
    sprite1->setTexture(*bg.texture);
    sprite1->setPosition(bg.offset_x, 0);
    sprite1->setScale(scale_x, scale_y);
    draw_sprite_with_shader(window, *sprite1, shader);

    std::unique_ptr<render::ISprite> sprite2 = window.createSprite();
    sprite2->setTexture(*bg.texture);
    sprite2->setPosition(bg.offset_x + static_cast<float>(window_size.x), 0);
    sprite2->setScale(scale_x, scale_y);
    draw_sprite_with_shader(window, *sprite2, shader);
}

static bool load_sprite_texture(component::drawable &draw,
                                render::IRenderWindow &window) {
    static bool debug_load = false;
    if (!debug_load && !draw.texture_path.empty()) {
        std::cout << "[Debug] load_sprite_texture - path: '" << draw.texture_path
                  << "', texture ptr: " << draw.texture.get() << std::endl;
        debug_load = true;
    }

    if (draw.texture) {
        return true;  // Already loaded
    }
    if (draw.texture_path.empty()) {
        // No texture path - this is a shape, not a sprite
        return false;
    }

    std::cout << "[Debug] Loading texture from: " << draw.texture_path << std::endl;
    draw.texture = std::shared_ptr<render::ITexture>(window.createTexture());
    if (!draw.texture->loadFromFile(draw.texture_path)) {
        std::cerr << "[Render] Failed to load texture: " << draw.texture_path << std::endl;
        return false;
    }

    // Debug: print texture size
    render::Vector2u tex_size = draw.texture->getSize();
    std::cout << "[Render] Loaded texture: " << draw.texture_path
              << " (" << tex_size.x << "x" << tex_size.y << ")" << std::endl;

    draw.sprite = std::shared_ptr<render::ISprite>(window.createSprite());
    draw.sprite->setTexture(*draw.texture);
    return true;
}

static void setup_sprite_texture_rect(
    render::ISprite &sprite, const component::drawable &draw,
    const sparse_array<component::animation> &animations, size_t entity_idx) {
    bool has_playing_animation = (entity_idx < animations.size()) &&
                                 animations[entity_idx] &&
                                 animations[entity_idx]->playing &&
                                 !animations[entity_idx]->frames.empty();

    if (has_playing_animation) {
        sprite.setTextureRect(
            animations[entity_idx]
                ->frames[animations[entity_idx]->current_frame]);
        return;
    }

    if (draw.sprite_rect.width > 0 && draw.sprite_rect.height > 0) {
        sprite.setTextureRect(draw.sprite_rect);
    }
}

static void render_sprite(component::drawable &draw,
                          const component::position &pos,
                          render::IRenderWindow &window,
                          render::IShader *shader,
                          const sparse_array<component::animation> &animations,
                          size_t entity_idx) {
    static bool debug_once = false;
    if (!debug_once && draw.tag == "enemy") {
        std::cout << "[Debug] render_sprite called for enemy, texture_path: '"
                  << draw.texture_path << "', texture: " << (draw.texture ? "loaded" : "null") << std::endl;
        debug_once = true;
    }

    if (!load_sprite_texture(draw, window)) {
        // Fallback: draw a colored rectangle if texture fails
        std::unique_ptr<render::IShape> fallback =
            window.createRectangleShape(render::Vector2f(20.0f * draw.scale, 20.0f * draw.scale));
        fallback->setPosition(pos.x, pos.y);
        fallback->setFillColor(render::Color(255, 0, 0));  // Red fallback
        window.draw(*fallback);
        return;
    }
    if (!draw.sprite)
        return;

    setup_sprite_texture_rect(*draw.sprite, draw, animations, entity_idx);

    // Apply color modulation to sprite
    draw.sprite->setColor(draw.color);

    // Get sprite width from animation frame or sprite_rect
    float sprite_width = draw.sprite_rect.width;
    if (entity_idx < animations.size() && animations[entity_idx] &&
        !animations[entity_idx]->frames.empty()) {
        sprite_width = static_cast<float>(
            animations[entity_idx]->frames[animations[entity_idx]->current_frame].width);
    }

    // Handle flip_x by using negative scale and adjusting position
    float scale_x = draw.flip_x ? -draw.scale : draw.scale;
    float offset_x = draw.flip_x ? sprite_width * draw.scale : 0.0f;

    draw.sprite->setPosition(pos.x + offset_x, pos.y);
    draw.sprite->setScale(scale_x, draw.scale);

    draw_sprite_with_shader(window, *draw.sprite, shader);
}

static void render_shape(const component::drawable &draw,
                         const component::position &pos,
                         render::IRenderWindow &window,
                         render::IShader *shader) {
    std::unique_ptr<render::IShape> shape =
        window.createRectangleShape(render::Vector2f(draw.size, draw.size));
    shape->setPosition(pos.x, pos.y);
    shape->setFillColor(draw.color);
    draw_shape_with_shader(window, *shape, shader);
}

void render_system(registry &r, sparse_array<component::position> &positions,
                   sparse_array<component::drawable> &drawables,
                   render::IRenderWindow &window, float dt) {
    sparse_array<component::animation> &animations = r.get_components<component::animation>();
    sparse_array<component::background> &backgrounds = r.get_components<component::background>();
    sparse_array<component::enemy_stunned> &stunneds = r.get_components<component::enemy_stunned>();

    Settings &settings = Settings::getInstance();
    render::IShader *colorblindShader = settings.getColorblindShader(window);

    for (size_t i = 0; i < backgrounds.size(); ++i) {
        std::optional<component::background> &bg = backgrounds[i];
        if (!bg)
            continue;
        render_background(*bg, window, colorblindShader, dt);
    }

    // Track which enemies have switched to stunned animation
    static std::vector<bool> using_stunned_anim;

    for (size_t i = 0; i < std::min(animations.size(), drawables.size()); ++i) {
        std::optional<component::animation> &anim = animations[i];
        std::optional<component::drawable> &drawable = drawables[i];

        if (!anim || !drawable || !anim->playing || anim->frames.empty())
            continue;

        // Handle animation switching for stunned enemies
        auto stunned = (i < stunneds.size()) ? stunneds[i] : std::nullopt;
        if (stunned && drawable->tag == "enemy" && !anim->frames.empty()) {
            // Ensure tracking vector is large enough
            if (using_stunned_anim.size() <= i) {
                using_stunned_anim.resize(i + 1, false);
            }

            // Get frame size from current animation frames
            int frame_w = anim->frames[0].width;
            int frame_h = anim->frames[0].height;
            bool should_use_stunned = stunned->stunned;

            if (should_use_stunned && !using_stunned_anim[i]) {
                // Switch to stunned animation (frames 5-8)
                anim->frames.clear();
                for (int f = 5; f < 9; ++f) {
                    anim->frames.push_back(render::IntRect(f * frame_w, 0, frame_w, frame_h));
                }
                anim->current_frame = 0;
                using_stunned_anim[i] = true;
            } else if (!should_use_stunned && using_stunned_anim[i]) {
                // Switch back to walking animation (frames 0-4)
                anim->frames.clear();
                for (int f = 0; f < 5; ++f) {
                    anim->frames.push_back(render::IntRect(f * frame_w, 0, frame_w, frame_h));
                }
                anim->current_frame = 0;
                using_stunned_anim[i] = false;
            }
        }

        anim->current_time += dt;

        if (anim->current_time < anim->frame_duration)
            continue;

        anim->current_time = 0;

        size_t frame_count = anim->frames.size();
        int direction = anim->reverse ? -1 : 1;
        int next_frame = static_cast<int>(anim->current_frame) + direction;

        bool at_boundary =
            (next_frame < 0) || (next_frame >= static_cast<int>(frame_count));

        if (at_boundary) {
            if (anim->loop) {
                next_frame =
                    (next_frame < 0) ? (static_cast<int>(frame_count) - 1) : 0;
            } else {
                next_frame =
                    (next_frame < 0) ? 0 : (static_cast<int>(frame_count) - 1);
                anim->playing = false;

                if (anim->destroy_on_finish) {
                    r.kill_entity(entity(i));
                }
            }
        }

        anim->current_frame = static_cast<size_t>(next_frame);
    }

    static bool debug_render_loop = false;
    if (!debug_render_loop) {
        std::cout << "[Debug] Render loop - positions.size(): " << positions.size()
                  << ", drawables.size(): " << drawables.size() << std::endl;
        debug_render_loop = true;
    }

    for (size_t i = 0; i < std::min(positions.size(), drawables.size()); ++i) {
        std::optional<component::position> &pos = positions[i];
        std::optional<component::drawable> &draw = drawables[i];

        if (!pos || !draw)
            continue;

        static bool debug_entities = false;
        if (!debug_entities && draw->tag == "enemy") {
            std::cout << "[Debug] Found enemy entity " << i << " - use_sprite: " << draw->use_sprite
                      << ", texture_path: '" << draw->texture_path << "'" << std::endl;
            debug_entities = true;
        }

        // Change color for stunned/angry enemies (Mario platformer)
        // Only apply color modulation for non-sprite drawables
        render::Color original_color = draw->color;
        auto stunned = (i < stunneds.size()) ? stunneds[i] : std::nullopt;
        if (stunned && draw->tag == "enemy" && !draw->use_sprite) {
            if (stunned->stunned) {
                // Make stunned enemies gray
                draw->color = render::Color(128, 128, 128);
            } else if (stunned->angry) {
                // Make angry enemies orange
                draw->color = render::Color(255, 165, 0);
            }
        }

        // Show POW block cover when depleted
        sparse_array<component::pow_block> &pow_blocks = r.get_components<component::pow_block>();
        if (draw->tag == "pow_block" && i < pow_blocks.size() && pow_blocks[i]) {
            if (pow_blocks[i]->hits_remaining < 0) {
                // POW depleted - show black rectangle to cover it
                draw->color = render::Color(0, 0, 0, 255);
            }
        }

        if (draw->use_sprite) {
            render_sprite(*draw, *pos, window, colorblindShader, animations, i);
        } else {
            render_shape(*draw, *pos, window, colorblindShader);
        }

        // Restore original color
        draw->color = original_color;
    }
}

} // namespace systems
