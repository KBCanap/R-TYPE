#include "../include/network/NetworkCommandHandler.hpp"
#include <iostream>
#include <set>
#include <mutex>
#include <atomic>

NetworkCommandHandler::NetworkCommandHandler(registry &registry,
                                             render::IRenderWindow &window,
                                             PlayerManager &player_mgr,
                                             EnemyManager &enemy_mgr,
                                             BossManager &boss_mgr)
    : registry_(registry), window_(window), player_manager_(player_mgr),
      enemy_manager_(enemy_mgr), boss_manager_(boss_mgr) {}

void NetworkCommandHandler::onCreateEntity(
    const network::CreateEntityCommand &cmd) {
    entity new_entity(0);

    std::cout << "[CLIENT] Creating entity type: "
              << static_cast<int>(cmd.entity_type) << " NET_ID: " << cmd.net_id
              << std::endl;

    switch (cmd.entity_type) {
    case network::EntityType::PLAYER:
        std::cout << "[CLIENT] -> Creating PLAYER" << std::endl;
        new_entity = createPlayerEntity(cmd);
        break;

    case network::EntityType::ENEMY:
        std::cout << "[CLIENT] -> Creating ENEMY (wave)" << std::endl;
        new_entity = createEnemyEntity(cmd);
        break;

    case network::EntityType::BOSS:
        std::cout << "[CLIENT] -> Creating BOSS" << std::endl;
        new_entity = createBossEntity(cmd);
        break;

    case network::EntityType::PROJECTILE:
    case network::EntityType::ALLIED_PROJECTILE:
        new_entity = createProjectileEntity(cmd);
        break;

    default:
        std::cerr << "[CLIENT] Unknown entity type: "
                  << static_cast<int>(cmd.entity_type) << std::endl;
        return;
    }

    if (new_entity == entity(0)) {
        return;
    }

    registry_.add_component<component::network_entity>(
        new_entity, component::network_entity(cmd.net_id, 0, false));

    registry_.add_component<component::network_state>(
        new_entity, component::network_state());

    {
        std::lock_guard<std::mutex> lock(net_id_mutex_);
        net_id_to_entity_.emplace(cmd.net_id, new_entity);
    }
}

void NetworkCommandHandler::onUpdateEntity(
    const network::UpdateEntityCommand &cmd) {
    auto opt_entity = findEntityByNetId(cmd.net_id);
    if (!opt_entity) {
        return;
    }

    entity ent = *opt_entity;

    auto &positions = registry_.get_components<component::position>();
    auto &healths = registry_.get_components<component::health>();
    auto &network_states = registry_.get_components<component::network_state>();

    if (ent < positions.size() && positions[ent]) {
        render::Vector2u window_size = window_.getSize();
        float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
        float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

        positions[ent]->x = pixel_x;
        positions[ent]->y = pixel_y;
    }

    if (ent < healths.size() && healths[ent]) {
        healths[ent]->current_hp = cmd.health;
    }

    if (ent < network_states.size() && network_states[ent]) {
        network_states[ent]->last_position.x = cmd.position_x;
        network_states[ent]->last_position.y = cmd.position_y;
        network_states[ent]->interpolation_time = 0.0f;
    }
}

void NetworkCommandHandler::onDestroyEntity(
    const network::DestroyEntityCommand &cmd) {
    auto opt_entity = findEntityByNetId(cmd.net_id);
    if (!opt_entity) {
        return;
    }

    entity ent = *opt_entity;
    registry_.kill_entity(ent);
    
    {
        std::lock_guard<std::mutex> lock(net_id_mutex_);
        net_id_to_entity_.erase(cmd.net_id);
    }
}

void NetworkCommandHandler::onFullStateSync(
    const network::FullStateSyncCommand &cmd) {
    std::cout << "[NetworkCommandHandler] Processing GAME_STATE with "
              << cmd.entities.size() << " entities" << std::endl;

    std::set<uint32_t> received_net_ids;

    for (const auto &entity_cmd : cmd.entities) {
        received_net_ids.insert(entity_cmd.net_id);

        auto existing_entity = findEntityByNetId(entity_cmd.net_id);

        if (existing_entity) {
            std::cout
                << "[NetworkCommandHandler] Updating existing entity NET_ID: "
                << entity_cmd.net_id << std::endl;

            network::UpdateEntityCommand update_cmd;
            update_cmd.net_id = entity_cmd.net_id;
            update_cmd.health = entity_cmd.health;
            update_cmd.position_x = entity_cmd.position_x;
            update_cmd.position_y = entity_cmd.position_y;
            onUpdateEntity(update_cmd);
        } else {
            std::cout << "[NetworkCommandHandler] Creating new entity NET_ID: "
                      << entity_cmd.net_id << std::endl;
            onCreateEntity(entity_cmd);
        }
    }
    /*
    // Supprimer les entités qui ne sont plus dans le GAME_STATE
    std::vector<uint32_t> entities_to_remove;
    for (const auto &pair : net_id_to_entity_) {
        if (received_net_ids.find(pair.first) == received_net_ids.end()) {
            entities_to_remove.push_back(pair.first);
        }
    }

    for (uint32_t net_id : entities_to_remove) {
        std::cout << "[NetworkCommandHandler] Removing disappeared entity
    NET_ID: "
                  << net_id << std::endl;
        network::DestroyEntityCommand destroy_cmd;
        destroy_cmd.net_id = net_id;
        onDestroyEntity(destroy_cmd);
    }*/
}

