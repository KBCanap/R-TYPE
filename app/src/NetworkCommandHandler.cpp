#include "../include/network/NetworkCommandHandler.hpp"
#include <iostream>

NetworkCommandHandler::NetworkCommandHandler(registry &registry,
                                             render::IRenderWindow &window,
                                             PlayerManager &player_mgr,
                                             EnemyManager &enemy_mgr,
                                             BossManager &boss_mgr)
    : registry_(registry), window_(window), player_manager_(player_mgr),
      enemy_manager_(enemy_mgr), boss_manager_(boss_mgr) {}

void NetworkCommandHandler::onCreateEntity(
    const network::CreateEntityCommand &cmd) {
    entity new_entity;

    switch (cmd.entity_type) {
    case network::EntityType::PLAYER:
        new_entity = createPlayerEntity(cmd);
        break;

    case network::EntityType::ENEMY:
        new_entity = createEnemyEntity(cmd);
        break;

    case network::EntityType::PROJECTILE:
    case network::EntityType::ALLIED_PROJECTILE:
        new_entity = createProjectileEntity(cmd);
        break;

    default:
        return;
    }

    if (new_entity._id == 0) {
        return;
    }

    registry_.add_component<component::network_entity>(
        new_entity, component::network_entity(cmd.net_id, 0, false));

    registry_.add_component<component::network_state>(
        new_entity, component::network_state());

    net_id_to_entity_[cmd.net_id] = new_entity;
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
        positions[ent]->x = cmd.position_x;
        positions[ent]->y = cmd.position_y;
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
    net_id_to_entity_.erase(cmd.net_id);
}

void NetworkCommandHandler::onFullStateSync(
    const network::FullStateSyncCommand &cmd) {
    net_id_to_entity_.clear();

    auto &network_entities =
        registry_.get_components<component::network_entity>();
    for (size_t i = 0; i < network_entities.size(); ++i) {
        if (network_entities[i]) {
            registry_.kill_entity(entity(i));
        }
    }

    for (const auto &entity_cmd : cmd.entities) {
        onCreateEntity(entity_cmd);
    }
}

void NetworkCommandHandler::onConnectionStatusChanged(
    const network::ConnectionStatusCommand &cmd) {
    std::cout << "Connection: " << static_cast<int>(cmd.old_state) << " → "
              << static_cast<int>(cmd.new_state) << std::endl;

    if (cmd.new_state == network::ConnectionState::CONNECTED) {
        std::cout << "Connected! Player ID: "
                  << static_cast<int>(cmd.player_id) << std::endl;
    } else if (cmd.new_state == network::ConnectionState::IN_GAME) {
        std::cout << "Game started! UDP Port: " << cmd.udp_port << std::endl;
    } else if (cmd.new_state == network::ConnectionState::ERROR) {
        std::cout << "Connection error!" << std::endl;
    }
}

void NetworkCommandHandler::onPlayerAssignment(
    const network::PlayerAssignmentCommand &cmd) {
    assigned_player_net_id_ = cmd.player_net_id;
    std::cout << "Player assigned NET_ID: " << cmd.player_net_id << std::endl;
}

std::optional<entity>
NetworkCommandHandler::findEntityByNetId(uint32_t net_id) const {
    auto it = net_id_to_entity_.find(net_id);
    if (it != net_id_to_entity_.end()) {
        return it->second;
    }
    return std::nullopt;
}

entity NetworkCommandHandler::createPlayerEntity(
    const network::CreateEntityCommand &cmd) {
    auto player_opt =
        player_manager_.createPlayer(cmd.position_x, cmd.position_y);

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
    entity enemy = enemy_manager_.spawnEnemy();

    auto &positions = registry_.get_components<component::position>();
    if (enemy < positions.size() && positions[enemy]) {
        positions[enemy]->x = cmd.position_x;
        positions[enemy]->y = cmd.position_y;
    }

    auto &healths = registry_.get_components<component::health>();
    if (enemy < healths.size() && healths[enemy]) {
        healths[enemy]->current_hp = cmd.health;
        healths[enemy]->max_hp = cmd.health;
    }

    return enemy;
}

entity NetworkCommandHandler::createProjectileEntity(
    const network::CreateEntityCommand &cmd) {
    auto projectile = registry_.spawn_entity();

    registry_.add_component<component::position>(
        projectile, component::position(cmd.position_x, cmd.position_y));

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
    switch (packet.msg_type) {
    case network::UDPMessageType::CLIENT_PING:
        std::cerr << "Received CLIENT_PING (unexpected on client)" << std::endl;
        break;

    case network::UDPMessageType::PLAYER_ASSIGNMENT: {
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
        if (packet.payload.size() != 17) {
            std::cerr << "Invalid ENTITY_CREATE size: " << packet.payload.size()
                      << " (expected 17)" << std::endl;
            break;
        }

        auto entity_data =
            network::PacketProcessor::parseEntityCreate(packet.payload);

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
        std::cerr << "Received PLAYER_INPUT (unexpected on client)"
                  << std::endl;
        break;

    default:
        std::cout << "Unhandled UDP packet: "
                  << static_cast<int>(packet.msg_type) << std::endl;
        break;
    }
}