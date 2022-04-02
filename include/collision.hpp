#pragma once

#include <common.hpp>
#include <iterator>
#include <optional>
#include <variant>

#include "identity.hpp"

namespace sm {

class Mail;
class Tower;

class Collsion {
   public:
    struct Collider {
        Identity::Type type;

        union {
            Mail *mail;
            Tower *tower;
        };
    };

    Collsion();

    // Add union with the pointers to each type with forward declations
    struct Event {
        Collider object;
    };

    Collider owner;
    Rectangle rect;
    std::array<std::optional<Event>, 10> collisions;
    size_t collisions_top{0};

    void push_collision(Event collsion);
};

void setup_collision(Collsion &collsion, Collsion::Collider collider,
                     Rectangle rect);

void refresh_collision(Collsion &collsion);

bool rectangles_overlap(const Rectangle &left, const Rectangle &right);

template <size_t N>
void update_collisions(std::array<Collsion *, N> colliders) {
    for (Collsion *inner : colliders) {
        if (inner == nullptr) continue;
        // Deal with active or not
        for (const Collsion *other : colliders) {
            // Self check
            if (other == nullptr || inner == other) {
                continue;
            }

            if (rectangles_overlap(inner->rect, other->rect)) {
                inner->push_collision(Collsion::Event{.object = other->owner});
            }
        }
    }
}

};  // namespace sm
