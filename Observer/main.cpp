#include "include/EventBus.hpp"
#include <any>
#include <iostream>
#include <string>


// Event define macro
#define Collision_evnt "Collision"
#define Damage_evnt "Damage"
#define Level_evnt "Level"

struct Entity {
    std::string name;
    int hp = 100;
    int xp = 0;
    int level = 1;

    void TakeDamage(int amount) {
        hp -= amount;
        std::cout << name << " took " << amount << " damage. HP: " << hp << "\n";
    }

    void GainXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            std::cout << name << " leveled up! Level: " << level << ", XP: " << xp << "\n";
        } else {
            std::cout << name << " gained " << amount << " XP. Total XP: " << xp << "\n";
        }
    }

    void Collision(void) {
        std::cout << name << " smash is head to a wall" << std::endl;
    }
};

int main() {
    EventBus bus;
    Entity player{"Player1"};

    bus.Subscribe(Damage_evnt, [&](const std::any& payload) {
        auto [target, amount] = std::any_cast<std::pair<Entity*, int>>(payload);
        if (target == &player) player.TakeDamage(amount);
    });

    bus.Subscribe(Level_evnt, [&](const std::any& payload) {
        auto [target, xp] = std::any_cast<std::pair<Entity*, int>>(payload);
        if (target == &player) player.GainXP(xp);
    });

    bus.Subscribe(Collision_evnt, [&](const std::any& payload) {
        auto target = std::any_cast<Entity*>(payload);
        if (target == &player) player.Collision();
    });

    bus.Notify(Damage_evnt, std::make_pair(&player, 25));
    bus.Notify(Level_evnt, std::make_pair(&player, 60));
    bus.Notify(Level_evnt, std::make_pair(&player, 50)); // triggers level up
    bus.Notify(Collision_evnt, &player);

    return 0;
}
