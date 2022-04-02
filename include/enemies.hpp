#pragma once

// STOP THE MAIL

#include "collision.hpp"
#include "common.hpp"
#include "waypoints.hpp"

namespace sm {

struct Mail {
    Position postion;
    Waypoint waypoint;
    Vel vel;
    Graphics gfx;
    Life life;
    Identity identity;
    Collsion collsion;
    Fixed speed;
    Fixed distance_from_end;
    bool active;
};

void step_mail_collsion(Mail& mail);

}  // namespace sm