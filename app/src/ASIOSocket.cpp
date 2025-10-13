#include "../include/network/ASIOSocket.hpp"
#include <iostream>

namespace network {

ASIOTCPSocket::ASIOTCPSocket(asio::io_context &io_context)
    : socket_(io_context) {
    std::cout << "[ASIOTCPSocket] Created" << std::endl;
}

ASIOTCPSocket::~ASIOTCPSocket() {
    std::cout << "[ASIOTCPSocket] Destroying" << std::endl;
    disconnect();
}

bool ASIOTCPSocket::connect(const std::string &host, uint16_t port) {
    try {
        std::cout << "[ASIOTCPSocket] Connecting to " << host << ":" << port
                  << std::endl;

        asio::ip::tcp::resolver resolver(socket_.get_executor());
        auto endpoints = resolver.resolve(host, std::to_string(port));

        asio::connect(socket_, endpoints);

        std::cout << "[ASIOTCPSocket] Connected successfully" << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "[ASIOTCPSocket] Connect error: " << e.what() << std::endl;
        return false;
    }
}

void ASIOTCPSocket::disconnect() {
    if (socket_.is_open()) {
        try {
            std::cout << "[ASIOTCPSocket] Disconnecting" << std::endl;

            asio::error_code ec;
            socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            if (ec) {
                std::cerr << "[ASIOTCPSocket] Shutdown error: " << ec.message()
                          << std::endl;
            }

            socket_.close(ec);
            if (ec) {
                std::cerr << "[ASIOTCPSocket] Close error: " << ec.message()
                          << std::endl;
            }

            std::cout << "[ASIOTCPSocket] Disconnected" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "[ASIOTCPSocket] Disconnect exception: " << e.what()
                      << std::endl;
        }
    }
}

bool ASIOTCPSocket::isOpen() const { return socket_.is_open(); }

size_t ASIOTCPSocket::send(const std::vector<uint8_t> &data) {
    try {
        std::cout << "[ASIOTCPSocket] Sending " << data.size() << " bytes"
                  << std::endl;

        size_t sent = asio::write(socket_, asio::buffer(data));

        std::cout << "[ASIOTCPSocket] Sent " << sent << " bytes" << std::endl;
        return sent;
    } catch (const std::exception &e) {
        std::cerr << "[ASIOTCPSocket] Send error: " << e.what() << std::endl;
        return 0;
    }
}

void ASIOTCPSocket::asyncRead(std::vector<uint8_t> &buffer,
                              std::function<void(bool, size_t)> callback) {
    //std::cout << "[ASIOTCPSocket] Starting async_read_some (buffer size: "
    //          << buffer.size() << ")" << std::endl;

    socket_.async_read_some(
        asio::buffer(buffer),
        [callback](std::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "[ASIOTCPSocket] async_read_some error: "
                          << ec.message() << std::endl;
            } else {
                std::cout << "[ASIOTCPSocket] async_read_some received "
                          << bytes_transferred << " bytes" << std::endl;
            }
            callback(!ec, bytes_transferred);
        });
}

void ASIOTCPSocket::asyncReadExactly(
    std::vector<uint8_t> &buffer, size_t size,
    std::function<void(bool, size_t)> callback) {

    std::cout << "[ASIOTCPSocket] Starting async_read (exactly " << size
              << " bytes)" << std::endl;

    // Ensure buffer has enough space
    if (buffer.size() < size) {
        std::cout << "[ASIOTCPSocket] Resizing buffer from " << buffer.size()
                  << " to " << size << std::endl;
        buffer.resize(size);
    }

    asio::async_read(
        socket_, asio::buffer(buffer.data(), size),
        [callback, size](std::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                std::cerr << "[ASIOTCPSocket] async_read error: "
                          << ec.message() << " (code: " << ec.value() << ")"
                          << std::endl;
            } else {
                std::cout << "[ASIOTCPSocket] async_read completed: "
                          << bytes_transferred << "/" << size << " bytes"
                          << std::endl;
            }

            bool success = !ec && bytes_transferred == size;
            callback(success, bytes_transferred);
        });
}

ASIOUDPSocket::ASIOUDPSocket(asio::io_context &io_context)
    : socket_(io_context) {
    //std::cout << "[ASIOUDPSocket] Created" << std::endl;
}

ASIOUDPSocket::~ASIOUDPSocket() {
    //std::cout << "[ASIOUDPSocket] Destroying" << std::endl;
    close();
}

