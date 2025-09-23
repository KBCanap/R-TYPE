#pragma once
#include <cstddef>

class entity {
private:
    std::size_t _id;
    
public:
    explicit entity(std::size_t id) : _id(id) {}
    
    operator std::size_t() const { return _id; }
    
    bool operator==(const entity& other) const { return _id == other._id; }
    bool operator!=(const entity& other) const { return _id != other._id; }
};
