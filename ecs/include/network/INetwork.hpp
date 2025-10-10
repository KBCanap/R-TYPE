#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <optional>

namespace network {

/**
 * @enum ConnectionState
 * @brief Client connection states per RFC
 */
enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    WAITING,
    GAME_STARTING,
    IN_GAME,
    ERROR
};

/**
 * @enum MessageType
 * @brief TCP message types from RFC
 */
enum class MessageType : uint8_t {
    TCP_CONNECT = 0x01,
    TCP_CONNECT_ACK = 0x02,
    TCP_CONNECT_NAK = 0x03,
    TCP_READY = 0x04,
    TCP_GAME_START = 0x05,
    TCP_ERROR = 0xFF
};

/**
 * @enum UDPMessageType
 * @brief UDP message types from RFC
 */
enum class UDPMessageType : uint8_t {
    CLIENT_PING = 0x00,
    PLAYER_ASSIGNMENT = 0x01,
    ENTITY_CREATE = 0x10,
    ENTITY_UPDATE = 0x11,
    ENTITY_DESTROY = 0x12,
    GAME_STATE = 0x13,
    PLAYER_INPUT = 0x20
};

/**
 * @enum EntityType
 * @brief Entity types from RFC
 */
enum class EntityType : uint8_t {
    PLAYER = 0x01,
    ENEMY = 0x02,
    PROJECTILE = 0x03,
    ALLIED_PROJECTILE = 0x04
};

/**
 * @enum InputEventType
 * @brief Player input event types from RFC
 */
enum class InputEventType : uint8_t { MOVE = 0x01, SHOOT = 0x02, QUIT = 0x03 };

/**
 * @enum Direction
 * @brief Movement directions from RFC
 */
enum class Direction : uint8_t {
    NONE = 0x00,
    UP = 0x01,       // bit 0
    DOWN = 0x02,     // bit 1
    LEFT = 0x04,     // bit 2
    RIGHT = 0x08     // bit 3
};

/**
 * @enum ErrorCode
 * @brief Error codes from RFC
 */
enum class ErrorCode : uint8_t {
    // Connection errors (pour TCP_CONNECT_NAK)
    GAME_FULL = 0x01,
    GAME_STARTED = 0x02,
    SERVER_ERROR = 0x03,
    // Protocol errors (pour TCP_ERROR)
    PROTOCOL_VIOLATION = 0x04,
    UNEXPECTED_MESSAGE = 0x05,
    TIMEOUT = 0x06
};

/**
 * @struct TCPMessage
 * @brief TCP message structure per RFC
 */
struct TCPMessage {
    MessageType msg_type;
    uint32_t data_length;
    std::vector<uint8_t> data;
};

/**
 * @struct UDPPacket
 * @brief UDP packet with reliability tracking
 */
struct UDPPacket {
    UDPMessageType msg_type;
    uint32_t data_length;
    uint32_t sequence_num;
    std::vector<uint8_t> payload;
};

/**
 * @struct EntityData
 * @brief Entity data from ENTITY_CREATE/GAME_STATE
 */
struct EntityData {
    uint32_t net_id;
    EntityType entity_type;
    uint32_t health;
    float position_x;
    float position_y;
};

/**
 * @struct EntityUpdateData
 * @brief Entity update data from ENTITY_UPDATE
 */
struct EntityUpdateData {
    uint32_t net_id;
    uint32_t health;
    float position_x;
    float position_y;
};

/**
 * @struct PlayerInputData
 * @brief Player input data for PLAYER_INPUT
 */
struct PlayerInputData {
    InputEventType event_type;
    Direction direction;
};

/**
 * @struct ConnectionResult
 * @brief Result of connection attempt
 */
struct ConnectionResult {
    bool success;
    std::string error_message;
    uint8_t player_id;
};

/**
 * @class INetwork
 * @brief Abstract network interface for R-Type client
 */
class INetwork {
  public:
    virtual ~INetwork() = default;

    /**
     * @brief Connect to TCP server
     * @param host Server hostname or IP
     * @param port Server TCP port
     * @return ConnectionResult with success status and player ID
     */
    virtual ConnectionResult connectTCP(const std::string &host,
                                        uint16_t port) = 0;

    /**
     * @brief Disconnect from server
     */
    virtual void disconnect() = 0;

    /**
     * @brief Get current connection state
     * @return Current ConnectionState
     */
    virtual ConnectionState getConnectionState() const = 0;

    /**
     * @brief Check if connected to server
     * @return true if connected
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Send TCP message to server
     * @param msg_type Message type from RFC
     * @param data Optional message payload
     * @return true if sent successfully
     */
    virtual bool sendTCP(MessageType msg_type,
                         const std::vector<uint8_t> &data = {}) = 0;

    /**
     * @brief Send UDP packet to server
     * @param packet UDP packet to send
     * @return true if sent successfully
     */
    virtual bool sendUDP(const UDPPacket &packet) = 0;

    /**
     * @brief Send player input to server
     * @param input Player input data
     * @return true if sent successfully
     */
    virtual bool sendPlayerInput(const PlayerInputData &input) = 0;

    /**
     * @brief Poll TCP messages (non-blocking)
     * @return Vector of received TCP messages
     */
    virtual std::vector<TCPMessage> pollTCP() = 0;

    /**
     * @brief Poll UDP packets (non-blocking)
     * @return Vector of received UDP packets
     */
    virtual std::vector<UDPPacket> pollUDP() = 0;

    /**
     * @brief Check if messages are available
     * @return true if messages in queue
     */
    virtual bool hasMessages() const = 0;

    /**
     * @brief Get assigned player ID
     * @return Player ID (1-4) or 0 if not connected
     */
    virtual uint8_t getPlayerID() const = 0;

    /**
     * @brief Get UDP port for game communication
     * @return UDP port or 0 if not started
     */
    virtual uint16_t getUDPPort() const = 0;

    /**
     * @brief Get network latency estimate
     * @return Latency in milliseconds
     */
    virtual float getLatency() const = 0;

    /**
     * @brief Update network state (call per frame)
     * @param dt Delta time
     */
    virtual void update(float dt) = 0;
};

} // namespace network