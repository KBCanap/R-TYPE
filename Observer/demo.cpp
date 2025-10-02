/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** main
*/

#include "include/EventBus.hpp"
#include <iostream>
#include <string>
#include <vector>

// Event payload structures
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
    std::string object;
};

struct PlayerHitPayload {
    class Entity *attacker;
    class Entity *victim;
    int damage;
};

struct ItemPickupPayload {
    class Entity *player;
    std::string item_name;
};

struct GameOverPayload {
    std::string reason;
    std::vector<class Entity *> players;
};

// Game entity class
class Entity {
  public:
    std::string name;
    int hp = 100;
    int xp = 0;
    int level = 1;
    bool is_alive = true;

    Entity(const std::string &n) : name(n) {}

    void TakeDamage(int amount) {
        hp -= amount;
        if (hp <= 0) {
            hp = 0;
            is_alive = false;
            std::cout << "[" << name << "] took " << amount
                      << " damage and DIED!\n";
        } else {
            std::cout << "[" << name << "] took " << amount
                      << " damage. HP: " << hp << "/100\n";
        }
    }

    void GainXP(int amount) {
        xp += amount;
        if (xp >= 100) {
            level++;
            xp -= 100;
            std::cout << "[" << name << "] LEVELED UP! Now Level " << level
                      << " (XP: " << xp << "/100)\n";
        } else {
            std::cout << "[" << name << "] gained " << amount
                      << " XP. Progress: " << xp << "/100\n";
        }
    }

    void OnCollision(const std::string &object) {
        std::cout << "[" << name << "] collided with " << object << "!\n";
    }

    void OnHit(Entity *victim, int damage) {
        std::cout << "[" << name << "] hit [" << victim->name << "]  for "
                  << damage << " damage!\n";
    }

    void OnPickup(const std::string &item) {
        std::cout << "[" << name << "] picked up: " << item << "\n";
    }
};

void PrintSeparator(const std::string &title) {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(50, '=') << "\n";
}

