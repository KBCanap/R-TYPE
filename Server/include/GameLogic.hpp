/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** GameLogic
*/

#ifndef GAMELOGIC_COMPLETE_HPP_
#define GAMELOGIC_COMPLETE_HPP_

#include "../../ecs/include/registery.hpp"
#include "../../ecs/include/components.hpp"
#include "../../ecs/include/systems.hpp"
#include "../../ecs/include/network/NetworkComponents.hpp"
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// Input events
enum InputEvent {
    KEY_UP_PRESS,
    KEY_UP_RELEASE,
    KEY_DOWN_PRESS,
    KEY_DOWN_RELEASE,
    KEY_LEFT_PRESS,
    KEY_LEFT_RELEASE,
    KEY_RIGHT_PRESS,
    KEY_RIGHT_RELEASE,
    KEY_SHOOT_PRESS,
    KEY_SHOOT_RELEASE
};

// Event structure with timestamp and sequence
struct ClientEvent {
    uint client_id;
    InputEvent action;
    uint sequence_number;
    std::chrono::steady_clock::time_point timestamp;
};

// Entity state snapshot for network sync
struct EntitySnapshot {
    uint net_id;
    std::string entity_type;
    component::position pos;
    component::velocity vel;
    int health;
    int score;
    uint tick;
    bool synced;
};

struct WorldSnapshot {
    uint tick;
    std::chrono::steady_clock::time_point timestamp;
    std::vector<EntitySnapshot> entities;
};

/**
 * @brief Class to handle the game logic for R-Type multiplayer server
 */
class GameLogic {
  public:
    GameLogic(std::shared_ptr<registry> reg);
    ~GameLogic();

    // Game loop control
    void start();
    void stop();
    void update(float deltaTime);
    void printEntityPositions();

    // Event handling from network thread
    void pushClientEvent(const ClientEvent &evt);
    void updatePlayerInputState(uint client_id, uint8_t direction);

    // Snapshot management
    WorldSnapshot generateSnapshot();
    std::vector<EntitySnapshot> getDeltaSnapshot(uint last_acked_tick);
    void markEntitiesSynced(const std::vector<uint32_t> &net_ids) ;

    // Player management
    entity createPlayer(uint client_id, uint net_id, float x, float y);
    void removePlayer(uint client_id);
    entity getPlayerEntity(uint client_id);

    // Getters
    uint getCurrentTick() const { return _current_tick; }
    uint generateNetId();

    bool isEntitySynced(uint net_id);

  private:
    std::unordered_map<uint, std::chrono::steady_clock::time_point> _last_input_time;

    std::shared_ptr<registry> _registry;
    std::queue<ClientEvent> _event_queue;
    std::mutex _event_mutex;
    std::unordered_map<uint, entity> _client_to_entity;
    bool _running;

    // Game tick tracking for sync
    uint _current_tick;
    std::chrono::steady_clock::time_point _last_update;

    // Snapshot history
    static const size_t MAX_SNAPSHOT_HISTORY = 128;
    std::deque<WorldSnapshot> _snapshot_history;

    // Game state
    float _game_time;
    float _enemy_spawn_timer;
    float _enemy_spawn_interval;
    int _total_score;
    bool _boss_spawned;
    entity _boss;
    std::vector<entity> _enemies;
    std::vector<entity> _projectiles;

    // Random number generation
    std::mt19937 _rng;
    uint _next_net_id;

    // Logic methods
    void processEvents();
    void handleEvent(const ClientEvent &evt);
    void registerSystems();
    void handlePlayerAction(entity player, InputEvent action);

    // Game logic
    void spawnEnemy();
    void spawnBoss();
    void cleanupDeadEntities();
    entity findEntityByNetId(uint net_id);

};

using namespace render;
class DummyRenderWindow : public render::IRenderWindow {
    public:
        DummyRenderWindow(unsigned int x, unsigned int y) : _x(x), _y(y) {}
        ~DummyRenderWindow() override = default;

    private:
        unsigned int _x;
        unsigned int _y;

    // Window management
    bool isOpen() const override { return 0;}
    void close() override {};
    void clear([[maybe_unused]]const Color &color = Color::Black()) override {};
    void display() override {};
    Vector2u getSize() const override {
            return Vector2u(_x, _y);
        }
    void setSize([[maybe_unused]]const Vector2u &size) override {};
    void setFramerateLimit([[maybe_unused]]unsigned int limit) override {};
    void setVerticalSyncEnabled([[maybe_unused]]bool enabled) override {};
    void setTitle([[maybe_unused]]const std::string &title) override {};

    // Event handling
    bool pollEvent([[maybe_unused]]Event &event) override { return 0;};

    // Drawing
    void draw([[maybe_unused]]ISprite &sprite) override {};
    void draw([[maybe_unused]]IShape &shape) override {};
    void draw([[maybe_unused]]IText &text) override {};
    void draw([[maybe_unused]]ISprite &sprite, [[maybe_unused]]IShader &shader) override {};
    void draw([[maybe_unused]]IShape &shape, [[maybe_unused]]IShader &shader) override {};
    void draw([[maybe_unused]]IText &text, [[maybe_unused]]IShader &shader) override {};

    // View management
    void setView([[maybe_unused]]IView &view) override {};
    std::unique_ptr<IView> getDefaultView() const override { return nullptr;};
    std::unique_ptr<IView> createView() override {return nullptr;};

    // Factory methods for creating drawable objects
    std::unique_ptr<ISprite> createSprite() override {return nullptr;};
    std::unique_ptr<ITexture> createTexture() override {return nullptr;};
    std::unique_ptr<IShape>
    createRectangleShape([[maybe_unused]]const Vector2f &size) override {return nullptr;};
    std::unique_ptr<IShape> createCircleShape([[maybe_unused]]float radius) override {return nullptr;};
    std::unique_ptr<IFont> createFont() override {return nullptr;};
    std::unique_ptr<IText> createText() override {return nullptr;};
    std::unique_ptr<IShader> createShader() override {return nullptr;};
    std::unique_ptr<IImage> createImage() override {return nullptr;};
};

#endif /* !GAMELOGIC_COMPLETE_HPP_ */