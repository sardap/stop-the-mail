#include <gfx/defenceSceneShared.h>
#include <gfx/mailSpritesheet.h>
#include <gfx/towerSpritesheet.h>
#include <nds.h>
#include <stdio.h>

#include <common.hpp>
#include <defence_scene.hpp>

namespace sm {

static const int mail_oam_id_offset = 127 - 70;
static const int tower_oam_id_offset = mail_oam_id_offset - 30;

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

    for (size_t i = 0; i < m_mails.size(); i++) {
        auto& mail = m_mails[i];
        mail.active = false;
        mail.gfx.oam = &oamMain;
        mail.gfx.oam_id = mail_oam_id_offset + i;
        mail.gfx.tile = oamAllocateGfx(mail.gfx.oam, SpriteSize_16x16,
                                       SpriteColorFormat_256Color);
    }

    for (size_t i = 0; i < m_towers.size(); i++) {
        auto& tower = m_towers[i];
        tower.active = false;
        tower.gfx.oam = &oamMain;
        tower.gfx.oam_id = tower_oam_id_offset + i;
        tower.gfx.tile = oamAllocateGfx(tower.gfx.oam, SpriteSize_16x16,
                                        SpriteColorFormat_256Color);
    }

    create_tower(TowerType::CAT, Position{.x = Fixed(10), .y = Fixed(20)});
}

DefenceScene::~DefenceScene() {
    for (const auto& mail : m_mails) {
        oamFreeGfx(&oamMain, mail.gfx.tile);
    }
}

void DefenceScene::update() {
    spawn_pending();

    iprintf("Current Round %d\n", m_round_idx);

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

    return mail;
}

void DefenceScene::free_mail(sm::Mail& mail, int idx) {
    mail.active = false;
    m_mail_idx = idx;
    oamSetHidden(mail.gfx.oam, mail.gfx.oam_id, true);
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
    update_oam(mail.postion, mail.gfx);
}

bool DefenceScene::any_mail_active() {
    for (const auto& mail : m_mails) {
        if (mail.active) {
            return true;
        }
    }

    return false;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
Tower& DefenceScene::create_tower(const TowerType& type, Position pos) {
    auto& tower = get_free_tower();
    tower.active = true;

    int tileOffset = -1;
    switch (type) {
        case TowerType::CAT:
            tileOffset = 0;
            break;
    }

    assert(tileOffset >= 0);

    tower.pos = pos;
    u8* offset = (u8*)towerSpritesheetTiles + (tileOffset * (16 * 16));
    dmaCopy(offset, tower.gfx.tile, 16 * 16);

    return tower;
}
#pragma GCC pop_options

void DefenceScene::free_tower(sm::Tower& tower, int idx) {
    tower.active = false;
    m_tower_idx = idx;
    oamSetHidden(tower.gfx.oam, tower.gfx.oam_id, true);
}

void DefenceScene::update_tower(sm::Tower& tower, int idx) {
    if (!tower.active) {
        return;
    }

    update_oam(tower.pos, tower.gfx);
}

Tower& DefenceScene::get_free_tower() {
    return get_free<Tower, 30>(m_towers, m_tower_idx, m_spare_tower);
}

}  // namespace sm