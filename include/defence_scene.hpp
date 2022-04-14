#pragma once

#include <array>

#include "container.hpp"
#include "effect.hpp"
#include "enemies.hpp"
#include "level.hpp"
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

   private:
    level::Level m_level;
    size_t m_round_idx;
    size_t m_spawn_idx;

    Container<70, 15, 30> m_objects;

    enum class InputState { SELECT, CREATE_PRESSED, CREATE };

    InputState m_input_state;
    Effect* m_cursor;
    int m_cursor_timeout;

    Effect* m_building_icon;

    void init_ui_elements();

    Tower* overlaps_with_tower(Rectangle& given);
    void spawn_pending();
    bool any_mail_active();
    void process_player_input();
    void ui_update();
};

}  // namespace sm