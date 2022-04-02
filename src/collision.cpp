#include <collision.hpp>

namespace sm {

Collsion::Collsion() : collisions_top(0) {
    for (size_t i = 0; i < collisions.size(); i++) {
        collisions[i] = std::nullopt;
    }
}

void Collsion::push_collision(Event collsion) {
    if (collisions_top >= collisions.size()) {
        return;
    }

    collisions[collisions_top] = collsion;
    collisions_top++;
}

void setup_collision(Collsion &collsion, Collsion::Collider owner,
                     Rectangle rect) {
    collsion.owner = owner;
    collsion.rect = rect;
    for (size_t i = 0; i < collsion.collisions.size(); i++) {
        collsion.collisions[i] = std::nullopt;
    }
    collsion.collisions_top = 0;
}

void refresh_collision(Collsion &collsion) {
    if (collsion.collisions_top >= collsion.collisions.size()) {
        collsion.collisions_top = collsion.collisions.size() - 1;
    }

    for (size_t i = collsion.collisions_top; i > 0; i--) {
        collsion.collisions[i] = std::nullopt;
    }
    collsion.collisions_top = 0;
}

bool rectangles_overlap(const Rectangle &left, const Rectangle &right) {
    int a = left.x < (right.x + right.w);
    int b = (left.x + left.w) > right.x;
    int c = left.y < (right.y + right.h);
    int d = (left.y + left.h) > right.y;

    return a && b && c && d;
}

};  // namespace sm
