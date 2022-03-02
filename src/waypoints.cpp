#include <nds/arm9/math.h>
#include <stdio.h>

#include <fixed/math.hpp>
#include <waypoints.hpp>

namespace sm {

Waypoints::Waypoints(std::initializer_list<Position> postions) {
    points = new Position[postions.size()];

    size_t i = 0;
    for (const auto& postion : postions) {
        points[i] = postion;
        i++;
    }

    count = postions.size();
}

Waypoints::~Waypoints() { delete points; }

static const Fixed WAYPOINT_DIST_TREASHOLD = Fixed(1.0);

void waypoint_update(Waypoint& waypoint, Vel& vel, Position& pos,
                     const Fixed& speed) {
    // Complete
    if (waypoint.current_waypoint_idx >= waypoint.waypoints->count) {
        vel.vx = Fixed(0);
        vel.vy = Fixed(0);
        return;
    }

    // Get current waypoint
    Position target = waypoint.waypoints->points[waypoint.current_waypoint_idx];
    if (distance(pos, target) < WAYPOINT_DIST_TREASHOLD) {
        waypoint.current_waypoint_idx++;
        waypoint_update(waypoint, vel, pos, speed);
    }

    auto tx = target.x - pos.x;
    auto ty = target.y - pos.y;
    auto dist = math::sqrt(tx * tx + ty * ty);

    vel.vx = (tx / dist) * speed;
    vel.vy = (ty / dist) * speed;
}

}  // namespace sm