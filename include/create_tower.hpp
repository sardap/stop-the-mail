#pragma once

#include <gfx/DefenceTowerSpritesheet.h>

#include <container.hpp>

namespace sm {

template <IsContainer T>
Tower* create_blank(T& container, const CreateTowerArg& arg) {
    return nullptr;
}

template <IsContainer T>
Tower* create_cat(T& container, const CreateTowerArg& arg) {
    const Position& pos = arg.pos;

    auto* tower_ptr = container.get_free_tower();
    if (tower_ptr == nullptr) {
        return nullptr;
    }
    auto& tower = *tower_ptr;
    assert(tower.active == false);
    tower.active = true;
    tower.damage_dealt = 0;

    tower.specific = Cat{};
    Cat& cat = std::get<Cat>(tower.specific);
    setup_collision(
        cat.col,
        Collsion::Collider{.type = Identity::Type::TOWER, .tower = &tower},
        Rectangle{.x = pos.x, .y = pos.y + 16, .w = 16, .h = 16});
    container.add_collsion(&cat.col);
    cat.current_cooldown = 0;

    tower.pos = pos;
    tower.gfx.color_format = SpriteColorFormat_256Color;
    tower.gfx.size = SpriteSize_16x16;
    u8* offset = (u8*)DefenceTowerSpritesheetTiles + (0 * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower_ptr;
}

template <IsContainer T>
Tower* create_begal(T& container, const CreateTowerArg& arg) {
    const Position& pos = arg.pos;

    auto* tower_ptr = container.get_free_tower();
    if (tower_ptr == nullptr) {
        return nullptr;
    }
    Tower& tower = *tower_ptr;
    assert(tower.active == false);
    tower.active = true;
    tower.damage_dealt = 0;

    tower.specific = Begal{};
    Begal& begal = std::get<Begal>(tower.specific);
    setup_collision(
        begal.col,
        Collsion::Collider{.type = Identity::Type::TOWER, .tower = &tower},
        Rectangle{.x = pos.x - 16, .y = pos.y - 16, .w = 32, .h = 32});
    container.add_collsion(&begal.col);
    begal.current_cooldown = 0;

    tower.pos = pos;
    tower.gfx.color_format = SpriteColorFormat_256Color;
    tower.gfx.priority = 1;
    tower.gfx.size = SpriteSize_16x16;
    u8* offset = (u8*)DefenceTowerSpritesheetTiles + (2 * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower_ptr;
}

}  // namespace sm