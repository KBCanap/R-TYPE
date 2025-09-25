#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <cmath>

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
        sf::Color color;
        float size;
        sf::Texture texture;
        sf::IntRect sprite_rect;
        bool use_sprite;
        float scale;
        std::string tag; // for identifying entity type (player, enemy, projectile)

        // Constructor for colored shapes (backward compatibility)
        drawable(sf::Color color = sf::Color::White, float size = 50.0f)
            : color(color), size(size), use_sprite(false), scale(1.0f), tag("") {}

        // Constructor for sprites
        drawable(const std::string& texture_path, sf::IntRect rect = sf::IntRect(), float scale = 2.0f, const std::string& tag = "")
            : color(sf::Color::White), size(50.0f), sprite_rect(rect), use_sprite(true), scale(scale), tag(tag) {
            texture.loadFromFile(texture_path);
        }
    };
    
    struct controllable {
        float speed;
        float last_vy; // to track vertical direction
        controllable(float speed = 200.0f) : speed(speed), last_vy(0.0f) {}
    };

    struct background {
        sf::Texture texture;
        float scroll_speed;
        float offset_x;
        background(float speed = 100.0f) : scroll_speed(speed), offset_x(0.0f) {}
    };

    struct projectile {
        float damage;
        float speed;
        bool friendly; // true for player projectiles, false for enemy projectiles
        std::string projectile_type; // "bullet", "laser", "missile", etc.
        float lifetime; // how long the projectile lives in seconds
        float age; // current age of the projectile
        bool piercing; // can go through multiple enemies
        int max_hits; // maximum number of hits before disappearing (for piercing)
        int hits; // current number of hits

        projectile(float damage = 10.0f, float speed = 400.0f, bool friendly = true,
                  const std::string& type = "bullet", float lifetime = 5.0f,
                  bool piercing = false, int max_hits = 1)
            : damage(damage), speed(speed), friendly(friendly), projectile_type(type),
              lifetime(lifetime), age(0.0f), piercing(piercing), max_hits(max_hits), hits(0) {}
    };

    struct projectile_pattern {
        std::string pattern_type; // "straight", "wave", "spiral", "bounce", "custom"

        // Pattern parameters - utilisés différemment selon le type
        float param1 = 0.0f;  // wave: amplitude, spiral: amplitude, bounce: unused
        float param2 = 0.0f;  // wave: frequency, spiral: turn_speed, bounce: unused
        float param3 = 0.0f;  // wave: phase_offset, spiral: phase_offset, bounce: unused
        float param4 = 0.0f;  // unused by default patterns, available for custom

        // Fonction de calcul de vélocité personnalisée (optionnelle)
        std::function<void(float&, float&, float, float, float, float, bool)> custom_function;
        // Paramètres: vx, vy, pos_x, pos_y, age, speed, friendly

        projectile_pattern(const std::string& type = "straight",
                          float p1 = 0.0f, float p2 = 0.0f, float p3 = 0.0f, float p4 = 0.0f);

        // Méthode pour appliquer le pattern - implémentée dans projectile_pattern.cpp
        void apply_pattern(float& vx, float& vy, float pos_x, float pos_y, float age, float speed, bool friendly) const;

        // Patterns prédéfinis - implémentés dans projectile_pattern.cpp
        static projectile_pattern straight();
        static projectile_pattern wave(float amplitude = 50.0f, float frequency = 0.01f, float phase_offset = 0.0f);
        static projectile_pattern spiral(float amplitude = 30.0f, float turn_speed = 5.0f, float phase_offset = 0.0f);
        static projectile_pattern bounce();
        static projectile_pattern spread(float spread_angle = 30.0f);
    };

    struct weapon {
        float fire_rate; // projectiles per second
        float last_shot_time;
        bool friendly; // determines if this weapon shoots friendly projectiles

        // Projectile configuration
        int projectile_count; // number of projectiles per shot
        float spread_angle; // angle between projectiles (for multi-shot)
        projectile_pattern movement_pattern; // pattern de mouvement configurable

        // Projectile stats
        float projectile_damage;
        float projectile_speed;
        float projectile_lifetime;
        bool projectile_piercing;
        int projectile_max_hits;

        // Burst properties
        int burst_count; // projectiles in burst
        int current_burst; // current projectile in burst
        float burst_interval; // time between projectiles in burst
        float last_burst_time;
        bool is_burst; // whether this weapon fires in bursts

        // Sprite configuration
        sf::IntRect projectile_sprite_rect;

        weapon(float rate = 2.0f, bool friendly = true, int proj_count = 1, float spread = 0.0f,
               const projectile_pattern& pattern = projectile_pattern::straight(), float damage = 20.0f,
               float speed = 500.0f, float lifetime = 5.0f, bool piercing = false, int max_hits = 1,
               bool burst = false, int burst_cnt = 3, float burst_int = 0.1f,
               sf::IntRect sprite_rect = sf::IntRect(60, 353, 12, 12))
            : fire_rate(rate), last_shot_time(0.0f), friendly(friendly),
              projectile_count(proj_count), spread_angle(spread), movement_pattern(pattern),
              projectile_damage(damage), projectile_speed(speed), projectile_lifetime(lifetime),
              projectile_piercing(piercing), projectile_max_hits(max_hits),
              burst_count(burst_cnt), current_burst(0), burst_interval(burst_int),
              last_burst_time(0.0f), is_burst(burst), projectile_sprite_rect(sprite_rect) {}
    };

    struct animation {
        std::vector<sf::IntRect> frames;
        float frame_duration; // seconds per frame
        float current_time;
        size_t current_frame;
        bool loop;
        bool playing;
        bool reverse; // for playing animation backwards
        bool destroy_on_finish; // destroy entity when animation finishes

        animation(float duration = 0.1f, bool loop = true, bool destroy = false)
            : frame_duration(duration), current_time(0.0f), current_frame(0), loop(loop), playing(true), reverse(false), destroy_on_finish(destroy) {}
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
        sound_effect(const std::string& path, float vol = 100.0f, bool once = true)
            : sound_path(path), volume(vol), play_once(once), is_playing(false) {}
    };

    struct music {
        std::string music_path;
        float volume;
        bool loop;
        bool is_playing;
        music(const std::string& path, float vol = 50.0f, bool should_loop = true)
            : music_path(path), volume(vol), loop(should_loop), is_playing(false) {}
    };

    struct audio_trigger {
        bool trigger_on_collision;
        bool trigger_on_creation;
        bool trigger_on_death;
        bool triggered;
        audio_trigger(bool on_collision = false, bool on_creation = true, bool on_death = false)
            : trigger_on_collision(on_collision), trigger_on_creation(on_creation),
              trigger_on_death(on_death), triggered(false) {}
    };

    struct projectile_behavior {
        projectile_pattern pattern;

        projectile_behavior(const projectile_pattern& p = projectile_pattern::straight())
            : pattern(p) {}
    };

    struct input {
        bool left;
        bool right;
        bool up;
        bool down;
        bool fire;
        bool left_pressed;    // Pour détecter le moment où la touche est pressée
        bool right_pressed;
        bool up_pressed;
        bool down_pressed;
        bool fire_pressed;

        input() : left(false), right(false), up(false), down(false), fire(false),
                  left_pressed(false), right_pressed(false), up_pressed(false),
                  down_pressed(false), fire_pressed(false) {}
    };

    struct ai_movement_pattern {
        std::string pattern_type; // "straight", "wave", "spiral", "sine_wave", "zigzag", "circle"

        // Pattern parameters
        float amplitude = 50.0f;    // Wave amplitude or circle radius
        float frequency = 0.01f;    // Wave frequency or rotation speed
        float phase_offset = 0.0f;  // Starting phase
        float base_speed = 150.0f;  // Base movement speed

        // Position tracking for patterns that need it
        float start_x = 0.0f;
        float start_y = 0.0f;
        float pattern_time = 0.0f;  // Time since pattern started

        ai_movement_pattern(const std::string& type = "straight",
                           float amp = 50.0f, float freq = 0.01f, float phase = 0.0f)
            : pattern_type(type), amplitude(amp), frequency(freq), phase_offset(phase) {}

        // Method to apply movement pattern
        void apply_pattern(float& vx, float& vy, float pos_x, float pos_y, float dt);

        // Predefined patterns
        static ai_movement_pattern straight(float speed = 150.0f);
        static ai_movement_pattern wave(float amplitude = 50.0f, float frequency = 0.01f, float speed = 150.0f);
        static ai_movement_pattern sine_wave(float amplitude = 80.0f, float frequency = 0.02f, float speed = 100.0f);
        static ai_movement_pattern zigzag(float amplitude = 60.0f, float frequency = 0.015f, float speed = 120.0f);
        static ai_movement_pattern circle(float radius = 40.0f, float speed = 100.0f);
    };

    struct ai_input {
        bool fire;
        float fire_timer;
        float fire_interval;
        ai_movement_pattern movement_pattern;

        ai_input(bool auto_fire = true, float interval = 1.0f,
                 const ai_movement_pattern& pattern = ai_movement_pattern::straight())
            : fire(auto_fire), fire_timer(0.0f), fire_interval(interval), movement_pattern(pattern) {}
    };

    struct score {
        int current_score;
        int enemies_killed;
        float survival_time;
        float last_time_point_awarded;

        score(int initial_score = 0)
            : current_score(initial_score), enemies_killed(0), survival_time(0.0f), last_time_point_awarded(0.0f) {}
    };

    struct health {
        int current_hp;
        int max_hp;
        int pending_damage;  // Damage to be applied this frame

        health(int hp = 100)
            : current_hp(hp), max_hp(hp), pending_damage(0) {}
    };

}