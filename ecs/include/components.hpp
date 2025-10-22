/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** components
*/

#pragma once
#include "render/IRenderWindow.hpp"
#include <cmath>
#include <functional>
#include <memory>
#include <string>

class registry;

namespace component {
struct position {
    float x, y;
    position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct velocity {
    float vx, vy;
    velocity(float vx = 0.0f, float vy = 0.0f) : vx(vx), vy(vy) {}
};

struct drawable {
    render::Color color;
    float size;
    std::shared_ptr<render::ITexture> texture;
    std::shared_ptr<render::ISprite> sprite;
    render::IntRect sprite_rect;
    bool use_sprite;
    float scale;
    std::string tag;
    std::string texture_path;

    drawable(render::Color color = render::Color::White(), float size = 50.0f)
        : color(color), size(size), texture(nullptr), sprite(nullptr),
          sprite_rect(), use_sprite(false), scale(1.0f), tag(""),
          texture_path("") {}

    drawable(const std::string &tex_path,
             render::IntRect rect = render::IntRect(), float scale = 2.0f,
             const std::string &tag = "")
        : color(render::Color::White()), size(50.0f), texture(nullptr),
          sprite(nullptr), sprite_rect(rect), use_sprite(true), scale(scale),
          tag(tag), texture_path(tex_path) {}
};

struct controllable {
    float speed;
    float last_vy;
    controllable(float speed = 200.0f) : speed(speed), last_vy(0.0f) {}
};

struct background {
    std::shared_ptr<render::ITexture> texture;
    float scroll_speed;
    float offset_x;
    background(float speed = 100.0f)
        : texture(nullptr), scroll_speed(speed), offset_x(0.0f) {}
};

struct projectile {
    float damage;
    float speed;
    bool friendly;
    std::string projectile_type;
    float lifetime;
    float age;
    bool piercing;
    int max_hits;
    int hits;

    projectile(float damage = 10.0f, float speed = 400.0f, bool friendly = true,
               const std::string &type = "bullet", float lifetime = 5.0f,
               bool piercing = false, int max_hits = 1)
        : damage(damage), speed(speed), friendly(friendly),
          projectile_type(type), lifetime(lifetime), age(0.0f),
          piercing(piercing), max_hits(max_hits), hits(0) {}
};

struct projectile_pattern {
    std::string pattern_type;
    float param1, param2, param3, param4;
    std::function<void(float &, float &, float, float, float, float, bool)>
        custom_function;

    projectile_pattern(const std::string &type = "straight", float p1 = 0.0f,
                       float p2 = 0.0f, float p3 = 0.0f, float p4 = 0.0f);

    void apply_pattern(float &vx, float &vy, float pos_x, float pos_y,
                       float age, float speed, bool friendly) const;

    static projectile_pattern straight();
    static projectile_pattern wave(float amplitude = 50.0f,
                                   float frequency = 0.01f,
                                   float phase_offset = 0.0f);
    static projectile_pattern spiral(float amplitude = 30.0f,
                                     float turn_speed = 5.0f,
                                     float phase_offset = 0.0f);
    static projectile_pattern bounce();
    static projectile_pattern spread(float spread_angle = 30.0f);
};

struct weapon {
    float fire_rate;
    float last_shot_time;
    bool friendly; // determines if this weapon shoots friendly projectiles

    // Projectile configuration
    int projectile_count;
    float spread_angle;
    projectile_pattern movement_pattern;

    // Projectile stats
    float projectile_damage;
    float projectile_speed;
    float projectile_lifetime;
    bool projectile_piercing;
    int projectile_max_hits;

    // Burst properties
    int burst_count;      // projectiles in burst
    int current_burst;    // current projectile in burst
    float burst_interval; // time between projectiles in burst
    float last_burst_time;
    bool is_burst;

    render::IntRect projectile_sprite_rect;

    // Custom fire function (overridable per weapon)
    std::function<void(registry &, const position &, bool)> fire_function;

    weapon(float rate = 2.0f, bool friendly = true, int proj_count = 1,
           float spread = 0.0f,
           const projectile_pattern &pattern = projectile_pattern::straight(),
           float damage = 20.0f, float speed = 500.0f, float lifetime = 5.0f,
           bool piercing = false, int max_hits = 1, bool burst = false,
           int burst_cnt = 3, float burst_int = 0.1f,
           render::IntRect sprite_rect = render::IntRect(60, 353, 12, 12))
        : fire_rate(rate), last_shot_time(0.0f), friendly(friendly),
          projectile_count(proj_count), spread_angle(spread),
          movement_pattern(pattern), projectile_damage(damage),
          projectile_speed(speed), projectile_lifetime(lifetime),
          projectile_piercing(piercing), projectile_max_hits(max_hits),
          burst_count(burst_cnt), current_burst(0), burst_interval(burst_int),
          last_burst_time(0.0f), is_burst(burst),
          projectile_sprite_rect(sprite_rect), fire_function(nullptr) {}

    void fire(registry &r, const position &shooter_pos, bool is_friendly);
};

struct animation {
    std::vector<render::IntRect> frames;
    float frame_duration;
    float current_time;
    size_t current_frame;
    bool loop;
    bool playing;
    bool reverse;
    bool destroy_on_finish;

