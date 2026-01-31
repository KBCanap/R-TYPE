# R-TYPE Development Guide

This document provides a comprehensive overview of CMake presets and the ECS (Entity Component System) development guidelines for the R-Type project.

## Prerequisites

- **CMake**: Version 3.20 or higher
- **C++ Standard**: C++17
- **Windows**: Visual Studio 2022 (MSVC) with x64 architecture
- **Linux**: g++ compiler with Unix Makefiles support

---

## CMake Configuration Presets

### Linux Configurations

#### `linux-config-dev` - Linux Development (Debug)
- **Platform**: Linux only
- **Generator**: Unix Makefiles
- **Build Directory**: `build/dev/`
- **Build Type**: Debug
- **Features**:
  - Debug symbols enabled
  - Compile commands export (`compile_commands.json`)
  - Unit tests **ENABLED** (`RTYPE_BUILD_TESTS=ON`)
  - Warnings as errors **DISABLED** (`RTYPE_WERROR=OFF`)
  - Runtime output: project root
  - Library output: `lib/`
  - CPM cache: `.cache/CPM/`

**Usage:**
```bash
cmake --preset linux-config-dev
cmake --build --preset linux-build-dev
```

#### `linux-config-release` - Linux Release (Production)
- **Platform**: Linux only
- **Generator**: Unix Makefiles
- **Build Directory**: `build/release/`
- **Build Type**: Release
- **Features**:
  - Full optimizations enabled (`-O3 -DNDEBUG`)
  - Compile commands export
  - Unit tests **DISABLED** (`RTYPE_BUILD_TESTS=OFF`)
  - Warnings as errors **DISABLED** (`RTYPE_WERROR=OFF`)
  - Runtime output: project root
  - Library output: `lib/`
  - CPM cache: `.cache/CPM/`

**Usage:**
```bash
cmake --preset linux-config-release
cmake --build --preset linux-build-release
```

### Windows Configurations

#### `windows-config-dev` - Windows Development (Debug)
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Build Directory**: `build/windows-dev/`
- **Build Type**: Debug
- **Features**:
  - Debug symbols and assertions
  - Runtime output: project root
  - CPM cache: `.cache/CPM/`
  - Tests not explicitly enabled (defaults to OFF)

**Usage:**
```bash
cmake --preset windows-config-dev
cmake --build --preset windows-build-dev
```

#### `windows-config-release` - Windows Release (Production)
- **Platform**: Windows only
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Build Directory**: `build/windows-release/`
- **Build Type**: Release
- **Features**:
  - Full optimizations
  - Compile commands export
  - Unit tests **DISABLED** (`RTYPE_BUILD_TESTS=OFF`)
  - Warnings as errors **DISABLED** (`RTYPE_WERROR=OFF`)
  - Runtime output: project root
  - Library output: `lib/`
  - CPM cache: `.cache/CPM/`

**Usage:**
```bash
cmake --preset windows-config-release
cmake --build --preset windows-build-release
```

### Build Configuration Summary

| Preset | Platform | Type | Tests | Jobs | Output |
|--------|----------|------|-------|------|--------|
| `linux-config-dev` | Linux | Debug | ✅ Yes | 8 | Root |
| `linux-config-release` | Linux | Release | ❌ No | 8 | Root |
| `windows-config-dev` | Windows | Debug | ❌ No | 8 | Root |
| `windows-config-release` | Windows | Release | ❌ No | 8 | Root |

### Running Tests

Tests are only available with `linux-config-dev`:

```bash
# Configure and build with tests
cmake --preset linux-config-dev
cmake --build --preset linux-build-dev

# Run all tests
cd build/dev && ctest

# Run specific tests
ctest --test-dir build/dev -R "test_pattern"
```

---

## ECS (Entity Component System) Architecture

The R-Type project uses a custom ECS architecture for managing game entities and logic. This section provides a complete guide for working with the ECS.

### Core Concepts

#### Entity
An **entity** is simply a unique identifier (ID) representing a game object. Entities have no data or behavior themselves—they are just IDs that bind components together.

