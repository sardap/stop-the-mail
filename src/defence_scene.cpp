#include <gfx/defenceSceneShared.h>
#include <gfx/mailSpritesheet.h>
#include <gfx/towerSpritesheet.h>
#include <nds.h>
#include <stdio.h>

#include <common.hpp>
#include <defence_scene.hpp>
#include <update_mail.hpp>
#include <update_tower.hpp>

namespace sm {

static const int mail_oam_id_offset = 127 - 70;
static const int tower_oam_id_offset = mail_oam_id_offset - 15;
static const int effects_oam_id_offset = tower_oam_id_offset - 30;

DefenceScene::DefenceScene(const level::Level& level)
    : m_level(std::move(level)), m_round_idx(0), m_spawn_idx(0) {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);

    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    decompress(m_level.background.bitmap, bgGetGfxPtr(bg), LZ77Vram);

    consoleDemoInit();

    vramSetBankB(VRAM_B_MAIN_SPRITE);
    oamInit(&oamMain, SpriteMapping_1D_128, false);

    dmaCopy(defenceSceneSharedPal, SPRITE_PALETTE, defenceSceneSharedPalLen);

    for (size_t i = 0; i < m_objects.m_mails.size(); i++) {
        auto& mail = m_objects.m_mails[i];
        mail.active = false;
        mail.gfx.oam = &oamMain;
        mail.gfx.oam_id = mail_oam_id_offset + i;
        mail.gfx.tile = oamAllocateGfx(mail.gfx.oam, SpriteSize_16x16,
                                       SpriteColorFormat_256Color);

        mail.collsion.owner.mail = &mail;
        mail.collsion.owner.type = Identity::Type::MAIL;
    }

    for (size_t i = 0; i < m_objects.m_towers.size(); i++) {
        auto& tower = m_objects.m_towers[i];
        tower.active = false;
        tower.gfx.oam = &oamMain;
        tower.gfx.oam_id = tower_oam_id_offset + i;
        tower.gfx.tile = oamAllocateGfx(tower.gfx.oam, SpriteSize_16x16,
                                        SpriteColorFormat_256Color);
    }

    for (size_t i = 0; i < m_objects.m_effects.size(); i++) {
        auto& effect = m_objects.m_effects[i];
        effect.active = false;
        effect.gfx.oam = &oamMain;
        effect.gfx.oam_id = effects_oam_id_offset + i;
        effect.gfx.tile = nullptr;
    }

    create_cat(m_objects, Position{.x = Fixed(30), .y = Fixed(20)},
               Position{.x = Fixed(30), .y = Fixed(36)});
}

DefenceScene::~DefenceScene() {
    for (const auto& mail : m_objects.m_mails) {
        oamFreeGfx(mail.gfx.oam, mail.gfx.tile);
    }

    for (const auto& tower : m_objects.m_towers) {
        oamFreeGfx(tower.gfx.oam, tower.gfx.tile);
    }

    for (const auto& effect : m_objects.m_effects) {
        if (effect.gfx.tile != nullptr) {
            oamFreeGfx(effect.gfx.oam, effect.gfx.tile);
        }
    }
}

void DefenceScene::update() {
    spawn_pending();

    // iprintf("Current Round %d\n", m_round_idx);

    // Update collisions
    update_collisions<100>(m_objects.m_collisions);

    for (size_t i = 0; i < m_objects.m_mails.size(); i++) {
        update_mail(m_objects, m_objects.m_mails[i], i);
    }

    for (size_t i = 0; i < m_objects.m_towers.size(); i++) {
        update_tower(m_objects, m_objects.m_towers[i]);
    }

    for (size_t i = 0; i < m_objects.m_effects.size(); i++) {
        update_effect(m_objects.m_effects[i]);
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
        create_mail(m_objects, m_level, *current_spawn);
        m_spawn_idx++;
    }
}

bool DefenceScene::any_mail_active() {
    for (const auto& mail : m_objects.m_mails) {
        if (mail.active) {
            return true;
        }
    }

    return false;
}

}  // namespace sm