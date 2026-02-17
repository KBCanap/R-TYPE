/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** tick_system
*/

#pragma once
#include <chrono>
#include <functional>

class TickSystem {
  public:
    using HighResClock = std::chrono::high_resolution_clock;
    using TimePoint = HighResClock::time_point;
    using Duration = std::chrono::duration<double>;

    explicit TickSystem(double target_tps = 60.0);

    void setTickRate(double tps);
    double getTickRate() const;

    void setMaxCatchupTicks(int max_ticks);
    int getMaxCatchupTicks() const;

    void run(const std::function<bool()> &should_continue,
             const std::function<void(double)> &update_func,
             const std::function<void(double)> &render_func);

    void reset();

    double getCurrentTime() const;
    double getTickDelta() const;
    double getInterpolationFactor() const;

    int getTickCount() const;
    double getActualTPS() const;
    bool isRunning() const;

  private:
    void updateTiming();

    double _target_tps;
    double _tick_delta;
    int _max_catchup_ticks;

    TimePoint _start_time;
    TimePoint _last_update_time;
    double _accumulated_time;
    double _interpolation_factor;

    int _tick_count;
    int _tps_counter;
    double _actual_tps;
    TimePoint _last_tps_update;

    bool _running;
    bool _initialized;
};