```cpp
entity player = registry.spawn_entity();
entity enemy = registry.spawn_entity();
```

#### Component
**Components** are pure data structures (POD - Plain Old Data) that store specific attributes. Components have no logic.

Examples:
- `position`: stores x, y coordinates
- `velocity`: stores movement speed (vx, vy)
- `drawable`: stores visual representation
- `health`: stores hit points

#### System
**Systems** contain all the game logic and operate on entities that have specific component combinations. Systems are functions registered to the registry.

```cpp
void movement_system(registry &r, 
                     sparse_array<position> &positions,
                     sparse_array<velocity> &velocities, 
                     float dt) {
    for (size_t i = 0; i < positions.size(); ++i) {
        auto &pos = positions[i];
        auto &vel = velocities[i];
        
        if (pos && vel) {
            pos->x += vel->vx * dt;
            pos->y += vel->vy * dt;
        }
    }
}
```

---

## Registry API

The **registry** is the central hub for managing entities, components, and systems.

### Entity Management

#### Spawn a new entity
```cpp
entity player = registry.spawn_entity();
```

#### Get entity from index
```cpp
entity e = registry.entity_from_index(5);
```

#### Kill an entity (removes all its components)
```cpp
registry.kill_entity(player);
```

### Component Management

#### Register a component type
```cpp
registry.register_component<position>();
registry.register_component<velocity>();
```

#### Add a component to an entity
```cpp
// Method 1: Move semantics
registry.add_component(player, position(100.0f, 200.0f));

// Method 2: Emplace (constructs in-place)
registry.emplace_component<velocity>(player, 50.0f, 0.0f);
```

#### Get all components of a type
```cpp
auto &positions = registry.get_components<position>();
auto &velocities = registry.get_components<velocity>();
```

#### Remove a component
```cpp
registry.remove_component<controllable>(player);
```

#### Access component data
```cpp
auto &positions = registry.get_components<position>();

// Option 1: Direct array access
if (positions[entity_id].has_value()) {
    float x = positions[entity_id]->x;
}

// Option 2: Iteration
for (size_t i = 0; i < positions.size(); ++i) {
    if (positions[i].has_value()) {
        positions[i]->x += 10.0f;
    }
}
```

### System Management

#### Register a system
Systems are registered using `add_system` with template parameters specifying required components:

```cpp
// System that operates on position and velocity
registry.add_system<position, velocity>(
    [](registry &r, 
       sparse_array<position> &positions,
       sparse_array<velocity> &velocities,
       float dt) {
        // System logic here
    }
);
```

#### Run all systems
```cpp
float delta_time = 0.016f; // ~60 FPS
registry.run_systems(delta_time);
```

---

## Core Components Reference

### Transform Components

#### `position`
Stores 2D position coordinates.

```cpp
struct position {
    float x, y;
    position(float x = 0.0f, float y = 0.0f);
};

// Usage
registry.add_component(entity, position(100.0f, 200.0f));
```

#### `velocity`
Stores 2D velocity vector.

```cpp
struct velocity {
    float vx, vy;
    velocity(float vx = 0.0f, float vy = 0.0f);
};

// Usage
registry.add_component(entity, velocity(150.0f, 0.0f));
```

### Visual Components

#### `drawable`
Manages entity rendering with sprites or shapes.

```cpp
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
};

// Usage - Simple colored shape
registry.add_component(entity, drawable(render::Color::Red(), 50.0f));

// Usage - Sprite
registry.add_component(entity, 
    drawable("assets/sprites/player.png",    // texture path
             render::IntRect(0, 0, 32, 32),  // sprite rect
             2.0f,                            // scale
             "player"));                      // tag
```

#### `animation`
Manages sprite animation frames.

