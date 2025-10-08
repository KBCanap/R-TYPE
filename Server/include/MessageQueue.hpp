/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** MessageQueue - Thread-safe queue pour les messages clients
*/

#ifndef MESSAGEQUEUE_HPP_
#define MESSAGEQUEUE_HPP_
#include "UdpProtocole.hpp"
#include <memory>
#include <queue>

struct ClientMessage {
    uint32_t client_id;
    std::string client_endpoint;
    ParsedUdpMessage parsed_message;
};

class MessageQueue {
public:
    MessageQueue() = default;
    ~MessageQueue() = default;

    void push(const ClientMessage& message) {
        _queue.push(std::make_shared<ClientMessage>(message));
    }

    std::shared_ptr<ClientMessage> pop() {
        if (_queue.empty()) {
            return nullptr;
        }
        auto message = _queue.front();
        _queue.pop();
        return message;
    }

    bool empty() const {
        return _queue.empty();
    }

    size_t size() const {
        return _queue.size();
    }

    void clear() {
        while (!_queue.empty()) {
            _queue.pop();
        }
    }

private:
    std::queue<std::shared_ptr<ClientMessage>> _queue;
};

#endif /* !MESSAGEQUEUE_HPP_ */
