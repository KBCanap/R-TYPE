/**
 * Observer Design Pattern
 *
 * Intent: Lets you define a subscription mechanism to notify multiple objects
 * about any events that happen to the object they're observing.
 *
 * Note that there's a lot of different terms with similar meaning associated
 * with this pattern. Just remember that the Subject is also called the
 * Publisher and the Observer is often called the Subscriber and vice versa.
 * Also the verbs "observe", "listen" or "track" usually mean the same thing.
 */

#include <iostream>
#include <list>
#include <string>

class IEvent {
  public:
    virtual ~IEvent() = default;
    virtual std::string GetType() const = 0;
};

class EventCollision : public IEvent {
  public:
    EventCollision(const std::string &a, const std::string &b)
        : entity_a_(a), entity_b_(b) {}

    std::string GetType() const override { return "Collision"; }
    std::string entity_a_;
    std::string entity_b_;
};

class IObserver {
  public:
    virtual ~IObserver() {};
    virtual void Update(const std::string &message_from_subject) = 0;
    virtual void OnEvent(const IEvent &event) = 0;
};

class ISubject {
  public:
    virtual ~ISubject() {};
    virtual void Attach(IObserver *observer) = 0;
    virtual void Detach(IObserver *observer) = 0;
    virtual void Notify() = 0;
};

class Entity : public IObserver {
  public:
    Entity(const std::string &name) : name_(name) {}

    void OnEvent(const IEvent &event) override {
        if (event.GetType() == "Collision") {
            const EventCollision &collision =
                static_cast<const EventCollision &>(event);
            std::cout << "Entity " << name_ << " received collision between "
                      << collision.entity_a_ << " and " << collision.entity_b_
                      << "\n";
        }
    }

    std::string name_;
    int pos = 0; // Example position
};

/**
 * The Subject owns some important state and notifies observers when the state
 * changes.
 */

class Subject : public ISubject {
  public:
    virtual ~Subject() { std::cout << "Goodbye, I was the Subject.\n"; }
    int GenerateObserverID() { return ++observer_id_counter_; }

    /**
     * The subscription management methods.
     */
    void Attach(IObserver *observer) override {
        list_observer_.push_back(observer);
    }
    void Detach(IObserver *observer) override {
        list_observer_.remove(observer);
    }
    void Notify() override {
        std::list<IObserver *>::iterator iterator = list_observer_.begin();
        HowManyObserver();
        while (iterator != list_observer_.end()) {
            (*iterator)->Update(message_);
            ++iterator;
        }
    }

    void CreateMessage(std::string message = "Empty") {
        this->message_ = message;
        Notify();
    }
    void HowManyObserver() {
        std::cout << "There are " << list_observer_.size()
                  << " observers in the list.\n";
    }

    /**
     * Usually, the subscription logic is only a fraction of what a Subject can
     * really do. Subjects commonly hold some important business logic, that
     * triggers a notification method whenever something important is about to
     * happen (or after it).
     */
    void SomeBusinessLogic() {
        this->message_ = "change message message";
        Notify();
        std::cout << "I'm about to do some thing important\n";
    }

  private:
    std::list<IObserver *> list_observer_;
    std::string message_;
    int observer_id_counter_ = 0;
};

class Observer : public IObserver {
  public:
    Observer(Subject &subject)
        : subject_(subject), number_(subject.GenerateObserverID()) {
        subject_.Attach(this);
        std::cout << "Hi, I'm the Observer \"" << number_ << "\".\n";
    }
    virtual ~Observer() {
        std::cout << "Goodbye, I was the Observer \"" << this->number_
                  << "\".\n";
    }

    void Update(const std::string &message_from_subject) override {
        message_from_subject_ = message_from_subject;
        PrintInfo();
    }
    void RemoveMeFromTheList() {
        subject_.Detach(this);
        std::cout << "Observer \"" << number_ << "\" removed from the list.\n";
    }
    void PrintInfo() {
        std::cout << "Observer \"" << this->number_
                  << "\": a new message is available --> "
                  << this->message_from_subject_ << "\n";
    }

  private:
    std::string message_from_subject_;
    Subject &subject_;
    static int static_number_;
    int number_;
};