```cpp
struct animation {
    std::vector<render::IntRect> frames;
    float frame_duration;
    float current_time;
    size_t current_frame;
    bool loop;
    bool playing;
    bool reverse;
    bool destroy_on_finish;
};

// Usage
animation anim(0.1f, true);  // 0.1s per frame, looping
anim.frames.push_back(render::IntRect(0, 0, 32, 32));
anim.frames.push_back(render::IntRect(32, 0, 32, 32));
anim.frames.push_back(render::IntRect(64, 0, 32, 32));
registry.add_component(entity, anim);
```

### Input Components

#### `input`
Stores player input state.

```cpp
struct input {
    bool left, right, up, down, fire;
    bool left_pressed, right_pressed, up_pressed, down_pressed, fire_pressed;
};

// Usage
input player_input;
player_input.fire = true;  // Player is holding fire button
player_input.fire_pressed = true;  // Fire button was just pressed this frame
registry.add_component(player, player_input);
```

#### `controllable`
Marks entity as player-controllable.

```cpp
struct controllable {
    float speed;
    float last_vy;
    controllable(float speed = 200.0f);
};

// Usage
registry.add_component(player, controllable(250.0f));
```

### Combat Components

#### `health`
Manages entity health points.

```cpp
struct health {
    int current_hp;
    int max_hp;
    int pending_damage;
    health(int hp = 100);
};

// Usage
registry.add_component(entity, health(100));  // 100 HP

// Apply damage
auto &healths = registry.get_components<health>();
if (healths[entity].has_value()) {
    healths[entity]->pending_damage += 25;  // Take 25 damage
}
```

#### `hitbox`
Defines collision boundaries.

```cpp
struct hitbox {
    float width, height;
    float offset_x, offset_y;
    hitbox(float w = 50.0f, float h = 50.0f, float ox = 0.0f, float oy = 0.0f);
};

// Usage
registry.add_component(entity, hitbox(32.0f, 32.0f, 0.0f, 0.0f));
```

#### `projectile`
Marks entity as a projectile with damage properties.

```cpp
struct projectile {
    float damage;
    float speed;
    bool friendly;              // true = player projectile, false = enemy
    std::string projectile_type;
    float lifetime;
    float age;
    bool piercing;
    int max_hits;
    int hits;
};

// Usage
registry.add_component(entity, 
    projectile(20.0f,      // damage
               500.0f,     // speed
               true,       // friendly
               "bullet",   // type
               5.0f,       // lifetime in seconds
               false,      // piercing
               1));        // max hits
```

#### `projectile_behavior`
Stores the movement pattern for a projectile. This is applied by the projectile system to modify projectile movement over time.

```cpp
struct projectile_behavior {
    projectile_pattern pattern;
    projectile_behavior(const projectile_pattern &p = projectile_pattern::straight());
};

// Usage
registry.add_component(entity, 
    projectile_behavior(projectile_pattern::wave(50.0f, 0.01f)));
```

**Note**: This component is automatically added by the `weapon::fire()` function based on the weapon's `movement_pattern`.

#### `weapon`
Manages weapon firing behavior.

```cpp
struct weapon {
    float fire_rate;
    float last_shot_time;
    bool friendly;
    
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
    int burst_count;
    int current_burst;
    float burst_interval;
    float last_burst_time;
    bool is_burst;
    
    render::IntRect projectile_sprite_rect;
    std::function<void(registry &, const position &, bool)> fire_function;
};

// Usage - Standard weapon
registry.add_component(player, weapon(
    2.0f,                                      // fire_rate (shots per second)
    true,                                      // friendly
    1,                                         // projectile_count
    0.0f,                                      // spread_angle
    projectile_pattern::straight(),            // movement_pattern
    20.0f,                                     // damage
    500.0f,                                    // speed
    5.0f,                                      // lifetime
    false,                                     // piercing
    1                                          // max_hits
));

// Usage - Spread shot weapon
registry.add_component(player, weapon(
    1.5f,                                      // fire_rate
    true,                                      // friendly
    3,                                         // projectile_count (3-way shot)
    15.0f,                                     // spread_angle (degrees)
    projectile_pattern::straight(),
    15.0f,                                     // damage
    450.0f,                                    // speed
    5.0f,
    false,
    1
));
```

### AI Components

