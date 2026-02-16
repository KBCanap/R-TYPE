#include "gamelogic/GameLogic.hpp"
#include <algorithm>
#include <cmath>

WorldSnapshot GameLogic::generateSnapshot() {
    WorldSnapshot snapshot;
    snapshot.tick = _current_tick;
    snapshot.timestamp = std::chrono::steady_clock::now();

    auto &positions = _registry->get_components<Position>();
    auto &velocities = _registry->get_components<Velocity>();
    auto &healths = _registry->get_components<Health>();
    auto &shields = _registry->get_components<Shield>();
    auto &scores = _registry->get_components<Score>();
    auto &network_comps = _registry->get_components<NetworkComponent>();

    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (!net_opt)
            continue;

        entity ent = _registry->entity_from_index(i);
        auto pos_opt = positions[ent];
        auto vel_opt = velocities[ent];

        if (pos_opt && vel_opt) {
            EntitySnapshot entity_snap;
            entity_snap.net_id = net_opt.value().net_id;
            entity_snap.entity_type = net_opt.value().entity_type;
            entity_snap.pos = pos_opt.value();
            entity_snap.vel = vel_opt.value();
            entity_snap.health = healths[ent] ? healths[ent].value().current_hp : 0;
            entity_snap.shield = shields[ent] ? shields[ent].value().current_shield : 0;
            entity_snap.score = scores[ent] ? scores[ent].value().current_score : 0;
            entity_snap.tick = _current_tick;

            snapshot.entities.push_back(entity_snap);
        }
    }

    _snapshot_history.push_back(snapshot);
    if (_snapshot_history.size() > MAX_SNAPSHOT_HISTORY) {
        _snapshot_history.pop_front();
    }

    return snapshot;
}

std::vector<EntitySnapshot> GameLogic::getDeltaSnapshot(uint last_acked_tick) {
    auto acked_snapshot =
        std::find_if(_snapshot_history.begin(), _snapshot_history.end(),
                     [last_acked_tick](const WorldSnapshot &s) {
                         return s.tick == last_acked_tick;
                     });

    if (acked_snapshot == _snapshot_history.end()) {
        auto current = generateSnapshot();
        return current.entities;
    }

    std::unordered_map<uint, EntitySnapshot> old_entities;
    for (const auto &entity : acked_snapshot->entities) {
        old_entities[entity.net_id] = entity;
    }

    std::vector<EntitySnapshot> delta;
    auto current_snapshot = generateSnapshot();

    const float POS_THRESHOLD = 5.0f;

    for (const auto &entity : current_snapshot.entities) {
        auto old_it = old_entities.find(entity.net_id);
        bool should_send = false;

        if (old_it == old_entities.end()) {
            should_send = true;
        } else {
            const EntitySnapshot &old = old_it->second;
            float pos_delta = std::sqrt(std::pow(entity.pos.x - old.pos.x, 2) +
                                        std::pow(entity.pos.y - old.pos.y, 2));

            should_send = (pos_delta > POS_THRESHOLD || entity.health != old.health ||
                          entity.shield != old.shield || entity.score != old.score);
        }

        if (should_send) {
            delta.push_back(entity);
        }
    }

    return delta;
}

void GameLogic::markEntitiesSynced() {
    auto &network_comps = _registry->get_components<NetworkComponent>();
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (net_opt) {
            net_opt.value().needs_update = false;
        }
    }
}

entity GameLogic::findEntityByNetId(uint net_id) {
    auto &network_comps = _registry->get_components<NetworkComponent>();
    for (size_t i = 0; i < network_comps.size(); ++i) {
        auto net_opt = network_comps[i];
        if (net_opt && net_opt.value().net_id == net_id) {
            return _registry->entity_from_index(i);
        }
    }
    return entity(static_cast<size_t>(-1));
}
