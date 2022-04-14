#include <gfx/defenceSceneSubShared.h>
#include <gfx/defenceStatsBackground.h>
#include <gfx/uiSprites.h>
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

static const int input_cooldown_max = 10;

static const Rectangle building_icon_rect =
    Rectangle{.x = Fixed(16), .y = Fixed(176), .w = Fixed(16), .h = Fixed(16)};

DefenceScene::DefenceScene() {}

DefenceScene::~DefenceScene() {}

void DefenceScene::load(SceneArgs args) {
    auto& defenceSceneArgs = std::get<DefenceSceneArgs>(args);
    m_level = std::move(defenceSceneArgs.level);
    m_round_idx = 0;
    m_spawn_idx = 0;
    m_cursor_timeout = 0;
    m_input_state = InputState::SELECT;

    // Main
    {
        videoSetMode(MODE_0_2D);
        vramSetBankA(VRAM_A_MAIN_BG);

        int bg = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
        decompress(defenceStatsBackgroundTiles, bgGetGfxPtr(bg), LZ77Vram);
        dmaCopy(defenceStatsBackgroundMap, bgGetMapPtr(bg),
                defenceStatsBackgroundMapLen);
        dmaCopy(defenceStatsBackgroundPal, BG_PALETTE,
                defenceStatsBackgroundPalLen);

        // auto* oam = &oamMain;
        // vramSetBankB(VRAM_B_MAIN_SPRITE);
        // oamInit(oam, SpriteMapping_1D_128, false);
        // dmaCopy(defenceSceneSubSharedPal, SPRITE_PALETTE,
        // defenceSceneSubSharedPalLen);
    }

    // Sub Screen
    {
        videoSetModeSub(MODE_5_2D);

        vramSetBankC(VRAM_C_SUB_BG);
        int bg = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
        decompress(m_level.background.bitmap, bgGetGfxPtr(bg), LZ77Vram);

        vramSetBankD(VRAM_D_SUB_SPRITE);
        oamInit(&oamSub, SpriteMapping_1D_128, false);
        dmaCopy(defenceSceneSubSharedPal, SPRITE_PALETTE_SUB,
                defenceSceneSubSharedPalLen);
    }

    auto* oam = &oamSub;
    // Logic
    for (size_t i = 0; i < m_objects.m_mails.size(); i++) {
        auto& mail = m_objects.m_mails[i];
        mail.active = false;
        mail.gfx.oam = oam;
        mail.gfx.oam_id = mail_oam_id_offset + i;
        mail.gfx.tile = oamAllocateGfx(mail.gfx.oam, SpriteSize_16x16,
                                       SpriteColorFormat_256Color);

        mail.collsion.owner.mail = &mail;
        mail.collsion.owner.type = Identity::Type::MAIL;
        mail.gfx.show = true;
    }

    for (size_t i = 0; i < m_objects.m_towers.size(); i++) {
        auto& tower = m_objects.m_towers[i];
        tower.active = false;
        tower.gfx.oam = oam;
        tower.gfx.oam_id = tower_oam_id_offset + i;
        tower.gfx.tile = oamAllocateGfx(tower.gfx.oam, SpriteSize_16x16,
                                        SpriteColorFormat_256Color);
        tower.gfx.show = true;
    }

    for (size_t i = 0; i < m_objects.m_effects.size(); i++) {
        auto& effect = m_objects.m_effects[i];
        effect.active = false;
        effect.gfx.oam = oam;
        effect.gfx.oam_id = effects_oam_id_offset + i;
        effect.gfx.tile = nullptr;
        effect.gfx.show = true;
    }

    init_ui_elements();
}

void DefenceScene::free() {
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

    scanKeys();
    process_player_input();

    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
}

void DefenceScene::init_ui_elements() {
    {
        m_cursor = m_objects.get_free_effect();
        m_cursor->active = true;
        m_cursor->gfx.tile = oamAllocateGfx(m_cursor->gfx.oam, SpriteSize_16x16,
                                            SpriteColorFormat_256Color);
        m_cursor->gfx.show = false;
        m_cursor->pos = Position{.x = Fixed(0), .y = Fixed(0)};
        m_cursor->specific = StaticEffect{};
        u8* offset = (u8*)uiSpritesTiles + (0 * (16 * 16));
        dmaCopy(offset, m_cursor->gfx.tile, 16 * 16);
    }

    {
        m_building_icon = m_objects.get_free_effect();
        m_building_icon->active = true;
        m_building_icon->gfx.tile =
            oamAllocateGfx(m_building_icon->gfx.oam, SpriteSize_16x16,
                           SpriteColorFormat_256Color);
        m_building_icon->gfx.show = true;
        m_building_icon->pos =
            Position{.x = building_icon_rect.x, .y = building_icon_rect.y};
        m_building_icon->specific = StaticEffect{};
        u8* offset = (u8*)uiSpritesTiles + (1 * (16 * 16));
        dmaCopy(offset, m_building_icon->gfx.tile, 16 * 16);
    }
}

Tower* DefenceScene::overlaps_with_tower(Rectangle& given) {
    Rectangle tower_rect{
        .x = Fixed(0), .y = Fixed(0), .w = Fixed(16), .h = Fixed(16)};
    for (size_t i = 0; i < m_objects.m_towers.size(); i++) {
        tower_rect.x = m_objects.m_towers[i].pos.x;
        tower_rect.y = m_objects.m_towers[i].pos.y;
        if (rectangles_overlap(given, tower_rect)) {
            return &m_objects.m_towers[i];
        }
    }

    return nullptr;
}

void DefenceScene::process_player_input() {
    // Cursor
    Position touch_position =
        Position{.x = Fixed(((globals::touch_position.px) / 16) * 16),
                 .y = Fixed(((globals::touch_position.py) / 16)) * 16};
    switch (m_input_state) {
        using enum InputState;
        case SELECT: {
            Rectangle touch_rect = Rectangle{.x = touch_position.x,
                                             .y = touch_position.y,
                                             .w = Fixed(2),
                                             .h = Fixed(2)};
            if (globals::touch_position.px != 0) {
                if (rectangles_overlap(building_icon_rect, touch_rect)) {
                    m_input_state = InputState::CREATE_PRESSED;
                } else if (overlaps_with_tower(touch_rect)) {
                    printf("here\n");
                } else {
                    m_cursor->gfx.show = true;
                    m_cursor->pos = touch_position;
                    m_cursor_timeout = 60;
                }
            } else if (m_cursor_timeout > 0) {
                m_cursor_timeout--;
            } else {
                m_cursor->gfx.show = false;
            }
            break;
        }
        case CREATE_PRESSED:
            if (globals::touch_position.px == 0 &&
                globals::touch_position.py == 0) {
                m_input_state = CREATE;
            }
            break;
        case CREATE: {
            m_cursor->gfx.show = false;
            if (globals::touch_position.px != 0) {
                create_cat(m_objects, touch_position,
                           Position{.x = touch_position.x,
                                    .y = touch_position.y + 16});
                m_input_state = InputState::SELECT;
            }
            break;
        }
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