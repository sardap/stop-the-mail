#pragma once

#include <common.hpp>
#include <functional>
#include <initializer_list>
#include <optional>

namespace sm {

struct Waypoints {
    Position* points;
    size_t count;

    Waypoints();
    Waypoints(std::initializer_list<Position> postions);
    ~Waypoints();

    Position get_last_point() const;
};

struct Waypoint {
    size_t current_waypoint_idx;
    const Waypoints* waypoints;
};

void waypoint_update(Waypoint& waypoint, Vel& vel, Position& pos,
                     const Fixed& speed);

}  // namespace sm