#### `ai_input`
Manages AI behavior and auto-firing.

```cpp
struct ai_input {
    bool fire;
    float fire_timer;
    float fire_interval;
    ai_movement_pattern movement_pattern;
};

// Usage
registry.add_component(enemy, 
    ai_input(true,                                  // auto-fire enabled
             1.0f,                                  // fire interval (seconds)
             ai_movement_pattern::wave(50.0f,       // amplitude
                                       0.01f)));    // frequency
```

#### `ai_movement_pattern`
Defines AI movement behavior patterns.

```cpp
struct ai_movement_pattern {
    std::string pattern_type;
    float amplitude;
    float frequency;
    float phase_offset;
    float base_speed;
    float start_x, start_y;
    float pattern_time;
    
    void apply_pattern(float &vx, float &vy, float pos_x, float pos_y, float dt);
    
    // Factory methods
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

// Usage
registry.add_component(enemy, 
    ai_input(true, 2.0f, ai_movement_pattern::sine_wave(80.0f, 0.02f, 100.0f)));
```

### Game State Components

#### `score`
Tracks player score statistics.

```cpp
struct score {
    int current_score;
    int enemies_killed;
    float survival_time;
    float last_time_point_awarded;
};

// Usage
registry.add_component(player, score(0));
```

#### `background`
Manages scrolling background.

```cpp
struct background {
    std::shared_ptr<render::ITexture> texture;
    float scroll_speed;
    float offset_x;
};

// Usage
registry.add_component(entity, background(100.0f));  // 100 pixels per second
```

### Audio Components

#### `sound_effect`
Plays one-shot sound effects.

```cpp
struct sound_effect {
    std::string sound_path;
    float volume;
    bool play_once;
    bool is_playing;
};

// Usage
registry.add_component(entity, 
    sound_effect("assets/sounds/explosion.wav", 80.0f, true));
```

#### `music`
Plays looping background music.

```cpp
struct music {
    std::string music_path;
    float volume;
    bool loop;
    bool is_playing;
};

// Usage
registry.add_component(entity, 
    music("assets/music/level1.ogg", 50.0f, true));
```

#### `audio_trigger`
Controls when audio should play.

```cpp
struct audio_trigger {
    bool trigger_on_collision;
    bool trigger_on_creation;
    bool trigger_on_death;
    bool triggered;
};

// Usage
audio_trigger trigger(true, false, false);  // Play on collision
registry.add_component(entity, trigger);
```

---

## Projectile Patterns

Projectile patterns control how projectiles move after being fired.

### Available Patterns

#### `straight()`
Projectiles move in a straight line.

```cpp
projectile_pattern pattern = projectile_pattern::straight();
```

#### `wave(amplitude, frequency, phase_offset)`
Projectiles follow a sine wave pattern.

```cpp
projectile_pattern pattern = projectile_pattern::wave(
    50.0f,    // amplitude - wave height
    0.01f,    // frequency - wave speed
    0.0f      // phase_offset - starting phase
);
```

#### `spiral(amplitude, turn_speed, phase_offset)`
Projectiles spiral outward.

```cpp
projectile_pattern pattern = projectile_pattern::spiral(
    30.0f,    // amplitude - spiral size
    5.0f,     // turn_speed - rotation speed
    0.0f      // phase_offset
);
```

#### `bounce()`
Projectiles bounce off screen edges.

```cpp
projectile_pattern pattern = projectile_pattern::bounce();
```

#### `spread(spread_angle)`
Projectiles spread out at an angle.

```cpp
projectile_pattern pattern = projectile_pattern::spread(30.0f);
```

### Custom Patterns

You can create custom projectile behaviors:

```cpp
projectile_pattern custom_pattern("custom");
custom_pattern.custom_function = 
    [](float &vx, float &vy, float pos_x, float pos_y, 
       float age, float speed, bool friendly) {
        // Custom movement logic
        vx = speed * std::cos(age * 2.0f);
        vy = speed * std::sin(age * 2.0f);
    };
```

---

