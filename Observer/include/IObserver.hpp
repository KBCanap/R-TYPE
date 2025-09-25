#pragma once
#include "Event.hpp"

class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void OnEvent(const Event& event) = 0;
};
