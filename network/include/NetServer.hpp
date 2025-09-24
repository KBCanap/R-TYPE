/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** NetServer
*/

#include <cstdint>
#include <string>
#include <vector>
#include <asio.hpp>
#include <thread>
#include <mutex>
#include <deque>
#include <memory>

class TCPServer {
    public:
        TCPServer(uint16_t port);
        ~TCPServer();

        std::vector<std::string> poll();

    private:
        void start_accept();
        void start_read(std::shared_ptr<asio::ip::tcp::socket> socket);

        asio::io_context ctx_;
        asio::ip::tcp::acceptor acceptor_;
        std::thread thread_;
        std::mutex mutex_;
        std::deque<std::string> messages_;
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