void NetworkCommandHandler::onConnectionStatusChanged(
    const network::ConnectionStatusCommand &cmd) {
    std::cout << "Connection: " << static_cast<int>(cmd.old_state) << " → "
              << static_cast<int>(cmd.new_state) << std::endl;

    if (cmd.new_state == network::ConnectionState::CONNECTED) {
        std::cout << "Connected! Player ID: " << static_cast<int>(cmd.player_id)
                  << std::endl;
    } else if (cmd.new_state == network::ConnectionState::IN_GAME) {
        std::cout << "Game started! UDP Port: " << cmd.udp_port << std::endl;
    } else if (cmd.new_state == network::ConnectionState::ERROR) {
        std::cout << "Connection error!" << std::endl;
    }
}

void NetworkCommandHandler::onPlayerAssignment(
    const network::PlayerAssignmentCommand &cmd) {
    assigned_player_net_id_.store(cmd.player_net_id);
    std::cout << "Player assigned NET_ID: " << cmd.player_net_id << std::endl;
}

uint32_t NetworkCommandHandler::getAssignedPlayerNetId() const {
    return assigned_player_net_id_.load();
}

std::optional<entity>
NetworkCommandHandler::findEntityByNetId(uint32_t net_id) const {
    std::lock_guard<std::mutex> lock(net_id_mutex_);
    auto it = net_id_to_entity_.find(net_id);
    if (it != net_id_to_entity_.end()) {
        return it->second;
    }
    return std::nullopt;
}

entity NetworkCommandHandler::createPlayerEntity(
    const network::CreateEntityCommand &cmd) {
    // ✅ CORRECTION : Convertir pourcentage en pixels
    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    auto player_opt = player_manager_.createPlayer(pixel_x, pixel_y);

    if (!player_opt) {
        return entity(0);
    }

    entity player = *player_opt;

    auto &healths = registry_.get_components<component::health>();
    if (player < healths.size() && healths[player]) {
        healths[player]->current_hp = cmd.health;
        healths[player]->max_hp = cmd.health;
    }

    return player;
}

entity NetworkCommandHandler::createEnemyEntity(
    const network::CreateEntityCommand &cmd) {
    auto enemy = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        enemy, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        enemy, component::velocity(0.0f, 0.0f));

    // Wave enemy - use r-typesheet9.gif
    registry_.add_component<component::drawable>(
        enemy, component::drawable("assets/sprites/r-typesheet9.gif",
                                   render::IntRect(), 1.0f, "enemy"));

    registry_.add_component<component::hitbox>(
        enemy, component::hitbox(50.0f, 58.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(enemy,
                                               component::health(cmd.health));

    // Add AI input with wave movement pattern (movement only, no shooting)
    float fire_interval = 1.5f;
    component::ai_movement_pattern movement_pattern =
        component::ai_movement_pattern::wave(50.0f, 0.01f, 120.0f);
    registry_.add_component<component::ai_input>(
        enemy, component::ai_input(true, fire_interval, movement_pattern));

    // Add animation frames for wave enemy (3 frames)
    auto &anim = registry_.add_component<component::animation>(
        enemy, component::animation(0.5f, true));
    anim->frames.push_back(render::IntRect(0, 0, 50, 58));   // Frame 1
    anim->frames.push_back(render::IntRect(51, 0, 57, 58));  // Frame 2
    anim->frames.push_back(render::IntRect(116, 0, 49, 58)); // Frame 3

    return enemy;
}

entity NetworkCommandHandler::createBossEntity(
    const network::CreateEntityCommand &cmd) {
    auto boss = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        boss, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        boss, component::velocity(0.0f, 100.0f));

    // Boss - use r-typesheet17.gif
    registry_.add_component<component::drawable>(
        boss, component::drawable("assets/sprites/r-typesheet17.gif",
                                  render::IntRect(), 2.0f, "boss"));

    registry_.add_component<component::hitbox>(
        boss, component::hitbox(130.0f, 220.0f, 0.0f, 0.0f));

    registry_.add_component<component::health>(boss,
                                               component::health(cmd.health));

    // No weapon component needed - server handles all shooting in multiplayer

    // Boss AI input - no movement pattern (bounce is handled by
    // position_system)
    auto boss_ai = registry_.add_component<component::ai_input>(
        boss, component::ai_input(
                  true, 0.5f, component::ai_movement_pattern::straight(0.0f)));
    // Disable AI movement so it doesn't override the bounce behavior
    boss_ai->movement_pattern.base_speed = 0.0f;

    // Boss animation: 8 frames
    auto &boss_anim = registry_.add_component<component::animation>(
        boss, component::animation(0.1f, true));
    for (int i = 0; i < 8; ++i) {
        boss_anim->frames.push_back(render::IntRect(i * 65, 0, 65, 132));
    }

    return boss;
}

