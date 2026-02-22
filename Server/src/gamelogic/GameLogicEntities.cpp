#include "gamelogic/GameLogic.hpp"
#include <cmath>

void GameLogic::spawnEnemy() {
    if (_level_id == 1) {
        spawnEnemyLevel1();
    } else if (_level_id == 2 || _endless_mode) {
        spawnEnemyLevel2();
    } else {
        spawnEnemyLevel1();
    }
}

void GameLogic::spawnEnemyLevel1() {
    entity enemy = _registry->spawn_entity();

    std::uniform_int_distribution<int> type_dist(0, 3);
    std::uniform_real_distribution<float> y_dist(0.1f, 0.9f);

    int enemy_type = type_dist(_rng);
    float spawn_y = y_dist(_rng);
    uint net_id = generateNetId();

    float vx = -0.15f;
    if (enemy_type == 3) {
        vx = -0.125f;
    }

    _registry->add_component(enemy, Position{0.95f, spawn_y});
    _registry->add_component(enemy, Velocity{vx, 0.0f});
    _registry->add_component(enemy, Enemy{enemy_type, 0.0f, 5, 0.0f, 2.0f});
    _registry->add_component(enemy, Health{25, 25, 0.0f});
    _registry->add_component(enemy, Hitbox{50.0f, 58.0f, 0.0f, 0.0f});
    _registry->add_component(enemy, NetworkComponent{net_id, true, "enemy"});

    _enemies.push_back(enemy);
    _new_entities.push_back({net_id, "enemy", 0.95f, spawn_y, 25, 0});
}

void GameLogic::spawnEnemyLevel2() {
    entity enemy = _registry->spawn_entity();

    std::uniform_int_distribution<int> type_dist(0, 9);
    std::uniform_real_distribution<float> y_dist(0.1f, 0.9f);

    int roll = type_dist(_rng);
    bool is_spread = (roll < 3);
    float spawn_y = y_dist(_rng);
    uint net_id = generateNetId();

    std::string entity_type;
    int hp;
    float hitbox_w, hitbox_h;

    if (is_spread) {
        entity_type = "enemy_level2_spread";
        hp = 35;
        hitbox_w = 66.0f;
        hitbox_h = 62.0f;
        _registry->add_component(enemy, Enemy{11, 0.0f, 10, 0.0f, 0.8f});
    } else {
        entity_type = "enemy_level2";
        hp = 25;
        hitbox_w = 44.0f;
        hitbox_h = 46.0f;
        _registry->add_component(enemy, Enemy{10, 0.0f, 10, 0.0f, 1.5f});
    }

    _registry->add_component(enemy, Position{0.95f, spawn_y});
    _registry->add_component(enemy, Velocity{-0.14f, 0.0f});
    _registry->add_component(enemy, Health{hp, hp, 0.0f});
    _registry->add_component(enemy, Hitbox{hitbox_w, hitbox_h, 0.0f, 0.0f});
    _registry->add_component(enemy, NetworkComponent{net_id, true, entity_type});

    _enemies.push_back(enemy);
    _new_entities.push_back({net_id, entity_type, 0.95f, spawn_y, hp, 0});
}

void GameLogic::checkBossSpawn() {
    if (_boss_active || _boss != entity(0))
        return;

    if (_total_score >= _next_boss_threshold) {
        if (_level_id == 1) {
            spawnBoss1();
        } else if (_level_id == 2) {
            spawnBoss2();
        } else if (_endless_mode) {
            if (_boss_count % 2 == 0) {
                spawnBoss1();
            } else {
                spawnBoss2();
            }
        }
        _next_boss_threshold += 300;
        _boss_count++;
    }
}

void GameLogic::spawnBoss() {
    if (_level_id == 1) {
        spawnBoss1();
    } else {
        spawnBoss2();
    }
}