## AI Movement Patterns

AI movement patterns control enemy movement behavior.

### Available Patterns

#### `straight(speed)`
Move straight left at constant speed.

```cpp
ai_movement_pattern pattern = ai_movement_pattern::straight(150.0f);
```

#### `wave(amplitude, frequency, speed)`
Move in a horizontal sine wave.

```cpp
ai_movement_pattern pattern = ai_movement_pattern::wave(
    50.0f,     // amplitude - wave height
    0.01f,     // frequency - wave speed
    150.0f     // base speed
);
```

#### `sine_wave(amplitude, frequency, speed)`
Move in a time-based sine wave.

```cpp
ai_movement_pattern pattern = ai_movement_pattern::sine_wave(
    80.0f,     // amplitude
    0.02f,     // frequency
    100.0f     // speed
);
```

#### `zigzag(amplitude, frequency, speed)`
Move in a zigzag triangle wave pattern.

```cpp
ai_movement_pattern pattern = ai_movement_pattern::zigzag(
    60.0f,     // amplitude
    0.015f,    // frequency
    120.0f     // speed
);
```

#### `circle(radius, speed)`
Move in a circular path.

```cpp
ai_movement_pattern pattern = ai_movement_pattern::circle(
    40.0f,     // radius
    100.0f     // speed
);
```

---

## Built-in Systems

The ECS includes several built-in systems for common game functionality.

### `position_system`
Updates entity positions based on velocity.

```cpp
void position_system(registry &r, 
                     sparse_array<position> &positions,
                     sparse_array<velocity> &velocities,
                     sparse_array<input> &inputs,
                     render::IRenderWindow &window,
                     float current_time,
                     float dt);
```

### `control_system`
Handles player input and movement.

```cpp
void control_system(registry &r,
                    sparse_array<controllable> &controllables,
                    sparse_array<velocity> &velocities,
                    sparse_array<input> &inputs,
                    float dt);
```

### `render_system`
Renders all drawable entities.

```cpp
void render_system(registry &r,
                   sparse_array<position> &positions,
                   sparse_array<drawable> &drawables,
                   render::IRenderWindow &window,
                   float dt);
```

### `weapon_system`
Handles weapon firing logic.

```cpp
void weapon_system(registry &r,
                   sparse_array<weapon> &weapons,
                   sparse_array<position> &positions,
                   sparse_array<input> &inputs,
                   sparse_array<ai_input> &ai_inputs,
                   float current_time);
```

### `projectile_system`
Updates projectile behavior and lifetime.

```cpp
void projectile_system(registry &r,
                       sparse_array<projectile> &projectiles,
                       sparse_array<position> &positions,
                       render::IRenderWindow &window,
                       float dt);
```

### `collision_system`
Detects and resolves collisions.

```cpp
void collision_system(registry &r,
                      sparse_array<position> &positions,
                      sparse_array<drawable> &drawables,
                      sparse_array<projectile> &projectiles,
                      sparse_array<hitbox> &hitboxes);
```

### `ai_input_system`
Updates AI behavior and firing.

```cpp
void ai_input_system(registry &r,
                     sparse_array<ai_input> &ai_inputs,
                     float dt);
```

### `health_system`
Processes damage and entity death.

```cpp
void health_system(registry &r,
                   sparse_array<health> &healths,
                   float dt);
```

### `score_system`
Updates player score based on kills and survival time.

```cpp
void score_system(registry &r,
                  sparse_array<score> &scores,
                  float dt);
```

### `audio_system`
Manages sound effects and music playback.

```cpp
void audio_system(registry &r,
                  sparse_array<sound_effect> &sound_effects,
                  sparse_array<music> &musics,
                  sparse_array<audio_trigger> &triggers,
                  render::IRenderAudio &audioManager);
```

---

## Complete Entity Creation Examples

### Player Entity

