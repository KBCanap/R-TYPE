#pragma once
#include "Event.hpp"

/**
 * @brief Vitrual class for Observer
 *
 */
class IObserver {
  public:
    virtual ~IObserver() = default;
    virtual void OnEvent(const Event &event) = 0;
};