void GameLogic::spawnBoss1() {
    if (_boss_active)
        return;

    _boss = _registry->spawn_entity();
    uint net_id = generateNetId();

    int num_players = _client_to_entity.size();
    int base_hp = 1000;
    int boss_hp = base_hp * (1.0f + (num_players - 1) * 0.75f);

    _registry->add_component(_boss, Position{0.85f, 0.5f});
    _registry->add_component(_boss, Velocity{0.0f, 0.25f});
    _registry->add_component(_boss, Boss{0, 0.0f, 1, 0.0f, 5, 15.0f});
    _registry->add_component(_boss, Enemy{100, 0.0f, 100});
    _registry->add_component(_boss, Health{boss_hp, boss_hp, 0.0f});
    _registry->add_component(_boss, Hitbox{130.0f, 220.0f, 0.0f, 0.0f});
    _registry->add_component(_boss, NetworkComponent{net_id, true, "boss"});

    _boss_active = true;
    _boss_spawned = true;

    _new_entities.push_back({net_id, "boss", 0.85f, 0.5f, boss_hp, 0});
}

void GameLogic::spawnBossPart1(float boss_x, float boss_y, int part_hp) {
    entity part1 = _registry->spawn_entity();
    uint net_id1 = generateNetId();
    _registry->add_component(part1, Position{boss_x - 0.075f, boss_y});
    _registry->add_component(part1, Velocity{0.0f, 0.0f});
    _registry->add_component(part1, Boss{0, 0.0f, 2, 0.0f, 0, 0.0f});
    _registry->add_component(part1, Enemy{101, 0.0f, 100});
    _registry->add_component(part1, Health{part_hp, part_hp, 0.0f});
    _registry->add_component(part1, Hitbox{116.0f, 69.0f, 0.0f, 0.0f});
    _registry->add_component(part1, NetworkComponent{net_id1, true, "boss_level2_part1"});
    _boss_parts.push_back(part1);
    _new_entities.push_back({net_id1, "boss_level2_part1", boss_x - 0.075f, boss_y, part_hp, 0});
}

void GameLogic::spawnBossPart2(float boss_x, float boss_y, int part_hp) {
    entity part2 = _registry->spawn_entity();
    uint net_id2 = generateNetId();
    _registry->add_component(part2, Position{boss_x, boss_y - 0.025f});
    _registry->add_component(part2, Velocity{0.0f, 0.0f});
    _registry->add_component(part2, Boss{1, 0.0f, 2, 0.0f, 5, 30.0f});
    _registry->add_component(part2, Enemy{101, 0.0f, 100});
    _registry->add_component(part2, Health{part_hp, part_hp, 0.0f});
    _registry->add_component(part2, Hitbox{98.0f, 100.0f, 0.0f, 0.0f});
    _registry->add_component(part2, NetworkComponent{net_id2, true, "boss_level2_part2"});
    _boss_parts.push_back(part2);
    _boss = part2;
    _new_entities.push_back({net_id2, "boss_level2_part2", boss_x, boss_y - 0.025f, part_hp, 0});
}

void GameLogic::spawnBossPart3(float boss_x, float boss_y, int part_hp) {
    entity part3 = _registry->spawn_entity();
    uint net_id3 = generateNetId();
    _registry->add_component(part3, Position{boss_x + 0.075f, boss_y});
    _registry->add_component(part3, Velocity{0.0f, 0.0f});
    _registry->add_component(part3, Boss{2, 0.0f, 2, 0.0f, 0, 0.0f});
    _registry->add_component(part3, Enemy{101, 0.0f, 100});
    _registry->add_component(part3, Health{part_hp, part_hp, 0.0f});
    _registry->add_component(part3, Hitbox{99.0f, 83.0f, 0.0f, 0.0f});
    _registry->add_component(part3, NetworkComponent{net_id3, true, "boss_level2_part3"});
    _boss_parts.push_back(part3);
    _new_entities.push_back({net_id3, "boss_level2_part3", boss_x + 0.075f, boss_y, part_hp, 0});
}

void GameLogic::spawnBoss2() {
    if (_boss_active)
        return;

    int num_players = _client_to_entity.size();
    int base_hp = 1000;
    int part_hp = base_hp * (1.0f + (num_players - 1) * 0.75f);

    float boss_x = 0.85f;
    float boss_y = 0.5f;

    _boss_parts.clear();

    spawnBossPart1(boss_x, boss_y, part_hp);
    spawnBossPart2(boss_x, boss_y, part_hp);
    spawnBossPart3(boss_x, boss_y, part_hp);

    _boss_active = true;
    _boss_spawned = true;
}

