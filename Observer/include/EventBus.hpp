#pragma once
#include "Event.hpp"
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>

/**
 * @brief Class to handle event manager
 *
 */
class EventBus {
  public:
    using Callback = std::function<void(const std::any &)>;
    using SubscriptionID = size_t;

  private:
    struct Subscription {
        SubscriptionID id;
        Callback callback;
    };

    std::unordered_map<EventType, std::vector<Subscription>> subscribers;
    SubscriptionID next_id = 0;

  public:
    /**
     * @brief Subscribe and return id for unsubscribing
     *
     * @param event_type
     * @param callback
     * @return SubscriptionID
     */
    SubscriptionID Subscribe(EventType event_type, Callback callback) {
        SubscriptionID id = next_id++;
        subscribers[event_type].push_back({id, std::move(callback)});
        return id;
    }

    /**
     * @brief Unsubscribe using the subscription ID
     *
     * @param event_type
     * @param id
     * @return true
     * @return false
     */
    bool Unsubscribe(EventType event_type, SubscriptionID id) {
        auto it = subscribers.find(event_type);
        if (it == subscribers.end())
            return false;

        auto &subs = it->second;
        auto sub_it = std::find_if(
            subs.begin(), subs.end(),
            [id](const Subscription &sub) { return sub.id == id; });

        if (sub_it != subs.end()) {
            subs.erase(sub_it);
            if (subs.empty()) {
                subscribers.erase(it);
            }
            return true;
        }
        return false;
    }

    /**
     * @brief Notify all subscribers of an event
     *
     * @param event_type
     * @param payload
     */
    void Notify(EventType event_type, const std::any &payload = {}) {
        auto it = subscribers.find(event_type);
        if (it == subscribers.end())
            return;
        auto subs_copy = it->second;
        for (const auto &sub : subs_copy) {
            sub.callback(payload);
        }
    }

    /**
     * @brief Notify using Event struct
     *
     * @param event
     */
    void Notify(const Event &event) { Notify(event.type, event.payload); }

    /**
     * @brief Check if an event has subscribers
     *
     * @param event_type
     * @return true
     * @return false
     */
    bool HasSubscribers(EventType event_type) const {
        auto it = subscribers.find(event_type);
        return it != subscribers.end() && !it->second.empty();
    }

    /**
     * @brief Clear all subscriptions for an event
     *
     * @param event_type
     */
    void ClearEvent(EventType event_type) { subscribers.erase(event_type); }

    /**
     * @brief Clear all subscriptions
     *
     * @warning This function remove all data
     * !! THIS IS A FULL CLEAR OF ALL DATA !!
     */
    void ClearAll() { subscribers.clear(); }

    /**
     * @brief Get the Subscriber Count object
     *
     * @param event_type
     * @return size_t
     */
    // Get subscriber count for an event
    size_t GetSubscriberCount(EventType event_type) const {
        auto it = subscribers.find(event_type);
        return it != subscribers.end() ? it->second.size() : 0;
    }
};
