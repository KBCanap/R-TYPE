#ifndef OBSERVER_LIBRARY_H
#define OBSERVER_LIBRARY_H

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace ObserverLib {

class Event {
  public:
    virtual ~Event() = default;
    virtual std::type_index getType() const = 0;

    template <typename T>
    bool isType() const {
        return getType() == std::type_index(typeid(T));
    }

    template <typename T>
    const T *as() const {
        return isType<T>() ? static_cast<const T *>(this) : nullptr;
    }
};

template <typename T>
class TypedEvent : public Event {
  public:
    explicit TypedEvent(const T &data) : data_(data) {}
    std::type_index getType() const override {
        return std::type_index(typeid(T));
    }
    const T &getData() const { return data_; }

  private:
    T data_;
};

class Observer {
  public:
    virtual ~Observer() = default;
    virtual void onEvent(const Event &event) = 0;
    virtual void onNotify(class Subject *subject, const std::any &data) {}
    virtual std::size_t getId() const;
};

class Subject {
  public:
    virtual ~Subject() = default;
    virtual void addObserver(std::shared_ptr<Observer> observer);
    virtual void removeObserver(std::shared_ptr<Observer> observer);
    virtual void removeObserver(Observer *observer);
    virtual void clearObservers();
    virtual void notifyObservers(const Event &event);
    virtual void notifyObservers(const std::any &data = {});
    template <typename T>
    void notifyObservers(const T &data) {
        notifyObservers(TypedEvent<T>(data));
    }
    std::size_t getObserverCount() const;

  protected:
    mutable std::mutex observers_mutex_;
    std::vector<std::weak_ptr<Observer>> observers_;
};

class FunctionObserver : public Observer {
  public:
    using EventCallback = std::function<void(const Event &)>;
    using NotifyCallback = std::function<void(Subject *, const std::any &)>;
    FunctionObserver(EventCallback cb);
    FunctionObserver(NotifyCallback cb);
    FunctionObserver(EventCallback ecb, NotifyCallback ncb);
    void onEvent(const Event &event) override;
    void onNotify(Subject *subject, const std::any &data) override;

  private:
    EventCallback event_callback_;
    NotifyCallback notify_callback_;
};

class EventDispatcher {
  public:
    static EventDispatcher &getInstance();
    void subscribe(const std::string &eventType,
                   std::shared_ptr<Observer> observer);
    void unsubscribe(const std::string &eventType,
                     std::shared_ptr<Observer> observer);
    void unsubscribe(const std::string &eventType, Observer *observer);
    void dispatch(const std::string &eventType, const Event &event);
    void dispatch(const std::string &eventType, const std::any &data = {});
    template <typename T>
    void dispatch(const std::string &eventType, const T &data) {
        dispatch(eventType, TypedEvent<T>(data));
    }
    void clearSubscriptions(const std::string &eventType);
    void clearAllSubscriptions();

  private:
    mutable std::mutex dispatcher_mutex_;
    std::unordered_map<std::string, std::vector<std::weak_ptr<Observer>>>
        event_subscribers_;
};

} // namespace ObserverLib

extern "C" {
ObserverLib::Subject *create_subject();
void destroy_subject(ObserverLib::Subject *subject);
ObserverLib::Observer *
create_function_observer(void (*callback)(const ObserverLib::Event *));
void destroy_observer(ObserverLib::Observer *observer);
void subject_add_observer(ObserverLib::Subject *subject,
                          ObserverLib::Observer *observer);
void subject_remove_observer(ObserverLib::Subject *subject,
                             ObserverLib::Observer *observer);
void subject_notify(ObserverLib::Subject *subject);
void subject_notify_with_data(ObserverLib::Subject *subject, const void *data,
                              size_t size);
ObserverLib::EventDispatcher *get_event_dispatcher();
void dispatcher_subscribe(ObserverLib::EventDispatcher *dispatcher,
                          const char *eventType,
                          ObserverLib::Observer *observer);
void dispatcher_dispatch(ObserverLib::EventDispatcher *dispatcher,
                         const char *eventType);
}

#endif // OBSERVER_LIBRARY_H
