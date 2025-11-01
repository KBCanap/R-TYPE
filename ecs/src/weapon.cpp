#include "../include/components.hpp"
#include "../include/registery.hpp"
#include <sol/sol.hpp>
#include <cmath>
#include <iostream>

namespace component {

void weapon::fire(registry &r, const position &shooter_pos, bool is_friendly) {
    if (fire_function) {
        fire_function(r, shooter_pos, is_friendly);
        return;
    }

    static sol::state lua;
    static bool initialized = false;

    if (!initialized) {
        lua.open_libraries(sol::lib::base, sol::lib::math);

        try {
            lua.script_file("scripts/weapon_fire.lua");
        } catch (const sol::error &e) {
            std::cerr << "[Lua Error] " << e.what() << std::endl;
            return;
        }

        initialized = true;
    }

    // Expose registry and parameters to Lua
    lua["projectile_count"] = projectile_count;
    lua["spread_angle"] = spread_angle;
    lua["projectile_speed"] = projectile_speed;
    lua["projectile_damage"] = projectile_damage;
    lua["projectile_lifetime"] = projectile_lifetime;
    lua["projectile_piercing"] = projectile_piercing;
    lua["projectile_max_hits"] = projectile_max_hits;
    lua["projectile_sprite_rect"] = projectile_sprite_rect;
    lua["movement_pattern"] = movement_pattern;
    lua["shooter_x"] = shooter_pos.x;
    lua["shooter_y"] = shooter_pos.y;
    lua["is_friendly"] = is_friendly;

    // Bind registry spawn and component functions
    lua["spawn_projectile"] = [&](float x, float y, float vx, float vy) {
        entity projectile_entity = r.spawn_entity();

        r.add_component(projectile_entity, position(x, y));
        r.add_component(projectile_entity, velocity(vx, vy));
        r.add_component(projectile_entity,
                        projectile(projectile_damage, projectile_speed,
                                   is_friendly, "bullet", projectile_lifetime,
                                   projectile_piercing, projectile_max_hits));
        r.add_component(projectile_entity,
                        projectile_behavior(movement_pattern));
        r.add_component(projectile_entity,
                        drawable("assets/sprites/r-typesheet1.gif",
                                 projectile_sprite_rect, 1.0f, "projectile"));
        r.add_component(projectile_entity,
                        hitbox(static_cast<float>(projectile_sprite_rect.width) * 2.0f,
                               static_cast<float>(projectile_sprite_rect.height) * 2.0f,
                               0.0f, 0.0f));
    };

    // Call Lua fire function
    sol::protected_function fire_func = lua["fire_weapon"];
    sol::protected_function_result result = fire_func();

    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[Lua Runtime Error] " << err.what() << std::endl;
    }
}

} // namespace component
