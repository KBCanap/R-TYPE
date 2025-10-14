/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** registery
*/

#include "../include/registery.hpp"

entity registry::spawn_entity() { return entity(_next_entity_id++); }

entity registry::entity_from_index(std::size_t idx) { return entity(idx); }

void registry::kill_entity(const entity &e) {
    for (auto &[type_idx, erase_func] : _erase_functions)
        erase_func(*this, e);
}

void registry::run_systems(float dt) {
    for (auto &system : _systems)
        system(*this, dt);
}