bool ASIOUDPSocket::open() {
    try {
        std::cout << "[ASIOUDPSocket] Opening UDP socket" << std::endl;
        socket_.open(asio::ip::udp::v4());
        std::cout << "[ASIOUDPSocket] UDP socket opened" << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "[ASIOUDPSocket] Open error: " << e.what() << std::endl;
        return false;
    }
}

void ASIOUDPSocket::close() {
    if (socket_.is_open()) {
        try {
            std::cout << "[ASIOUDPSocket] Closing" << std::endl;

            asio::error_code ec;
            socket_.close(ec);

            if (ec) {
                std::cerr << "[ASIOUDPSocket] Close error: " << ec.message()
                          << std::endl;
            } else {
                std::cout << "[ASIOUDPSocket] Closed successfully" << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "[ASIOUDPSocket] Close exception: " << e.what()
                      << std::endl;
        }
    }
}

bool ASIOUDPSocket::isOpen() const { return socket_.is_open(); }

bool ASIOUDPSocket::setRemoteEndpoint(const std::string &host, uint16_t port) {
    try {
        std::cout << "[ASIOUDPSocket] Setting remote endpoint to " << host
                  << ":" << port << std::endl;

        asio::ip::udp::resolver resolver(socket_.get_executor());
        auto endpoints =
            resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port));
        remote_endpoint_ = *endpoints.begin();

        //std::cout << "[ASIOUDPSocket] Remote endpoint set successfully"
        //          << std::endl;
        return true;
    } catch (const std::exception &e) {
        //std::cerr << "[ASIOUDPSocket] Set endpoint error: " << e.what()
        //          << std::endl;
        return false;
    }
}

size_t ASIOUDPSocket::sendTo(const std::vector<uint8_t> &data) {
    try {
        std::cout << "[ASIOUDPSocket] Sending " << data.size() << " bytes to "
                  << remote_endpoint_.address().to_string() << ":"
                  << remote_endpoint_.port() << std::endl;

        size_t sent = socket_.send_to(asio::buffer(data), remote_endpoint_);

        std::cout << "[ASIOUDPSocket] Sent " << sent << " bytes" << std::endl;
        return sent;
    } catch (const std::exception &e) {
        std::cerr << "[ASIOUDPSocket] Send error: " << e.what() << std::endl;
        return 0;
    }
}

void ASIOUDPSocket::asyncReceive(std::vector<uint8_t> &buffer,
                                 std::function<void(bool, size_t)> callback) {
    std::cout << "[ASIOUDPSocket] Starting async_receive (buffer size: "
              << buffer.size() << ")" << std::endl;

    socket_.async_receive(
        asio::buffer(buffer),
        [callback](std::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                //std::cerr << "[ASIOUDPSocket] async_receive error: "
                //          << ec.message() << std::endl;
            } else {
                //std::cout << "[ASIOUDPSocket] async_receive got "
                //          << bytes_transferred << " bytes" << std::endl;
            }
            callback(!ec, bytes_transferred);
        });
}

ASIOContext::ASIOContext()
    : work_guard_(std::make_unique<
                  asio::executor_work_guard<asio::io_context::executor_type>>(
          asio::make_work_guard(io_context_))) {
    // std::cout << "[ASIOContext] Created with work guard" << std::endl;
}

ASIOContext::~ASIOContext() {
    // std::cout << "[ASIOContext] Destroying" << std::endl;
    stop();
}

void ASIOContext::run() {
    std::cout << "[ASIOContext] Starting io_context::run()" << std::endl;

    try {
        io_context_.run();
        // std::cout << "[ASIOContext] io_context::run() completed" <<
        // std::endl;
    } catch (const std::exception &e) {
        // std::cerr << "[ASIOContext] run() exception: " << e.what() <<
        // std::endl;
        throw;
    }
}

void ASIOContext::stop() {
    std::cout << "[ASIOContext] Stopping io_context" << std::endl;

    if (work_guard_) {
        work_guard_.reset();
        // std::cout << "[ASIOContext] Work guard released" << std::endl;
    }

    if (!io_context_.stopped()) {
        io_context_.stop();
        // std::cout << "[ASIOContext] io_context stopped" << std::endl;
    }
}

ITCPSocket *ASIOContext::createTCPSocket() {
    std::cout << "[ASIOContext] Creating TCP socket" << std::endl;
    return new ASIOTCPSocket(io_context_);
}

IUDPSocket *ASIOContext::createUDPSocket() {
    std::cout << "[ASIOContext] Creating UDP socket" << std::endl;
    return new ASIOUDPSocket(io_context_);
}

} // namespace network