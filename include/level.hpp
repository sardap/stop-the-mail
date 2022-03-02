#pragma once

#include <vector>

#include "common.hpp"
#include "waypoints.hpp"

namespace sm::level {

enum class MailType {
    INVALID = 0,
    BASIC_ENVELOPE = 1,
    OFFICIAL_ENVELOPE = 2,
    EXPRESS_ENVELOPE = 3,
    EXPRESS_OFFICIAL_ENVELOPE = 4,
    BASIC_PACKAGE = 5,
};

struct SpawnInfo {
    uint32 frame;
    MailType type;
};

struct Background {
    const unsigned int* bitmap{nullptr};
};

struct Round {
    std::array<std::optional<SpawnInfo>, 200> spawns;
};

struct PathMask {
    std::array<Rectangle, 50> paths;
};

struct Level {
    Waypoints mail_waypoints;
    Position starting_point;
    std::array<Round, 200> rounds;
    Background background;
    PathMask pathMask;
};

}  // namespace sm::level