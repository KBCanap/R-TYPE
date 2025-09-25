#pragma once
#include <string>
#include <any>
#include <unordered_map>
#include <vector>
#include <functional>

class EventBus {
public:
    using Callback = std::function<void(const std::any&)>;

    void Subscribe(const std::string& event_name, Callback callback) {
        subscribers[event_name].push_back(callback);
    }

    void Notify(const std::string& event_name, const std::any& payload = {}) {
        for (const auto& callback : subscribers[event_name]) {
            callback(payload);
        }
    }

private:
    std::unordered_map<std::string, std::vector<Callback>> subscribers;
};
