#include "../include/NetServer.hpp"

TCPServer::TCPServer(uint16_t port)
    : acceptor_(ctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    start_accept();
    thread_ = std::thread([this]() { ctx_.run(); });
}

TCPServer::~TCPServer() {
    ctx_.stop();
    if (thread_.joinable()) thread_.join();
}

std::vector<std::string> TCPServer::poll() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> out(messages_.begin(), messages_.end());
    messages_.clear();
    return out;
}

void TCPServer::start_accept() {
    auto socket = std::make_shared<asio::ip::tcp::socket>(ctx_);
    acceptor_.async_accept(*socket, [this, socket](std::error_code ec) {
        if (!ec) start_read(socket);
        start_accept();
    });
}

void TCPServer::start_read(std::shared_ptr<asio::ip::tcp::socket> socket) {
    auto buf = std::make_shared<std::vector<char>>(1024);
    socket->async_read_some(asio::buffer(*buf), [this, socket, buf](std::error_code ec, std::size_t len) {
        if (!ec) {
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.emplace_back(std::string(buf->data(), len));
            start_read(socket);
        }
    });
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