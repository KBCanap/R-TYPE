function apply_projectile_pattern()
    local vx = vx or 0.0
    local vy = vy or 0.0
    local base_direction = friendly and 1.0 or -1.0

    if pattern_type == "straight" then
        local current_speed = math.sqrt(vx * vx + vy * vy)
        if current_speed > 0 then
            local scale = speed / current_speed
            vx = vx * scale
            vy = vy * scale
        end

    elseif pattern_type == "wave" then
        vx = base_direction * speed
        vy = math.sin((pos_x + param3) * param2) * param1

    elseif pattern_type == "spiral" then
        local angle = age * param2 + param3
        local spiral_radius = param1 * (age * 0.5)
        vx = base_direction * speed + spiral_radius * math.cos(angle)
        vy = spiral_radius * math.sin(angle)

    elseif pattern_type == "bounce" then
        local screen_width = 800.0
        local screen_height = 600.0
        if pos_x <= 0 or pos_x >= screen_width then
            vx = -vx
        end
        if pos_y <= 0 or pos_y >= screen_height then
            vy = -vy
        end

    elseif pattern_type == "spread" then
        local current_speed = math.sqrt(vx * vx + vy * vy)
        if current_speed > 0 then
            local scale = speed / current_speed
            vx = vx * scale
            vy = vy * scale
        end
    end

    return { vx = vx, vy = vy }
end