int main() {
    EventBus bus;
    Entity player1{"Player1"};
    Entity player2{"Player2"};
    Entity player3{"Player3"};

    PrintSeparator("EVENTBUS COMPREHENSIVE TEST");

    // ============================================
    // TEST 1: Basic Subscribe
    // ============================================
    std::cout << "\n[TEST 1] Subscribe to events\n";

    auto dmg_sub_p1 =
        bus.Subscribe(EventType::Damage, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<DamagePayload>(payload);
                if (data.target == &player1)
                    player1.TakeDamage(data.amount);
            } catch (const std::bad_any_cast &) {
            }
        });

    auto dmg_sub_p2 =
        bus.Subscribe(EventType::Damage, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<DamagePayload>(payload);
                if (data.target == &player2)
                    player2.TakeDamage(data.amount);
            } catch (const std::bad_any_cast &) {
            }
        });

    auto xp_sub_p1 =
        bus.Subscribe(EventType::LevelUp, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<XPPayload>(payload);
                if (data.target == &player1)
                    player1.GainXP(data.amount);
            } catch (const std::bad_any_cast &) {
            }
        });

    auto col_sub_p1 =
        bus.Subscribe(EventType::Collision, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<CollisionPayload>(payload);
                if (data.target == &player1)
                    player1.OnCollision(data.object);
            } catch (const std::bad_any_cast &) {
            }
        });

    auto col_sub_p2 =
        bus.Subscribe(EventType::Collision, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<CollisionPayload>(payload);
                if (data.target == &player2)
                    player2.OnCollision(data.object);
            } catch (const std::bad_any_cast &) {
            }
        });

    std::cout << "  Subscriptions created\n";
    std::cout << "  Damage subscribers: "
              << bus.GetSubscriberCount(EventType::Damage) << "\n";
    std::cout << "  LevelUp subscribers: "
              << bus.GetSubscriberCount(EventType::LevelUp) << "\n";
    std::cout << "  Collision subscribers: "
              << bus.GetSubscriberCount(EventType::Collision) << "\n";

    // ============================================
    // TEST 2: Notify with payload
    // ============================================
    PrintSeparator("TEST 2: Notify Events");

    bus.Notify(EventType::Damage, DamagePayload{&player1, 25});
    bus.Notify(EventType::Damage, DamagePayload{&player2, 15});
    bus.Notify(EventType::LevelUp, XPPayload{&player1, 60});
    bus.Notify(EventType::Collision, CollisionPayload{&player1, "wall"});

    // ============================================
    // TEST 3: Notify with Event struct
    // ============================================
    PrintSeparator("TEST 3: Notify Using Event Struct");

    Event dmg_event(EventType::Damage, DamagePayload{&player2, 10});
    bus.Notify(dmg_event);

    Event xp_event(EventType::LevelUp, XPPayload{&player1, 45});
    bus.Notify(xp_event);
    std::cout << "Event struct notifications working\n";

    // ============================================
    // TEST 4: Custom Event - Player Hits Player
    // ============================================
    PrintSeparator("TEST 4: Custom Event - Player Hits Player");

    // Subscribe both players to PlayerHit event
    auto hit_sub_attacker =
        bus.Subscribe(EventType::PlayerHit, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<PlayerHitPayload>(payload);
                if (data.attacker == &player1) {
                    player1.OnHit(data.victim, data.damage);
                } else if (data.attacker == &player2) {
                    player2.OnHit(data.victim, data.damage);
                }
            } catch (const std::bad_any_cast &) {
            }
        });

    auto hit_sub_victim =
        bus.Subscribe(EventType::PlayerHit, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<PlayerHitPayload>(payload);
                if (data.victim == &player1) {
                    player1.TakeDamage(data.damage);
                } else if (data.victim == &player2) {
                    player2.TakeDamage(data.damage);
                }
            } catch (const std::bad_any_cast &) {
            }
        });

    std::cout << "\nPlayer1 hits Player2:\n";
    bus.Notify(EventType::PlayerHit, PlayerHitPayload{&player1, &player2, 30});

    std::cout << "\nPlayer2 hits Player1:\n";
    bus.Notify(EventType::PlayerHit, PlayerHitPayload{&player2, &player1, 20});

    // Send collision to BOTH players when they hit each other
    std::cout << "\nBoth players collide with each other:\n";
    bus.Notify(EventType::Collision, CollisionPayload{&player1, "Player2"});
    bus.Notify(EventType::Collision, CollisionPayload{&player2, "Player1"});

    // ============================================
    // TEST 5: Multiple Events in Sequence
    // ============================================
    PrintSeparator("TEST 5: Multiple Events in Sequence");

    bus.Notify(EventType::Damage, DamagePayload{&player1, 10});
    bus.Notify(EventType::LevelUp, XPPayload{&player1, 50}); // Should level up
    bus.Notify(EventType::Collision, CollisionPayload{&player2, "door"});

    // ============================================
    // TEST 6: Item Pickup Custom Event
    // ============================================
    PrintSeparator("TEST 6: Custom Event - Item Pickup");

    auto item_sub =
        bus.Subscribe(EventType::ItemPickup, [&](const std::any &payload) {
            try {
                auto data = std::any_cast<ItemPickupPayload>(payload);
                if (data.player == &player1) {
                    player1.OnPickup(data.item_name);
                } else if (data.player == &player2) {
                    player2.OnPickup(data.item_name);
                }
            } catch (const std::bad_any_cast &) {
            }
        });

    bus.Notify(EventType::ItemPickup,
               ItemPickupPayload{&player1, "Health Potion"});
    bus.Notify(EventType::ItemPickup,
               ItemPickupPayload{&player2, "Magic Sword"});

    // ============================================
    // TEST 7: Unsubscribe
    // ============================================
    PrintSeparator("TEST 7: Unsubscribe");

    std::cout << "Before unsubscribe - Damage subscribers: "
              << bus.GetSubscriberCount(EventType::Damage) << "\n";

    if (bus.Unsubscribe(EventType::Damage, dmg_sub_p1)) {
        std::cout << "Player1 unsubscribed from Damage\n";
    }

    std::cout << "After unsubscribe - Damage subscribers: "
              << bus.GetSubscriberCount(EventType::Damage) << "\n\n";

    std::cout << "Sending damage to both players:\n";
    bus.Notify(EventType::Damage, DamagePayload{&player1, 20}); // Won't trigger
    bus.Notify(EventType::Damage, DamagePayload{&player2, 20}); // Will trigger
    std::cout << "(Notice: Player1 didn't take damage)\n";

    // ============================================
    // TEST 8: HasSubscribers
    // ============================================
    PrintSeparator("TEST 8: HasSubscribers");

    std::cout << "Damage has subscribers: "
              << (bus.HasSubscribers(EventType::Damage) ? "YES" : "NO") << "\n";
    std::cout << "LevelUp has subscribers: "
              << (bus.HasSubscribers(EventType::LevelUp) ? "YES" : "NO")
              << "\n";
    std::cout << "GameOver has subscribers: "
              << (bus.HasSubscribers(EventType::GameOver) ? "YES" : "NO")
              << "\n";

    // ============================================
    // TEST 9: ClearEvent (specific event)
    // ============================================
    PrintSeparator("TEST 9: ClearEvent (Specific Type)");

    std::cout << "Before clear - Collision subscribers: "
              << bus.GetSubscriberCount(EventType::Collision) << "\n";

    bus.ClearEvent(EventType::Collision);

    std::cout << "After clear - Collision subscribers: "
              << bus.GetSubscriberCount(EventType::Collision) << "\n";
    std::cout << "Collision has subscribers: "
              << (bus.HasSubscribers(EventType::Collision) ? "YES" : "NO")
              << "\n\n";

    std::cout << "Attempting collision event:\n";
    bus.Notify(EventType::Collision, CollisionPayload{&player1, "nothing"});
    std::cout << "(No handler called - event cleared)\n";

    // ============================================
    // TEST 10: Events with no payload
    // ============================================
    PrintSeparator("TEST 10: Events with Empty Payload");

    int global_event_count = 0;
    auto global_sub =
        bus.Subscribe(EventType::GameOver, [&](const std::any &payload) {
            global_event_count++;
            std::cout << " GAME OVER triggered! Count: " << global_event_count
                      << "\n";

            // Try to get payload if it exists
            try {
                auto data = std::any_cast<GameOverPayload>(payload);
                std::cout << "   Reason: " << data.reason << "\n";
            } catch (const std::bad_any_cast &) {
                std::cout << "   (No payload provided)\n";
            }
        });

    bus.Notify(EventType::GameOver); // No payload
    bus.Notify(EventType::GameOver,
               GameOverPayload{"All players died", {&player1, &player2}});

    // ============================================
    // TEST 11: Multiple subscribers to same event
    // ============================================
    PrintSeparator("TEST 11: Multiple Subscribers Same Event");

    auto log_sub1 = bus.Subscribe(EventType::LevelUp, [](const std::any &) {
        std::cout << "  [Logger 1] XP event logged\n";
    });

    auto log_sub2 = bus.Subscribe(EventType::LevelUp, [](const std::any &) {
        std::cout << "  [Logger 2] Achievement tracker updated\n";
    });

    auto log_sub3 = bus.Subscribe(EventType::LevelUp, [](const std::any &) {
        std::cout << "  [Logger 3] Saved to database\n";
    });

    std::cout << "LevelUp has " << bus.GetSubscriberCount(EventType::LevelUp)
              << " subscribers\n\n";
    std::cout << "Triggering LevelUp event:\n";
    bus.Notify(EventType::LevelUp, XPPayload{&player1, 10});

    // ============================================
    // TEST 12: ClearAll
    // ============================================
    PrintSeparator("TEST 12: ClearAll Subscriptions");

    std::cout << "Before ClearAll:\n";
    std::cout << "  Damage: " << bus.GetSubscriberCount(EventType::Damage)
              << " subs\n";
    std::cout << "  LevelUp: " << bus.GetSubscriberCount(EventType::LevelUp)
              << " subs\n";
    std::cout << "  PlayerHit: " << bus.GetSubscriberCount(EventType::PlayerHit)
              << " subs\n";
    std::cout << "  GameOver: " << bus.GetSubscriberCount(EventType::GameOver)
              << " subs\n";

    bus.ClearAll();

    std::cout << "\nAfter ClearAll:\n";
    std::cout << "  Damage: " << bus.GetSubscriberCount(EventType::Damage)
              << " subs\n";
    std::cout << "  LevelUp: " << bus.GetSubscriberCount(EventType::LevelUp)
              << " subs\n";
    std::cout << "  PlayerHit: " << bus.GetSubscriberCount(EventType::PlayerHit)
              << " subs\n";
    std::cout << "  GameOver: " << bus.GetSubscriberCount(EventType::GameOver)
              << " subs\n";

    std::cout << "\nAttempting to trigger events after ClearAll:\n";
    bus.Notify(EventType::Damage, DamagePayload{&player1, 50});
    bus.Notify(EventType::LevelUp, XPPayload{&player1, 50});
    bus.Notify(EventType::GameOver);
    std::cout << "(No handlers called - all subscriptions cleared)\n";

    // ============================================
    // TEST 13: Unsubscribe non-existent
    // ============================================
    PrintSeparator("TEST 13: Edge Cases");

    std::cout << "Unsubscribe with invalid ID: "
              << (bus.Unsubscribe(EventType::Damage, 99999)
                      ? "SUCCESS"
                      : "FAILED (expected)")
              << "\n";

    std::cout << "Unsubscribe from empty event: "
              << (bus.Unsubscribe(EventType::Damage, dmg_sub_p1)
                      ? "SUCCESS"
                      : "FAILED (expected)")
              << "\n";

    std::cout << "GetSubscriberCount for unused event: "
              << bus.GetSubscriberCount(EventType::ItemPickup) << "\n";

    std::cout << std::string(50, '=') << "\n\n";

    return 0;
}