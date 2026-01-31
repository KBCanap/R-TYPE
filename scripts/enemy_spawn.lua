function get_enemy_config()
    local weapon_types = { "spread", "burst", "wave" }
    local selected_weapon = weapon_types[math.random(#weapon_types)]

    return {
        movement = {
            type = "wave",
            amplitude = 50.0,
            frequency = 0.01,
            speed = 120.0
        },
        weapon_type = selected_weapon
    }
end
