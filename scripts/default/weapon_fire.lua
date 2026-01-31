function fire_weapon()
    local base_angle = 0.0
    local angle_step = spread_angle

    if projectile_count > 1 then
        base_angle = -spread_angle * (projectile_count - 1) / 2.0
    end

    for i = 0, projectile_count - 1 do
        local angle = base_angle + (angle_step * i)
        local angle_rad = angle * math.pi / 180.0

        local vx = projectile_speed * math.cos(angle_rad)
        local vy = projectile_speed * math.sin(angle_rad)

        if not is_friendly then
            vx = -math.abs(vx)
        else
            vx = math.abs(vx)
        end

        local spawn_x = shooter_x + (is_friendly and 50.0 or -20.0)
        local spawn_y = shooter_y + 25.0

        spawn_projectile(spawn_x, spawn_y, vx, vy)
    end
end
