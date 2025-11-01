/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** render_system
*/

#include "../../app/include/settings.hpp"
#include "../../include/systems.hpp"
#include "../include/render/IRenderWindow.hpp"
#include <memory>

namespace systems {

// Helper for windows securised access to std::optional
template <typename T>
inline bool safe_optional_check(const std::optional<T> &opt) {
#ifdef _WIN32
    return opt.has_value();
#else
    return static_cast<bool>(opt);
#endif
}

template <typename T>
inline T *safe_optional_ptr(std::optional<T> &opt) {
#ifdef _WIN32
    return opt.has_value() ? &(*opt) : nullptr;
#else
    return opt ? &(*opt) : nullptr;
#endif
}

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
    if (draw.texture || draw.texture_path.empty())
        return true;

    draw.texture = std::shared_ptr<render::ITexture>(window.createTexture());
    if (!draw.texture->loadFromFile(draw.texture_path))
        return false;

    draw.sprite = std::shared_ptr<render::ISprite>(window.createSprite());
    draw.sprite->setTexture(*draw.texture);
    return true;
}

static void setup_sprite_texture_rect(
    render::ISprite &sprite, const component::drawable &draw,
    const sparse_array<component::animation> &animations, size_t entity_idx) {
    bool has_animation = (entity_idx < animations.size()) &&
                         safe_optional_check(animations[entity_idx]) &&
                         !animations[entity_idx]->frames.empty();

    if (has_animation) {
        const auto &anim = animations[entity_idx];
        if (safe_optional_check(anim) &&
            anim->current_frame < anim->frames.size()) {
            sprite.setTextureRect(anim->frames[anim->current_frame]);
        }
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
    if (!load_sprite_texture(draw, window))
        return;
    if (!draw.sprite)
        return;

    draw.sprite->setPosition(pos.x, pos.y);
    draw.sprite->setScale(draw.scale, draw.scale);

    setup_sprite_texture_rect(*draw.sprite, draw, animations, entity_idx);
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
    sparse_array<component::animation> &animations =
        r.get_components<component::animation>();
    sparse_array<component::background> &backgrounds =
        r.get_components<component::background>();
    sparse_array<component::enemy_stunned> &stunneds =
        r.get_components<component::enemy_stunned>();

    Settings &settings = Settings::getInstance();
    render::IShader *colorblindShader = settings.getColorblindShader(window);

    // Render backgrounds
    for (size_t i = 0; i < backgrounds.size(); ++i) {
        if (safe_optional_check(backgrounds[i])) {
            render_background(*backgrounds[i], window, colorblindShader, dt);
        }
    }

    // Process animations
    size_t max_anim_entities = std::min(animations.size(), drawables.size());
    for (size_t i = 0; i < max_anim_entities; ++i) {
        auto *anim = safe_optional_ptr(animations[i]);
        auto *drawable = safe_optional_ptr(drawables[i]);

        if (!anim || !drawable || !anim->playing || anim->frames.empty())
            continue;

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

    // Render entities
    size_t max_entities = std::min(positions.size(), drawables.size());
    for (size_t i = 0; i < max_entities; ++i) {
        auto *pos = safe_optional_ptr(positions[i]);
        auto *draw = safe_optional_ptr(drawables[i]);

        if (!pos || !draw)
            continue;

        render::Color original_color = draw->color;
        auto *stunned =
            (i < stunneds.size()) ? safe_optional_ptr(stunneds[i]) : nullptr;
        if (stunned && stunned->stunned && draw->tag == "enemy") {
            // Make stunned enemies gray
            draw->color = render::Color(128, 128, 128);
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