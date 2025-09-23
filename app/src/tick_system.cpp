#include "../include/tick_system.hpp"
#include <algorithm>
#include <thread>
#include <cmath>

TickSystem::TickSystem(double target_tps)
    : _target_tps(target_tps)
    , _tick_delta(1.0 / target_tps)
    , _max_catchup_ticks(5)
    , _accumulated_time(0.0)
    , _interpolation_factor(0.0)
    , _tick_count(0)
    , _tps_counter(0)
    , _actual_tps(0.0)
    , _running(false)
    , _initialized(false)
{
}

void TickSystem::setTickRate(double tps) {
    _target_tps = tps;
    _tick_delta = 1.0 / tps;
}

double TickSystem::getTickRate() const {
    return _target_tps;
}

void TickSystem::setMaxCatchupTicks(int max_ticks) {
    _max_catchup_ticks = std::max(1, max_ticks);
}

int TickSystem::getMaxCatchupTicks() const {
    return _max_catchup_ticks;
}

void TickSystem::run(const std::function<bool()>& should_continue,
                    const std::function<void(double)>& update_func,
                    const std::function<void(double)>& render_func) {

    if (!_initialized) {
        reset();
    }

    _running = true;

    while (should_continue() && _running) {
        updateTiming();

        int ticks_processed = 0;
        while (_accumulated_time >= _tick_delta && ticks_processed < _max_catchup_ticks) {
            update_func(_tick_delta);
            _accumulated_time -= _tick_delta;
            _tick_count++;
            _tps_counter++;
            ticks_processed++;
        }

        if (_accumulated_time >= _tick_delta) {
            _accumulated_time = std::fmod(_accumulated_time, _tick_delta);
        }

        _interpolation_factor = _accumulated_time / _tick_delta;

        render_func(_interpolation_factor);

        auto current_time = HighResClock::now();
        auto tps_elapsed = std::chrono::duration_cast<Duration>(current_time - _last_tps_update).count();
        if (tps_elapsed >= 1.0) {
            _actual_tps = _tps_counter / tps_elapsed;
            _tps_counter = 0;
            _last_tps_update = current_time;
        }

        auto frame_time = std::chrono::duration_cast<Duration>(HighResClock::now() - current_time).count();
        double min_frame_time = 1.0 / 240.0; // Cap at 240 FPS
        if (frame_time < min_frame_time) {
            std::this_thread::sleep_for(std::chrono::duration<double>(min_frame_time - frame_time));
        }
    }

    _running = false;
}

void TickSystem::reset() {
    _start_time = HighResClock::now();
    _last_update_time = _start_time;
    _last_tps_update = _start_time;
    _accumulated_time = 0.0;
    _interpolation_factor = 0.0;
    _tick_count = 0;
    _tps_counter = 0;
    _actual_tps = 0.0;
    _initialized = true;
}

double TickSystem::getCurrentTime() const {
    if (!_initialized) return 0.0;
    return std::chrono::duration_cast<Duration>(HighResClock::now() - _start_time).count();
}

double TickSystem::getTickDelta() const {
    return _tick_delta;
}

double TickSystem::getInterpolationFactor() const {
    return _interpolation_factor;
}

int TickSystem::getTickCount() const {
    return _tick_count;
}

double TickSystem::getActualTPS() const {
    return _actual_tps;
}

bool TickSystem::isRunning() const {
    return _running;
}

void TickSystem::updateTiming() {
    auto current_time = HighResClock::now();
    double delta_time = std::chrono::duration_cast<Duration>(current_time - _last_update_time).count();

    delta_time = std::min(delta_time, _tick_delta * _max_catchup_ticks);

    _accumulated_time += delta_time;
    _last_update_time = current_time;
}