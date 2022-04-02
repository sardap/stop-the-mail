#include <gfx/defenceSceneShared.h>
#include <gfx/mailSpritesheet.h>
#include <gfx/towerSpritesheet.h>
#include <nds.h>
#include <stdio.h>

#include <common.hpp>
#include <defence_scene.hpp>

namespace sm {

static const int mail_oam_id_offset = 127 - 70;
static const int tower_oam_id_offset = mail_oam_id_offset - 15;
static const int effects_oam_id_offset = tower_oam_id_offset - 30;

DefenceScene::DefenceScene(const level::Level& level)
    : m_level(std::move(level)),
      m_round_idx(0),
      m_spawn_idx(0),
      m_mail_idx(0),
      m_tower_idx(0) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);

    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    decompress(m_level.background.bitmap, bgGetGfxPtr(bg), LZ77Vram);

    consoleDemoInit();

    vramSetBankB(VRAM_B_MAIN_SPRITE);
    oamInit(&oamMain, SpriteMapping_1D_128, false);

    dmaCopy(defenceSceneSharedPal, SPRITE_PALETTE, defenceSceneSharedPalLen);

    // Zero out collisions
    for (size_t i = 0; i < m_collisions.size(); i++) {
        m_collisions[i] = nullptr;
    }

    for (size_t i = 0; i < m_mails.size(); i++) {
        auto& mail = m_mails[i];
        mail.active = false;
        mail.gfx.oam = &oamMain;
        mail.gfx.oam_id = mail_oam_id_offset + i;
        mail.gfx.tile = oamAllocateGfx(mail.gfx.oam, SpriteSize_16x16,
                                       SpriteColorFormat_256Color);

        mail.collsion.owner.mail = &mail;
        mail.collsion.owner.type = Identity::Type::MAIL;
    }

    for (size_t i = 0; i < m_towers.size(); i++) {
        auto& tower = m_towers[i];
        tower.active = false;
        tower.gfx.oam = &oamMain;
        tower.gfx.oam_id = tower_oam_id_offset + i;
        tower.gfx.tile = oamAllocateGfx(tower.gfx.oam, SpriteSize_16x16,
                                        SpriteColorFormat_256Color);
    }

    for (size_t i = 0; i < m_effects.size(); i++) {
        auto& effect = m_effects[i];
        effect.active = false;
        effect.gfx.oam = &oamMain;
        effect.gfx.oam_id = effects_oam_id_offset + i;
        effect.gfx.tile = nullptr;
    }

    create_cat(Position{.x = Fixed(30), .y = Fixed(20)},
               Position{.x = Fixed(30), .y = Fixed(36)});
}

DefenceScene::~DefenceScene() {
    for (const auto& mail : m_mails) {
        oamFreeGfx(mail.gfx.oam, mail.gfx.tile);
    }

    for (const auto& tower : m_towers) {
        oamFreeGfx(tower.gfx.oam, tower.gfx.tile);
    }

    for (const auto& effect : m_effects) {
        if (effect.gfx.tile != nullptr) {
            oamFreeGfx(effect.gfx.oam, effect.gfx.tile);
        }
    }
}

void DefenceScene::update() {
    spawn_pending();

    iprintf("Current Round %d\n", m_round_idx);

    // Update collisions
    update_collisions<100>(m_collisions);

    for (size_t i = 0; i < m_mails.size(); i++) {
        update_mail(m_mails[i], i);
    }

    for (size_t i = 0; i < m_towers.size(); i++) {
        update_tower(m_towers[i], i);
    }
}

void DefenceScene::spawn_pending() {
    const auto& round = m_level.rounds[m_round_idx];
    const auto current_spawn = round.spawns[m_spawn_idx];

    if (m_spawn_idx >= round.spawns.size() || !current_spawn) {
        if (any_mail_active()) {
            return;
        }
        m_round_idx++;
        if (m_round_idx >= m_level.rounds.size()) {
            m_round_idx--;
        }
        return;
    }

    if (globals::current_frame >= round.spawns[m_spawn_idx]->frame) {
        create_mail(*current_spawn);
        m_spawn_idx++;
    }
}

Mail& DefenceScene::get_free_mail() {
    return get_free<Mail, 70>(m_mails, m_mail_idx, m_spare_mail);
}

