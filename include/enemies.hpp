#pragma once

// STOP THE MAIL

#include "common.hpp"
#include "waypoints.hpp"

namespace sm {

struct Mail {
    Position postion;
    Waypoint waypoint;
    Vel vel;
    Graphics gfx;
    Life life;
    Fixed speed;
    bool active;
};

void update_mail(Mail& mail);

}  // namespace sm