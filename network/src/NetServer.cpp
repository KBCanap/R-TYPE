#include "../include/NetServer.hpp"
#include <iostream>

TCPServer::TCPServer(uint16_t port)
    : acceptor_(ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), 
      next_client_id_(1) {
    start_accept();
    thread_ = std::thread([this]() { ctx_.run(); });
}

TCPServer::~TCPServer() {
    ctx_.stop();
    if (thread_.joinable()) thread_.join();
}

std::vector<ClientMessage> TCPServer::poll()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClientMessage> out(messages_.begin(), messages_.end());
    messages_.clear();
    return out;
}

void TCPServer::start_accept()
{
    auto socket = std::make_shared<asio::ip::tcp::socket>(ctx_);

    acceptor_.async_accept(*socket, [this, socket](std::error_code ec) {
        if (!ec) {
            uint32_t client_id = generateClientId();
            std::string endpoint = getEndpointString(*socket);
            
            auto client = std::make_shared<ClientConnection>();
            client->id = client_id;
            client->socket = socket;
            client->endpoint = endpoint;
            client->is_connected = true;
            
            {
                std::lock_guard<std::mutex> lock(mutex_);
                clients_[client_id] = client;
            }

            start_read(client);
        }
        start_accept();
    });
}

void TCPServer::start_read(std::shared_ptr<ClientConnection> client)
{
    auto buf = std::make_shared<std::vector<char>>(1024);

    client->socket->async_read_some(asio::buffer(*buf), 
        [this, client, buf](std::error_code ec, std::size_t len) {
            if (!ec) {
                ClientMessage msg;
                msg.client_id = client->id;
                msg.client_endpoint = client->endpoint;
                msg.message = std::string(buf->data(), len);
                
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    messages_.emplace_back(msg);
                }
                
                start_read(client);
            } else {
                std::lock_guard<std::mutex> lock(mutex_);
                client->is_connected = false;
                clients_.erase(client->id);
            }
        });
}

bool TCPServer::sendToClient(uint32_t client_id, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end() && it->second->is_connected) {
        try {
            asio::write(*(it->second->socket), asio::buffer(message));
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error sending to client " << client_id << ": " << e.what() << std::endl;
            it->second->is_connected = false;
            clients_.erase(client_id);
            return false;
        }
    }
    return false;
}

void TCPServer::disconnectClient(uint32_t client_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        try {
            it->second->socket->close();
        } catch (...) {
        }
        it->second->is_connected = false;
        clients_.erase(client_id);
    }
}

std::vector<uint32_t> TCPServer::getConnectedClients()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint32_t> client_ids;
    for (const auto& pair : clients_) {
        if (pair.second->is_connected) {
            client_ids.push_back(pair.first);
        }
    }
    return client_ids;
}

uint32_t TCPServer::generateClientId()
{
    return next_client_id_++;
}

std::string TCPServer::getEndpointString(const asio::ip::tcp::socket& socket)
{
    try {
        auto remote = socket.remote_endpoint();
        return remote.address().to_string() + ":" + std::to_string(remote.port());
    } catch (const std::exception&) {
        return "unknown";
    }
}

UDPServer::UDPServer(uint16_t port)
    : socket_(ctx_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
    start_receive();
    thread_ = std::thread([this]() { ctx_.run(); });
}

UDPServer::~UDPServer() {
    ctx_.stop();
    if (thread_.joinable()) thread_.join();
}

std::vector<std::string> UDPServer::poll() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> out(messages_.begin(), messages_.end());
    messages_.clear();
    return out;
}

void UDPServer::start_receive() {
    socket_.async_receive_from(asio::buffer(buffer_), remote_,
        [this](std::error_code ec, std::size_t len) {
            if (!ec) {
                std::lock_guard<std::mutex> lock(mutex_);
                messages_.emplace_back(std::string(buffer_.data(), len));
            }
            start_receive();
        });
}