entity NetworkCommandHandler::createProjectileEntity(
    const network::CreateEntityCommand &cmd) {
    auto projectile = registry_.spawn_entity();

    render::Vector2u window_size = window_.getSize();
    float pixel_x = cmd.position_x * static_cast<float>(window_size.x);
    float pixel_y = cmd.position_y * static_cast<float>(window_size.y);

    registry_.add_component<component::position>(
        projectile, component::position(pixel_x, pixel_y));

    registry_.add_component<component::velocity>(
        projectile, component::velocity(0.0f, 0.0f));

    bool is_friendly =
        (cmd.entity_type == network::EntityType::ALLIED_PROJECTILE);

    std::string texture_path = "assets/sprites/r-typesheet1.gif";
    render::IntRect sprite_rect = render::IntRect(60, 353, 12, 12);
    std::string tag = is_friendly ? "allied_projectile" : "projectile";

    registry_.add_component<component::drawable>(
        projectile, component::drawable(texture_path, sprite_rect, 2.0f, tag));

    registry_.add_component<component::projectile>(
        projectile, component::projectile(25.0f, 500.0f, is_friendly, "bullet",
                                          5.0f, false, 1));

    registry_.add_component<component::health>(projectile,
                                               component::health(cmd.health));

    return projectile;
}

void NetworkCommandHandler::onRawTCPMessage(const network::TCPMessage &msg) {
    switch (msg.msg_type) {
    case network::MessageType::TCP_CONNECT_ACK:
        if (msg.data.size() >= 1) {
            network::ConnectionStatusCommand cmd;
            cmd.old_state = network::ConnectionState::CONNECTING;
            cmd.new_state = network::ConnectionState::CONNECTED;
            cmd.player_id = msg.data[0];
            cmd.udp_port = 0;
            onConnectionStatusChanged(cmd);
        } else {
            std::cerr << "Invalid TCP_CONNECT_ACK payload" << std::endl;
        }
        break;

    case network::MessageType::TCP_CONNECT_NAK: {
        network::ConnectionStatusCommand cmd;
        cmd.old_state = network::ConnectionState::CONNECTING;
        cmd.new_state = network::ConnectionState::ERROR;
        cmd.player_id = 0;
        cmd.udp_port = 0;
        onConnectionStatusChanged(cmd);
    } break;

    case network::MessageType::TCP_GAME_START:
        if (msg.data.size() >= 2) {
            uint16_t udp_port =
                (static_cast<uint16_t>(msg.data[0]) << 8) | msg.data[1];
            network::ConnectionStatusCommand cmd;
            cmd.old_state = network::ConnectionState::WAITING;
            cmd.new_state = network::ConnectionState::GAME_STARTING;
            cmd.player_id = 0;
            cmd.udp_port = udp_port;
            onConnectionStatusChanged(cmd);
        } else {
            std::cerr << "Invalid TCP_GAME_START payload" << std::endl;
        }
        break;

    case network::MessageType::TCP_ERROR: {
        network::ConnectionStatusCommand cmd;
        cmd.old_state = network::ConnectionState::CONNECTED;
        cmd.new_state = network::ConnectionState::ERROR;
        cmd.player_id = 0;
        cmd.udp_port = 0;
        onConnectionStatusChanged(cmd);
    } break;

    default:
        std::cout << "Unhandled TCP message: " << static_cast<int>(msg.msg_type)
                  << std::endl;
        break;
    }
}

