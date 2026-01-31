#include "../include/network/ASIOSocket.hpp"
#include <iostream>

namespace network {

ASIOTCPSocket::ASIOTCPSocket(asio::io_context &io_context)
    : socket_(io_context) {}

ASIOTCPSocket::~ASIOTCPSocket() { disconnect(); }

bool ASIOTCPSocket::connect(const std::string &host, uint16_t port) {
    try {
        asio::ip::tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, std::to_string(port));
        asio::connect(socket_, endpoints);
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

void ASIOTCPSocket::disconnect() {
    if (socket_.is_open()) {
        try {
            socket_.close();
        } catch (...) {
        }
    }
}

bool ASIOTCPSocket::isOpen() const { return socket_.is_open(); }

size_t ASIOTCPSocket::send(const std::vector<uint8_t> &data) {
    try {
        return asio::write(socket_, asio::buffer(data));
    } catch (const std::exception &e) {
        return 0;
    }
}

void ASIOTCPSocket::asyncRead(std::vector<uint8_t> &buffer,
                              std::function<void(bool, size_t)> callback) {
    socket_.async_read_some(
        asio::buffer(buffer),
        [callback](std::error_code ec, std::size_t bytes_transferred) {
            callback(!ec, bytes_transferred);
        });
}

void ASIOTCPSocket::asyncReadExactly(
    std::vector<uint8_t> &buffer, size_t size,
    std::function<void(bool, size_t)> callback) {
    if (buffer.size() < size) {
        buffer.resize(size);
    }

    asio::async_read(
        socket_, asio::buffer(buffer.data(), size),
        [callback](std::error_code ec, std::size_t bytes_transferred) {
            callback(!ec, bytes_transferred);
        });
}

ASIOUDPSocket::ASIOUDPSocket(asio::io_context &io_context)
    : socket_(io_context) {}

ASIOUDPSocket::~ASIOUDPSocket() { close(); }

bool ASIOUDPSocket::open() {
    try {
        socket_.open(asio::ip::udp::v4());
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

void ASIOUDPSocket::close() {
    if (socket_.is_open()) {
        try {
            socket_.close();
        } catch (...) {
        }
    }
}

bool ASIOUDPSocket::isOpen() const { return socket_.is_open(); }

bool ASIOUDPSocket::setRemoteEndpoint(const std::string &host, uint16_t port) {
    try {
        asio::ip::udp::resolver resolver(socket_.get_executor());
        auto endpoints =
            resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port));
        remote_endpoint_ = *endpoints.begin();
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

size_t ASIOUDPSocket::sendTo(const std::vector<uint8_t> &data) {
    try {
        return socket_.send_to(asio::buffer(data), remote_endpoint_);
    } catch (const std::exception &e) {
        return 0;
    }
}

void ASIOUDPSocket::asyncReceive(std::vector<uint8_t> &buffer,
                                 std::function<void(bool, size_t)> callback) {
    socket_.async_receive(
        asio::buffer(buffer),
        [callback](std::error_code ec, std::size_t bytes_transferred) {
            callback(!ec, bytes_transferred);
        });
}

ASIOContext::ASIOContext() {}
ASIOContext::~ASIOContext() { stop(); }
void ASIOContext::run() { io_context_.run(); }
void ASIOContext::stop() { io_context_.stop(); }
ITCPSocket *ASIOContext::createTCPSocket() {
    return new ASIOTCPSocket(io_context_);
}
IUDPSocket *ASIOContext::createUDPSocket() {
    return new ASIOUDPSocket(io_context_);
}

} // namespace network