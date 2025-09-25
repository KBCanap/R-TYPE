#pragma once
#include <string>
#include <any>

struct Event {
    std::string name;
    std::any payload; // optional data
};
