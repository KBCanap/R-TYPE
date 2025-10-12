/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** NetServer
*/

#include <asio.hpp>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct ClientMessage {
    std::string client_endpoint;
    std::string message;
    uint32_t client_id;
};

struct ClientConnection {
    uint32_t id;
    std::shared_ptr<asio::ip::tcp::socket> socket;
    std::string endpoint;
    bool is_connected;
};

class TCPServer {
  public:
    TCPServer(uint16_t port);
    ~TCPServer();

    std::vector<ClientMessage> poll();
    bool sendToClient(uint32_t client_id, const std::string &message);
    void disconnectClient(uint32_t client_id);
    std::vector<uint32_t> getConnectedClients();

  private:
    void start_accept();
    void start_read(std::shared_ptr<ClientConnection> client);
    uint32_t generateClientId();
    std::string getEndpointString(const asio::ip::tcp::socket &socket);

    asio::io_context ctx_;
    asio::ip::tcp::acceptor acceptor_;
    std::thread thread_;
    std::mutex mutex_;
    std::deque<ClientMessage> messages_;

    std::unordered_map<uint32_t, std::shared_ptr<ClientConnection>> clients_;
    uint32_t next_client_id_;
};

class UDPServer {
  public:
    UDPServer(uint16_t port);
    ~UDPServer();

    std::vector<std::string> poll();

  private:
    void start_receive();

    asio::io_context ctx_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_;
    std::array<char, 1024> buffer_;
    std::thread thread_;
    std::mutex mutex_;
    std::deque<std::string> messages_;
};