    animation(float duration = 0.1f, bool loop = true, bool destroy = false)
        : frame_duration(duration), current_time(0.0f), current_frame(0),
          loop(loop), playing(true), reverse(false),
          destroy_on_finish(destroy) {}
};

struct hitbox {
    float width, height;
    float offset_x, offset_y;
    hitbox(float w = 50.0f, float h = 50.0f, float ox = 0.0f, float oy = 0.0f)
        : width(w), height(h), offset_x(ox), offset_y(oy) {}
};

struct sound_effect {
    std::string sound_path;
    float volume;
    bool play_once;
    bool is_playing;
    sound_effect(const std::string &path, float vol = 100.0f, bool once = true)
        : sound_path(path), volume(vol), play_once(once), is_playing(false) {}
};

struct music {
    std::string music_path;
    float volume;
    bool loop;
    bool is_playing;
    music(const std::string &path, float vol = 50.0f, bool should_loop = true)
        : music_path(path), volume(vol), loop(should_loop), is_playing(false) {}
};

struct audio_trigger {
    bool trigger_on_collision;
    bool trigger_on_creation;
    bool trigger_on_death;
    bool triggered;
    audio_trigger(bool on_collision = false, bool on_creation = true,
                  bool on_death = false)
        : trigger_on_collision(on_collision), trigger_on_creation(on_creation),
          trigger_on_death(on_death), triggered(false) {}
};

struct projectile_behavior {
    projectile_pattern pattern;

    projectile_behavior(
        const projectile_pattern &p = projectile_pattern::straight())
        : pattern(p) {}
};

struct input {
    bool left;
    bool right;
    bool up;
    bool down;
    bool fire;
    bool left_pressed;
    bool right_pressed;
    bool up_pressed;
    bool down_pressed;
    bool fire_pressed;

    input()
        : left(false), right(false), up(false), down(false), fire(false),
          left_pressed(false), right_pressed(false), up_pressed(false),
          down_pressed(false), fire_pressed(false) {}
};

struct ai_movement_pattern {
    std::string pattern_type; // "straight", "wave", "spiral", "sine_wave",
                              // "zigzag", "circle"

    // Pattern parameters
    float amplitude = 50.0f;   // Wave amplitude or circle radius
    float frequency = 0.01f;   // Wave frequency or rotation speed
    float phase_offset = 0.0f; // Starting phase
    float base_speed = 150.0f; // Base movement speed

    float start_x = 0.0f;
    float start_y = 0.0f;
    float pattern_time = 0.0f;

    ai_movement_pattern(const std::string &type = "straight", float amp = 50.0f,
                        float freq = 0.01f, float phase = 0.0f)
        : pattern_type(type), amplitude(amp), frequency(freq),
          phase_offset(phase) {}

    void apply_pattern(float &vx, float &vy, float pos_x, float pos_y,
                       float dt);

    static ai_movement_pattern straight(float speed = 150.0f);
    static ai_movement_pattern wave(float amplitude = 50.0f,
                                    float frequency = 0.01f,
                                    float speed = 150.0f);
    static ai_movement_pattern sine_wave(float amplitude = 80.0f,
                                         float frequency = 0.02f,
                                         float speed = 100.0f);
    static ai_movement_pattern zigzag(float amplitude = 60.0f,
                                      float frequency = 0.015f,
                                      float speed = 120.0f);
    static ai_movement_pattern circle(float radius = 40.0f,
                                      float speed = 100.0f);
};

struct ai_input {
    bool fire;
    float fire_timer;
    float fire_interval;
    ai_movement_pattern movement_pattern;

    ai_input(
        bool auto_fire = true, float interval = 1.0f,
        const ai_movement_pattern &pattern = ai_movement_pattern::straight())
        : fire(auto_fire), fire_timer(0.0f), fire_interval(interval),
          movement_pattern(pattern) {}
};

struct score {
    int current_score;
    int enemies_killed;
    float survival_time;
    float last_time_point_awarded;

    score(int initial_score = 0)
        : current_score(initial_score), enemies_killed(0), survival_time(0.0f),
          last_time_point_awarded(0.0f) {}
};

struct health {
    int current_hp;
    int max_hp;
    int pending_damage;

    health(int hp = 100) : current_hp(hp), max_hp(hp), pending_damage(0) {}
};

struct gravity {
    float acceleration;
    float max_velocity;
    float jump_strength;
    bool on_ground;

    gravity(float accel = 1500.0f, float max_vel = 1000.0f,
            float jump = 0.0f)
        : acceleration(accel), max_velocity(max_vel), jump_strength(jump),
          on_ground(false) {}
};

struct platform_tag {
    platform_tag() = default;
};

struct dead {
    float death_time;
    float death_jump_velocity;
    bool ignore_platforms;

    dead(float death_time_val = 0.0f, float jump_vel = -800.0f)
        : death_time(death_time_val), death_jump_velocity(jump_vel),
          ignore_platforms(true) {}
};

struct enemy_stunned {
    bool stunned;
    float knockback_velocity;

    enemy_stunned(bool is_stunned = false, float knockback = 0.0f)
        : stunned(is_stunned), knockback_velocity(knockback) {}
};

} // namespace component