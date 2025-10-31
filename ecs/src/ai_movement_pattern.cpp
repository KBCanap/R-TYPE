/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** ai_movement_pattern
*/

#include "../include/components.hpp"
#include <sol/sol.hpp>
#include <cmath>
#include <iostream>

namespace component {

void ai_movement_pattern::apply_pattern(float &vx, float &vy, float pos_x,
                                        float pos_y, float dt) {

    pattern_time += dt;

    static sol::state lua;
    static bool initialized = false;

    if (!initialized) {
        lua.open_libraries(sol::lib::base, sol::lib::math);

        // Load external Lua script
        try {
            lua.script_file("scripts/ai_pattern.lua");
        } catch (const sol::error &e) {
            std::cerr << "[Lua Error] " << e.what() << std::endl;
            vx = -base_speed;
            vy = 0.0f;
            return;
        }

        initialized = true;
    }

    // Pass parameters to Lua
    lua["pattern_type"] = pattern_type;
    lua["base_speed"] = base_speed;
    lua["amplitude"] = amplitude;
    lua["frequency"] = frequency;
    lua["phase_offset"] = phase_offset;
    lua["pattern_time"] = pattern_time;
    lua["pos_x"] = pos_x;
    lua["pos_y"] = pos_y;
    lua["dt"] = dt;

    // Call Lua function
    sol::protected_function pattern_func = lua["apply_pattern"];
    sol::protected_function_result result = pattern_func();

    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[Lua Runtime Error] " << err.what() << std::endl;
        vx = -base_speed;
        vy = 0.0f;
        return;
    }

    // Extract result
    sol::table velocity = result;
    vx = velocity["vx"];
    vy = velocity["vy"];
}

ai_movement_pattern ai_movement_pattern::straight(float speed) {
    ai_movement_pattern pattern("straight");
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern ai_movement_pattern::wave(float amplitude, float frequency,
                                              float speed) {
    ai_movement_pattern pattern("wave", amplitude, frequency);
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern
ai_movement_pattern::sine_wave(float amplitude, float frequency, float speed) {
    ai_movement_pattern pattern("sine_wave", amplitude, frequency);
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern ai_movement_pattern::zigzag(float amplitude,
                                                float frequency, float speed) {
    ai_movement_pattern pattern("zigzag", amplitude, frequency);
    pattern.base_speed = speed;
    return pattern;
}

ai_movement_pattern ai_movement_pattern::circle(float radius, float speed) {
    ai_movement_pattern pattern("circle", radius, 0.02f);
    pattern.base_speed = speed;
    return pattern;
}

} // namespace component