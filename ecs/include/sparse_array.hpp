#pragma once
#include <vector>
#include <optional>

template<typename Component>
class sparse_array {
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type&;
    using const_reference_type = const value_type&;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;

private:
    container_t _data;

public:
    sparse_array() = default;

    reference_type operator[](size_t idx) {
        if (idx >= _data.size()) _data.resize(idx + 1);
        return _data[idx];
    }

    const_reference_type operator[](size_t idx) const {
        if (idx >= _data.size()) {
            static const value_type empty;
            return empty;
        }
        return _data[idx];
    }

    template<class... Args>
    reference_type emplace_at(size_type pos, Args&&... args) {
        if (pos >= _data.size()) _data.resize(pos + 1);
        _data[pos] = Component(std::forward<Args>(args)...);
        return _data[pos];
    }

    reference_type insert_at(size_type pos, Component&& value) {
        if (pos >= _data.size()) _data.resize(pos + 1);
        _data[pos] = std::move(value);
        return _data[pos];
    }

    void erase(size_type pos) {
        if (pos < _data.size()) _data[pos].reset();
    }

    void clear() { _data.clear(); }

    size_type size() const { return _data.size(); }
};
