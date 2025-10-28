function apply_pattern()
    local vx = -base_speed
    local vy = 0.0

    if pattern_type == "straight" then
        vy = 0.1

    elseif pattern_type == "wave" then
        vy = amplitude * math.sin(frequency * pos_x + phase_offset)

    elseif pattern_type == "sine_wave" then
        vy = amplitude * math.sin(frequency * pattern_time + phase_offset)

    elseif pattern_type == "zigzag" then
        local triangle_wave = math.abs((frequency * pattern_time + phase_offset) % 2.0 - 1.0) * 2.0 - 1.0
        vy = amplitude * triangle_wave

    elseif pattern_type == "circle" then
        if not start_x then start_x = pos_x end
        if not start_y then start_y = pos_y end

        local angle = frequency * pattern_time + phase_offset
        local target_x = start_x + amplitude * math.cos(angle) - base_speed * pattern_time
        local target_y = start_y + amplitude * math.sin(angle)

        vx = (target_x - pos_x) / dt
        vy = (target_y - pos_y) / dt
    end

    return { vx = vx, vy = vy }
end