```cpp
entity player = registry.spawn_entity();

// Transform
registry.add_component(player, position(100.0f, 300.0f));
registry.add_component(player, velocity(0.0f, 0.0f));

// Visual
registry.add_component(player, 
    drawable("assets/sprites/player.png",
             render::IntRect(0, 0, 33, 17),  // sprite rect
             2.0f,                            // scale
             "player"));                      // tag

// Gameplay
registry.add_component(player, controllable(200.0f));        // movement speed
registry.add_component(player, input());                     // input handler
registry.add_component(player, health(100));                 // 100 HP
registry.add_component(player, hitbox(66.0f, 34.0f));       // collision box
registry.add_component(player, score(0));                    // score tracker

// Weapon
registry.add_component(player, weapon(
    2.0f,                                   // fire rate
    true,                                   // friendly
    1,                                      // projectile count
    0.0f,                                   // spread angle
    projectile_pattern::straight(),         // pattern
    20.0f,                                  // damage
    500.0f,                                 // speed
    5.0f,                                   // lifetime
    false,                                  // piercing
    1                                       // max hits
));
```

### Enemy Entity with Wave Movement

```cpp
entity enemy = registry.spawn_entity();

// Transform
registry.add_component(enemy, position(800.0f, 300.0f));
registry.add_component(enemy, velocity(-150.0f, 0.0f));

// Visual
registry.add_component(enemy,
    drawable("assets/sprites/enemy.png",
             render::IntRect(0, 0, 32, 32),
             2.0f,
             "enemy"));

// Gameplay
registry.add_component(enemy, health(50));
registry.add_component(enemy, hitbox(64.0f, 64.0f));

// AI
registry.add_component(enemy,
    ai_input(true,                              // auto-fire enabled
             2.0f,                              // fire every 2 seconds
             ai_movement_pattern::wave(         // wave movement
                 50.0f,                         // amplitude
                 0.01f,                         // frequency
                 150.0f)));                     // speed

// Enemy weapon (shoots left)
registry.add_component(enemy, weapon(
    1.5f,                                   // fire rate
    false,                                  // NOT friendly (enemy projectile)
    1,
    0.0f,
    projectile_pattern::straight(),
    15.0f,                                  // damage
    400.0f,                                 // speed
    5.0f,
    false,
    1
));

// Audio on death
registry.add_component(enemy,
    sound_effect("assets/sounds/enemy_death.wav", 70.0f, true));
registry.add_component(enemy,
    audio_trigger(false, false, true));     // trigger on death
```

### Boss Entity with Circular Movement

```cpp
entity boss = registry.spawn_entity();

// Transform
registry.add_component(boss, position(700.0f, 300.0f));
registry.add_component(boss, velocity(0.0f, 0.0f));

// Visual
registry.add_component(boss,
    drawable("assets/sprites/boss.png",
             render::IntRect(0, 0, 128, 128),
             1.5f,
             "boss"));

// Gameplay
registry.add_component(boss, health(500));           // High HP
registry.add_component(boss, hitbox(192.0f, 192.0f));

// AI with circle movement
registry.add_component(boss,
    ai_input(true, 0.5f,                            // fire every 0.5s
             ai_movement_pattern::circle(
                 100.0f,                            // radius
                 80.0f)));                          // speed

// Multi-shot spread weapon
registry.add_component(boss, weapon(
    0.5f,                                           // fast fire rate
    false,                                          // enemy weapon
    5,                                              // 5 projectiles
    30.0f,                                          // wide spread
    projectile_pattern::wave(40.0f, 0.02f),        // wavy projectiles
    25.0f,                                          // high damage
    350.0f,
    5.0f,
    false,
    1
));
```

### Power-up Entity

```cpp
entity powerup = registry.spawn_entity();

// Transform
registry.add_component(powerup, position(600.0f, 250.0f));
registry.add_component(powerup, velocity(-100.0f, 0.0f));  // scroll left slowly

// Visual
registry.add_component(powerup,
    drawable("assets/sprites/powerup.png",
             render::IntRect(0, 0, 16, 16),
             2.5f,
             "powerup"));

// Collision detection
registry.add_component(powerup, hitbox(40.0f, 40.0f));

// Audio on pickup
registry.add_component(powerup,
    sound_effect("assets/sounds/powerup.wav", 80.0f, true));
registry.add_component(powerup,
    audio_trigger(true, false, false));     // trigger on collision
```