void NetworkCommandHandler::onRawUDPPacket(const network::UDPPacket &packet) {
    std::cout << "[NetworkCommandHandler] Processing UDP packet type: "
              << static_cast<int>(packet.msg_type)
              << " payload size: " << packet.payload.size() << std::endl;

    switch (packet.msg_type) {
    case network::UDPMessageType::CLIENT_PING:
        std::cerr << "Received CLIENT_PING (unexpected on client)" << std::endl;
        break;

    case network::UDPMessageType::PLAYER_ASSIGNMENT: {
        std::cout << "[NetworkCommandHandler] Processing PLAYER_ASSIGNMENT"
                  << std::endl;
        if (packet.payload.size() != 4) {
            std::cerr << "Invalid PLAYER_ASSIGNMENT size: "
                      << packet.payload.size() << " (expected 4)" << std::endl;
            break;
        }

        uint32_t player_net_id =
            network::PacketProcessor::parsePlayerAssignment(packet.payload);
        network::PlayerAssignmentCommand cmd;
        cmd.player_net_id = player_net_id;
        onPlayerAssignment(cmd);
        break;
    }

    case network::UDPMessageType::ENTITY_CREATE: {
        std::cout << "[NetworkCommandHandler] Processing ENTITY_CREATE"
                  << std::endl;
        if (packet.payload.size() != 17) {
            std::cerr << "Invalid ENTITY_CREATE size: " << packet.payload.size()
                      << " (expected 17)" << std::endl;
            break;
        }

        auto entity_data =
            network::PacketProcessor::parseEntityCreate(packet.payload);

        std::cout << "[NetworkCommandHandler] Entity data - NET_ID: "
                  << entity_data.net_id
                  << " Type: " << static_cast<int>(entity_data.entity_type)
                  << std::endl;

        network::CreateEntityCommand cmd;
        cmd.net_id = entity_data.net_id;
        cmd.entity_type = entity_data.entity_type;
        cmd.health = entity_data.health;
        cmd.position_x = entity_data.position_x;
        cmd.position_y = entity_data.position_y;

        onCreateEntity(cmd);
        break;
    }

    case network::UDPMessageType::ENTITY_UPDATE: {
        std::cout << "[NetworkCommandHandler] Processing ENTITY_UPDATE"
                  << std::endl;
        if (packet.payload.size() == 0 || packet.payload.size() % 16 != 0) {
            std::cerr << "Invalid ENTITY_UPDATE size: " << packet.payload.size()
                      << " (must be multiple of 16)" << std::endl;
            break;
        }

        auto updates =
            network::PacketProcessor::parseEntityUpdate(packet.payload);

        for (const auto &update : updates) {
            network::UpdateEntityCommand cmd;
            cmd.net_id = update.net_id;
            cmd.health = update.health;
            cmd.position_x = update.position_x;
            cmd.position_y = update.position_y;

            onUpdateEntity(cmd);
        }
        break;
    }

    case network::UDPMessageType::ENTITY_DESTROY: {
        std::cout << "[NetworkCommandHandler] Processing ENTITY_DESTROY"
                  << std::endl;
        if (packet.payload.size() == 0 || packet.payload.size() % 4 != 0) {
            std::cerr << "Invalid ENTITY_DESTROY size: "
                      << packet.payload.size() << " (must be multiple of 4)"
                      << std::endl;
            break;
        }

        auto net_ids =
            network::PacketProcessor::parseEntityDestroy(packet.payload);

        for (uint32_t net_id : net_ids) {
            network::DestroyEntityCommand cmd;
            cmd.net_id = net_id;

            onDestroyEntity(cmd);
        }
        break;
    }

    case network::UDPMessageType::GAME_STATE: {
        std::cout << "[NetworkCommandHandler] Processing GAME_STATE"
                  << std::endl;
        if (packet.payload.size() < 4) {
            std::cerr << "Invalid GAME_STATE size: " << packet.payload.size()
                      << " (minimum 4)" << std::endl;
            break;
        }

        if ((packet.payload.size() - 4) % 17 != 0) {
            std::cerr << "Invalid GAME_STATE size: " << packet.payload.size()
                      << " (should be 4 + N*17)" << std::endl;
            break;
        }

        auto entities =
            network::PacketProcessor::parseGameState(packet.payload);

        network::FullStateSyncCommand cmd;
        for (const auto &entity_data : entities) {
            network::CreateEntityCommand entity_cmd;
            entity_cmd.net_id = entity_data.net_id;
            entity_cmd.entity_type = entity_data.entity_type;
            entity_cmd.health = entity_data.health;
            entity_cmd.position_x = entity_data.position_x;
            entity_cmd.position_y = entity_data.position_y;

            cmd.entities.push_back(entity_cmd);
        }

        onFullStateSync(cmd);
        break;
    }

    case network::UDPMessageType::PLAYER_INPUT:
        break;

    default:
        std::cerr << "[NetworkCommandHandler] Unknown UDP message type: " 
                  << static_cast<int>(packet.msg_type) << std::endl;
        break;
    }
}