#pragma once

#include <array>

#include "effect.hpp"
#include "enemies.hpp"
#include "level.hpp"
#include "towers.hpp"

namespace sm {

class DefenceScene {
   public:
    DefenceScene(const level::Level& level);
    ~DefenceScene();

    void update();

   private:
    level::Level m_level;
    size_t m_round_idx;
    size_t m_spawn_idx;

    size_t m_mail_idx;
    std::array<Mail, 70> m_mails;
    Mail m_spare_mail;

    size_t m_tower_idx;
    std::array<Tower, 15> m_towers;
    Tower m_spare_tower;

    size_t m_effect_idx;
    std::array<Effect, 30> m_effects;
    Effect m_spare_effect;

    std::array<Collsion*, 100> m_collisions;

    void spawn_pending();

    template <typename T, size_t N>
    T& get_free(std::array<T, N>& source, size_t& idx, T& spare) {
        for (size_t i = 0; i < source.size(); i++) {
            if (auto& result = source[idx]; !result.active) {
                idx++;
                return result;
            }
            idx++;
            if (idx >= source.size()) {
                idx = 0;
            }
        }

        return spare;
    }

    Mail& create_mail(const level::SpawnInfo& spawn_info);
    void free_mail(sm::Mail& mail, int idx);
    void update_mail(sm::Mail& mail, int idx);
    Mail& get_free_mail();
    bool any_mail_active();

    Tower& create_cat(Position pos, Position colPos);

    void free_tower(sm::Tower& mail, int idx);
    void update_tower(sm::Tower& mail, int idx);
    Tower& get_free_tower();

    void add_collsion(Collsion* collsion);
    void remove_collsion(Collsion* collsion);
};

}  // namespace sm