---

## Custom System Example

Here's how to create a custom system:

```cpp
// Custom system that makes entities blink
void blink_system(registry &r,
                  sparse_array<drawable> &drawables,
                  float dt) {
    static float blink_timer = 0.0f;
    blink_timer += dt;
    
    bool visible = static_cast<int>(blink_timer * 5.0f) % 2 == 0;
    
    for (size_t i = 0; i < drawables.size(); ++i) {
        auto &draw = drawables[i];
        if (draw.has_value() && draw->tag == "damaged") {
            // Toggle visibility by modifying alpha
            draw->color.a = visible ? 255 : 100;
        }
    }
}

// Register the system
registry.add_system<drawable>([](registry &r, 
                                  sparse_array<drawable> &drawables, 
                                  float dt) {
    blink_system(r, drawables, dt);
});
```

---

## Best Practices

### 1. Component Design
- Keep components simple and data-only
- No methods except constructors
- No inheritance between components
- Use `std::optional` for optional fields instead of pointers

### 2. System Design
- Each system should have a single responsibility
- Systems should be independent of each other
- Iterate through sparse arrays checking for `has_value()`
- Use entity ID as array index for O(1) component access

### 3. Entity Creation
- Always add required components immediately after spawning
- Use consistent component initialization
- Consider using factory functions for complex entities

### 4. Performance Tips
- Avoid creating/destroying entities mid-frame
- Batch entity creation when possible
- Reuse entities by resetting components instead of destroying
- Profile systems to identify bottlenecks

### 5. Memory Management
- Components are stored in `sparse_array` (vector of optional)
- Destroyed entities leave gaps (empty optionals)
- Consider periodic defragmentation for long-running games

---

## Troubleshooting

### Entity not rendering
- Check that entity has both `position` and `drawable` components
- Verify `drawable.use_sprite` is set correctly
- Check sprite texture path is valid
- Ensure entity position is within screen bounds

### Collision not working
- Verify both entities have `hitbox` components
- Check `hitbox` dimensions match sprite size
- Ensure `position` components exist
- Debug collision bounds visually

### Weapon not firing
- Check `weapon.fire_rate` and `weapon.last_shot_time`
- Verify entity has `position` component
- For players: ensure `input.fire` or `input.fire_pressed` is true
- For AI: ensure `ai_input.fire` is true

### AI not moving
- Verify entity has `ai_input` component
- Check `ai_movement_pattern` is properly configured
- Ensure entity has `velocity` component
- Verify `ai_input_system` is registered and running

---

## Directory Structure

```
project-root/
├── ecs/
│   ├── include/
│   │   ├── entity.hpp
│   │   ├── components.hpp
│   │   ├── registery.hpp
│   │   ├── sparse_array.hpp
│   │   ├── systems.hpp
│   │   ├── weapon_presets.hpp
│   │   ├── asset_helper.hpp
│   │   ├── render/
│   │   │   ├── IRenderWindow.hpp
│   │   │   └── IRenderAudio.hpp
│   │   └── network/
│   │       ├── INetwork.hpp
│   │       ├── ISocket.hpp
│   │       ├── ANetworkManager.hpp
│   │       ├── NetworkSystem.hpp
│   │       ├── NetworkCommands.hpp
│   │       └── NetworkComponents.hpp
│   └── src/
│       ├── registery.cpp
│       ├── weapon.cpp
│       ├── projectile_pattern.cpp
│       ├── ai_movement_pattern.cpp
│       ├── systems.cpp
│       ├── ANetworkManager.cpp
│       └── NetworkSystem.cpp
├── app/                        # Client application
├── Server/                     # Server source
├── tests/                      # Unit tests
└── assets/
    ├── sprites/
    ├── sounds/
    └── music/
```

---

**For detailed API documentation**: Generate Doxygen documentation from source code comments.