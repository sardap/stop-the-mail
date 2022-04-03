#pragma once

#include <container.hpp>
#include <enemies.hpp>
#include <level.hpp>

namespace sm {

template <IsContainer T>
Mail& create_mail(T& container, const level::Level& level,
                  const level::SpawnInfo& spawn_info) {
    auto& mail = container.get_free_mail();

    mail.postion = level.starting_point;
    mail.waypoint = Waypoint{
        .current_waypoint_idx = 0,
        .waypoints = &level.mail_waypoints,
    };
    mail.vel.vx = Fixed(0);
    mail.vel.vy = Fixed(0);
    mail.active = true;

    int tileOffset = 0;
    switch (spawn_info.type) {
        case level::MailType::INVALID:
        case level::MailType::BASIC_ENVELOPE:
            mail.speed = Fixed(0.4);
            mail.life.maxHp = Fixed(1);
            tileOffset = 0;
            break;
        case level::MailType::OFFICIAL_ENVELOPE:
            mail.speed = Fixed(0.8);
            mail.life.maxHp = Fixed(2);
            tileOffset = 1;
            break;
        case level::MailType::EXPRESS_ENVELOPE:
            mail.speed = Fixed(1.6);
            mail.life.maxHp = Fixed(1);
            tileOffset = 2;
            break;
        case level::MailType::EXPRESS_OFFICIAL_ENVELOPE:
            mail.speed = Fixed(1.8);
            mail.life.maxHp = Fixed(2);
            tileOffset = 3;
            break;
        case level::MailType::BASIC_PACKAGE:
            mail.speed = Fixed(0.3);
            mail.life.maxHp = Fixed(5);
            tileOffset = 4;
            break;
    }
    mail.life.currentHp = mail.life.maxHp;
    u8* offset = (u8*)mailSpritesheetTiles + (tileOffset * (16 * 16));
    dmaCopy(offset, mail.gfx.tile, 16 * 16);

    container.add_collsion(&mail.collsion);
    step_mail_collsion(mail);

    return mail;
}

template <IsContainer T>
void free_mail(T& container, sm::Mail& mail, size_t idx) {
    container.free_mail(mail, idx);
    oamSetHidden(mail.gfx.oam, mail.gfx.oam_id, true);
    container.remove_collsion(&mail.collsion);
}

template <IsContainer T>
void update_mail(T& container, sm::Mail& mail, size_t idx) {
    if (!mail.active) {
        return;
    }

    if (mail.life.currentHp <= Fixed(0)) {
        free_mail(container, mail, idx);
        return;
    }

    waypoint_update(mail.waypoint, mail.vel, mail.postion, mail.speed);
    if (mail.waypoint.current_waypoint_idx >= mail.waypoint.waypoints->count) {
        free_mail(container, mail, idx);
        return;
    }
    postion_update(mail.postion, mail.vel);
    step_mail_collsion(mail);

    update_oam(mail.postion, mail.gfx);

    mail.distance_from_end =
        distance(mail.postion, mail.waypoint.waypoints->get_last_point());

    refresh_collision(mail.collsion);
}

}  // namespace sm