void GameLogic::spawnProjectile(float x, float y, bool is_player_projectile, int damage) {
    entity proj = _registry->spawn_entity();

    float proj_vx = is_player_projectile ? 0.75f : -0.375f;
    uint net_id = generateNetId();

    _registry->add_component(proj, Position{x, y});
    _registry->add_component(proj, Velocity{proj_vx, 0.0f});
    _registry->add_component(proj, Projectile{is_player_projectile, damage, 5.0f});
    _registry->add_component(proj, Hitbox{8.0f, 8.0f, 0.0f, 0.0f});

    std::string proj_type = is_player_projectile ? "allied_projectile" : "projectile";
    _registry->add_component(proj, NetworkComponent{net_id, true, proj_type});

    _projectiles.push_back(proj);
    _new_entities.push_back({net_id, proj_type, x, y, 0, 0});
}

void GameLogic::spawnProjectileAtAngle(float x, float y, float angle, bool is_player_projectile, int damage) {
    entity proj = _registry->spawn_entity();

    float angle_rad = angle * 3.14159265f / 180.0f;
    float speed = is_player_projectile ? 0.75f : 0.375f;

    float proj_vx = speed * std::cos(angle_rad);
    float proj_vy = speed * std::sin(angle_rad);

    if (!is_player_projectile) {
        proj_vx = -std::abs(proj_vx);
    }

    uint net_id = generateNetId();

    _registry->add_component(proj, Position{x, y});
    _registry->add_component(proj, Velocity{proj_vx, proj_vy});
    _registry->add_component(proj, Projectile{is_player_projectile, damage, 5.0f});
    _registry->add_component(proj, Hitbox{8.0f, 8.0f, 0.0f, 0.0f});

    std::string proj_type = is_player_projectile ? "allied_projectile" : "projectile";
    _registry->add_component(proj, NetworkComponent{net_id, true, proj_type});

    _projectiles.push_back(proj);
    _new_entities.push_back({net_id, proj_type, x, y, 0, 0});
}

void GameLogic::spawnPowerUp() {
    entity powerup = _registry->spawn_entity();

    std::uniform_real_distribution<float> y_dist(0.1f, 0.9f);
    std::uniform_int_distribution<int> type_dist(0, 2);

    float spawn_y = y_dist(_rng);
    int type = type_dist(_rng);
    uint net_id = generateNetId();

    std::string entity_type;
    if (type == 0) entity_type = "powerup_shield";
    else if (type == 1) entity_type = "powerup_spread";
    else entity_type = "powerup_companion";

    _registry->add_component(powerup, Position{0.95f, spawn_y});
    _registry->add_component(powerup, Velocity{-0.125f, 0.0f});
    _registry->add_component(powerup, PowerUp{static_cast<PowerUpType>(type), 30.0f});
    _registry->add_component(powerup, Hitbox{42.0f, 34.0f, 0.0f, 0.0f});
    _registry->add_component(powerup, NetworkComponent{net_id, true, entity_type});

    _powerups.push_back(powerup);
    _new_entities.push_back({net_id, entity_type, 0.95f, spawn_y, 0, 0});
}

void GameLogic::spawnCompanionForPlayer(entity player_ent, uint client_id) {
    // Only one companion per player
    if (_player_companions.count(client_id))
        return;

    auto &positions = _registry->get_components<Position>();
    if (!positions[player_ent])
        return;

    Position &player_pos = positions[player_ent].value();
    entity companion = _registry->spawn_entity();
    uint net_id = generateNetId();

    // Offset: top-right of player (normalized 0-1 coordinates)
    float cx = player_pos.x + 0.05f;
    float cy = player_pos.y - 0.04f;

    // Companion fires at 1/3 of player's fire rate
    auto &weapons = _registry->get_components<Weapon>();
    float fire_rate = 8.0f;
    if (weapons[player_ent])
        fire_rate = weapons[player_ent].value().fire_rate;

    _registry->add_component(companion, Position{cx, cy});
    _registry->add_component(companion, Velocity{0.0f, 0.0f});
    _registry->add_component(companion, CompanionComponent{client_id, 0.0f, 3.0f / fire_rate});
    _registry->add_component(companion, NetworkComponent{net_id, true, "companion"});

    _player_companions.emplace(client_id, companion);
    _new_entities.push_back({net_id, "companion", cx, cy, 0, 0});
}
