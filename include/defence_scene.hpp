#pragma once

#include <array>

#include "bulletin_board.hpp"
#include "container.hpp"
#include "create_tower_menu.hpp"
#include "effect.hpp"
#include "enemies.hpp"
#include "level.hpp"
#include "player.hpp"
#include "scene.hpp"
#include "towers.hpp"

namespace sm {

class DefenceScene {
   public:
    DefenceScene();
    ~DefenceScene();

    void load(SceneArgs args);
    void free();

    void update();

    using MainObjects = Container<70, 15, 30, 100>;

   private:
    level::Level m_level;
    size_t m_round_idx;
    size_t m_spawn_idx;
    BulletinBoard m_board;
    Player m_player;
    int m_money;

    int m_menu_bg;

    const CreateTowerItem<MainObjects>* m_selected_item;

    Container<0, 0, 0, 0> m_sub_objects;
    Container<70, 15, 30, 100> m_main_objects;

    enum class InputState {
        SELECT,
        CREATE_PRESSED,
        CREATE_MENU_SHOWING,
        CREATE_MENU_SELECTED,
        CREATE
    };

    InputState m_input_state;
    Effect* m_cursor;
    int m_cursor_timeout;

    Effect* m_building_icon;
    Effect* m_next_round_icon;

    struct MSGeneral {
        TextGroup lives_text;
        TextGroup round_text;
        TextGroup money_text;
    };
    struct MSTowerStats {
        Tower* tower;
    };
    std::variant<MSGeneral, MSTowerStats> m_ms_state;

    void init_ui_elements();

    Tower* overlaps_with_tower(Rectangle& given);
    void spawn_pending();
    bool any_mail_active();
    void process_player_input();

    void ui_change_general();
    void ui_cleanup();
    void ui_update();
};

}  // namespace sm