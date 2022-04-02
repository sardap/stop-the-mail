#pragma once

#include <common.hpp>
#include <functional>
#include <initializer_list>
#include <optional>

namespace sm {

struct Waypoints {
    Position* points;
    size_t count;

    Waypoints(std::initializer_list<Position> postions);
    ~Waypoints();

    Position get_last_point();
};

struct Waypoint {
    size_t current_waypoint_idx;
    Waypoints* waypoints;
};

void waypoint_update(Waypoint& waypoint, Vel& vel, Position& pos,
                     const Fixed& speed);

}  // namespace sm