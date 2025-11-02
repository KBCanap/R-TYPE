#include "../include/components.hpp"
#include <cmath>
#include <iostream>
#include <sol/sol.hpp>

namespace component {

projectile_pattern::projectile_pattern(const std::string &type, float p1,
                                       float p2, float p3, float p4)
    : pattern_type(type), param1(p1), param2(p2), param3(p3), param4(p4) {}

void projectile_pattern::apply_pattern(float &vx, float &vy, float pos_x,
                                       float pos_y, float age, float speed,
                                       bool friendly) const {
    static sol::state lua;
    static bool initialized = false;

    if (!initialized) {
        lua.open_libraries(sol::lib::base, sol::lib::math);

        try {
            lua.script_file("scripts/projectile_pattern.lua");
        } catch (const sol::error &e) {
            std::cerr << "[Lua Error] " << e.what() << std::endl;
            return;
        }

        initialized = true;
    }

    // Pass parameters to Lua
    lua["pattern_type"] = pattern_type;
    lua["param1"] = param1;
    lua["param2"] = param2;
    lua["param3"] = param3;
    lua["param4"] = param4;
    lua["pos_x"] = pos_x;
    lua["pos_y"] = pos_y;
    lua["age"] = age;
    lua["speed"] = speed;
    lua["friendly"] = friendly;
    lua["vx"] = vx;
    lua["vy"] = vy;

    // Call Lua function
    sol::protected_function pattern_func = lua["apply_projectile_pattern"];
    sol::protected_function_result result = pattern_func();

    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[Lua Runtime Error] " << err.what() << std::endl;
        return;
    }

    sol::table velocity = result;
    vx = velocity["vx"];
    vy = velocity["vy"];
}

projectile_pattern projectile_pattern::straight() {
    return projectile_pattern("straight");
}

projectile_pattern projectile_pattern::wave(float amplitude, float frequency,
                                            float phase_offset) {
    return projectile_pattern("wave", amplitude, frequency, phase_offset);
}

projectile_pattern projectile_pattern::spiral(float amplitude, float turn_speed,
                                              float phase_offset) {
    return projectile_pattern("spiral", amplitude, turn_speed, phase_offset);
}

projectile_pattern projectile_pattern::bounce() {
    return projectile_pattern("bounce");
}

projectile_pattern projectile_pattern::spread(float spread_angle) {
    return projectile_pattern("spread", spread_angle);
}

} // namespace component
