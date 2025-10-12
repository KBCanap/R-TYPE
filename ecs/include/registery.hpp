#pragma once
#include "entity.hpp"
#include "sparse_array.hpp"
#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

class registry {
  private:
    std::unordered_map<std::type_index, std::any> _components_arrays;
    std::unordered_map<std::type_index,
                       std::function<void(registry &, const entity &)>>
        _erase_functions;
    std::size_t _next_entity_id = 0;

  public:
    std::vector<std::function<void(registry &, float)>> _systems;

    entity spawn_entity();
    entity entity_from_index(std::size_t idx);
    void kill_entity(const entity &e);

    template <class Component>
    sparse_array<Component> &register_component() {
        std::type_index type_idx(typeid(Component));
        auto it = _components_arrays.find(type_idx);
        if (it == _components_arrays.end()) {
            _components_arrays[type_idx] = sparse_array<Component>();
            _erase_functions[type_idx] = [](registry &r, const entity &e) {
                r.get_components<Component>().erase(e);
            };
        }
        return std::any_cast<sparse_array<Component> &>(
            _components_arrays[type_idx]);
    }

    template <class Component>
    sparse_array<Component> &get_components() {
        std::type_index type_idx(typeid(Component));
        auto it = _components_arrays.find(type_idx);
        if (it == _components_arrays.end())
            return register_component<Component>();
        return std::any_cast<sparse_array<Component> &>(it->second);
    }

    template <class Component>
    const sparse_array<Component> &get_components() const {
        std::type_index type_idx(typeid(Component));
        return std::any_cast<const sparse_array<Component> &>(
            _components_arrays.at(type_idx));
    }

    template <typename Component>
    typename sparse_array<Component>::reference_type
    add_component(const entity &to, Component &&c) {
        return get_components<Component>().insert_at(
            to, std::forward<Component>(c));
    }

    template <typename Component, typename... Params>
    typename sparse_array<Component>::reference_type
    emplace_component(const entity &to, Params &&...p) {
        return get_components<Component>().emplace_at(
            to, std::forward<Params>(p)...);
    }

    template <typename Component>
    void remove_component(const entity &from) {
        get_components<Component>().erase(from);
    }

    template <class... Components, typename Function>
    void add_system(Function &&f) {
        _systems.emplace_back(
            [f = std::forward<Function>(f)](registry &reg, float dt) {
                f(reg, reg.get_components<Components>()..., dt);
            });
    }

    void run_systems(float dt);
};
