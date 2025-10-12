#include "include/Event.hpp"
#include "include/EventBus.hpp"
#include <iostream>
#include <string>

struct DamagePayload {
    class Entity *target;
    int amount;
};

struct XPPayload {
    class Entity *target;
    int amount;
};

struct CollisionPayload {
    class Entity *target;
};

class Entity {
  public:
    std::string name;
    int hp = 100;
    int xp = 0;
    int level = 1;

    Entity(const std::string &n) : name(n) {}

    void TakeDamage(int amount) {
        hp -= amount;
        std::cout << name << " took " << amount << " damage. HP: " << hp
                  << "\n";
    }

    void GainXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            std::cout << name << " leveled up! Level: " << level
                      << ", XP: " << xp << "\n";
        } else {
            std::cout << name << " gained " << amount << " XP. Total XP: " << xp
                      << "\n";
        }
    }

    void OnCollision() {
        std::cout << name << " smashed their head into a wall!\n";
    }
};

int main() {
    EventBus bus;
    Entity player{"Player1"};

    EventBus::SubscriptionID damage_sub_p1 =
        bus.Subscribe(EventType::Damage, [&](const std::any &payload) {
            try {
                auto damage_data = std::any_cast<DamagePayload>(payload);
                if (damage_data.target == &player) {
                    player.TakeDamage(damage_data.amount);
                }
            } catch (const std::bad_any_cast &e) {
                std::cerr << "Invalid damage payload\n";
            }
        });

    EventBus::SubscriptionID level_sub_p1 =
        bus.Subscribe(EventType::LevelUp, [&](const std::any &payload) {
            try {
                auto xp_data = std::any_cast<XPPayload>(payload);
                if (xp_data.target == &player) {
                    player.GainXP(xp_data.amount);
                }
            } catch (const std::bad_any_cast &e) {
                std::cerr << "Invalid XP payload\n";
            }
        });

    EventBus::SubscriptionID collision_sub_p1 =
        bus.Subscribe(EventType::Collision, [&](const std::any &payload) {
            try {
                auto collision_data = std::any_cast<CollisionPayload>(payload);
                if (collision_data.target == &player) {
                    player.OnCollision();
                }
            } catch (const std::bad_any_cast &e) {
                std::cerr << "Invalid collision payload\n";
            }
        });

    Entity player2{"Player2"};

    EventBus::SubscriptionID damage_sub_p2 =
        bus.Subscribe(EventType::Damage, [&](const std::any &payload) {
            try {
                auto damage_data = std::any_cast<DamagePayload>(payload);
                if (damage_data.target == &player) {
                    player.TakeDamage(damage_data.amount);
                }
            } catch (const std::bad_any_cast &e) {
                std::cerr << "Invalid damage payload\n";
            }
        });

    EventBus::SubscriptionID level_sub_p2 =
        bus.Subscribe(EventType::LevelUp, [&](const std::any &payload) {
            try {
                auto xp_data = std::any_cast<XPPayload>(payload);
                if (xp_data.target == &player) {
                    player.GainXP(xp_data.amount);
                }
            } catch (const std::bad_any_cast &e) {
                std::cerr << "Invalid XP payload\n";
            }
        });

    EventBus::SubscriptionID collision_sub_p2 =
        bus.Subscribe(EventType::Collision, [&](const std::any &payload) {
            try {
                auto collision_data = std::any_cast<CollisionPayload>(payload);
                if (collision_data.target == &player) {
                    player.OnCollision();
                }
            } catch (const std::bad_any_cast &e) {
                std::cerr << "Invalid collision payload\n";
            }
        });

    std::cout << "=== Event System Demo ===\n\n";

    // Trigger events
    bus.Notify(EventType::Damage, DamagePayload{&player, 25});
    bus.Notify(EventType::LevelUp, XPPayload{&player, 60});
    bus.Notify(EventType::LevelUp, XPPayload{&player, 50}); // triggers level up
    bus.Notify(EventType::Collision, CollisionPayload{&player});

    std::cout << "\n=== Testing Unsubscribe ===\n";

    // Unsubscribe from damage
    if (bus.Unsubscribe(EventType::Damage, damage_sub_p1)) {
        std::cout << "Successfully unsubscribed from Damage events\n";
    }

    // This should not trigger anything since we unsubscribed
    bus.Notify(EventType::Damage, DamagePayload{&player, 10});
    std::cout << "Damage event sent but no callback triggered (unsubscribed)\n";

    // This should still work
    bus.Notify(EventType::Collision, CollisionPayload{&player});
    std::cout << "\n=== Event Statistics ===\n";
    std::cout << "Damage subscribers: "
              << bus.GetSubscriberCount(EventType::Damage) << "\n";
    std::cout << "LevelUp subscribers: "
              << bus.GetSubscriberCount(EventType::LevelUp) << "\n";
    std::cout << "Collision subscribers: "
              << bus.GetSubscriberCount(EventType::Collision) << "\n";

    std::cout << "Collision has subscribers: "
              << (bus.HasSubscribers(EventType::Collision) ? "TRUE" : "FALSE")
              << std::endl;
    bus.ClearAll();
    std::cout << "Collision has subscribers: "
              << (bus.HasSubscribers(EventType::Collision) ? "TRUE" : "FALSE")
              << std::endl;

    return 0;
}