Mail& DefenceScene::create_mail(const level::SpawnInfo& spawn_info) {
    auto& mail = get_free_mail();

    mail.postion = m_level.starting_point;
    mail.waypoint = Waypoint{
        .current_waypoint_idx = 0,
        .waypoints = &m_level.mail_waypoints,
    };
    mail.vel.vx = Fixed(0);
    mail.vel.vy = Fixed(0);
    mail.active = true;

    int tileOffset = 0;
    switch (spawn_info.type) {
        case level::MailType::INVALID:
        case level::MailType::BASIC_ENVELOPE:
            mail.speed = Fixed(0.4);
            mail.life.maxHp = 1;
            tileOffset = 0;
            break;
        case level::MailType::OFFICIAL_ENVELOPE:
            mail.speed = Fixed(0.8);
            mail.life.maxHp = 1;
            tileOffset = 1;
            break;
        case level::MailType::EXPRESS_ENVELOPE:
            mail.speed = Fixed(1.6);
            mail.life.maxHp = 1;
            tileOffset = 2;
            break;
        case level::MailType::EXPRESS_OFFICIAL_ENVELOPE:
            mail.speed = Fixed(1.8);
            mail.life.maxHp = 1;
            tileOffset = 3;
            break;
        case level::MailType::BASIC_PACKAGE:
            mail.speed = Fixed(0.3);
            mail.life.maxHp = 1;
            tileOffset = 4;
            break;
    }
    mail.life.currentHp = mail.life.maxHp;
    u8* offset = (u8*)mailSpritesheetTiles + (tileOffset * (16 * 16));
    dmaCopy(offset, mail.gfx.tile, 16 * 16);

    add_collsion(&mail.collsion);
    step_mail_collsion(mail);

    return mail;
}

void DefenceScene::free_mail(sm::Mail& mail, int idx) {
    mail.active = false;
    m_mail_idx = idx;
    oamSetHidden(mail.gfx.oam, mail.gfx.oam_id, true);
    remove_collsion(&mail.collsion);
}

void DefenceScene::update_mail(sm::Mail& mail, int idx) {
    if (!mail.active) {
        return;
    }

    waypoint_update(mail.waypoint, mail.vel, mail.postion, mail.speed);
    if (mail.waypoint.current_waypoint_idx >= mail.waypoint.waypoints->count) {
        free_mail(mail, idx);
        return;
    }
    postion_update(mail.postion, mail.vel);
    step_mail_collsion(mail);

    update_oam(mail.postion, mail.gfx);

    mail.distance_from_end =
        distance(mail.postion, mail.waypoint.waypoints->get_last_point());

    refresh_collision(mail.collsion);
}

bool DefenceScene::any_mail_active() {
    for (const auto& mail : m_mails) {
        if (mail.active) {
            return true;
        }
    }

    return false;
}

Tower& DefenceScene::create_cat(Position pos, Position colPos) {
    auto& tower = get_free_tower();
    tower.active = true;

    auto tileOffset = 0;
    Cat& cat = std::get<Cat>(tower.specific);
    setup_collision(
        cat.col,
        Collsion::Collider{.type = Identity::Type::TOWER, .tower = &tower},
        Rectangle{.x = colPos.x, .y = colPos.y, .w = 16, .h = 16});
    add_collsion(&cat.col);

    tower.pos = pos;
    u8* offset = (u8*)towerSpritesheetTiles + (tileOffset * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower;
}

void DefenceScene::free_tower(sm::Tower& tower, int idx) {
    tower.active = false;
    m_tower_idx = idx;
    oamSetHidden(tower.gfx.oam, tower.gfx.oam_id, true);
}

#pragma GCC push_options
#pragma GCC optimize("O0")

void DefenceScene::update_tower(sm::Tower& tower, int idx) {
    if (!tower.active) {
        return;
    }

    update_oam(tower.pos, tower.gfx);

    if (auto* cat = std::get_if<Cat>(&tower.specific); cat) {
        for (size_t i = 0; i < cat->col.collisions.size(); i++) {
            if (!cat->col.collisions[i]) {
                break;
            }
            const auto& collider = cat->col.collisions[i]->object;
            switch (collider.type) {
                case Identity::Type::INVALID:
                    break;
                case Identity::Type::MAIL:
                    break;
                case Identity::Type::TOWER:
                    break;
            }
        }

        refresh_collision(cat->col);
    }
}
#pragma GCC pop_options

Tower& DefenceScene::get_free_tower() {
    return get_free<Tower, 15>(m_towers, m_tower_idx, m_spare_tower);
}

void DefenceScene::add_collsion(Collsion* collsion) {
    for (size_t i = 0; i < m_collisions.size(); i++) {
        if (m_collisions[i] == nullptr) {
            m_collisions[i] = collsion;
            break;
        }
    }
}

void DefenceScene::remove_collsion(Collsion* collsion) {
    for (size_t i = 0; i < m_collisions.size(); i++) {
        if (m_collisions[i] == collsion) {
            m_collisions[i] = nullptr;
            break;
        }
    }
}

}  // namespace sm