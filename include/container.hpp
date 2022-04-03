#pragma once

#include <array>
#include <effect.hpp>
#include <enemies.hpp>
#include <towers.hpp>

namespace sm {

template <typename T>
concept IsContainer = requires(T t) {
    { t.get_free_mail() } -> std::convertible_to<Mail&>;
    { t.get_free_effect() } -> std::convertible_to<Effect&>;
    { t.get_free_tower() } -> std::convertible_to<Tower&>;
};

template <size_t MC = 70, size_t TC = 15, size_t EC = 30>
class Container {
   public:
    Container() : m_mail_idx(0), m_tower_idx(0), m_effect_idx(0) {}

   private:
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

   public:
    size_t m_mail_idx;
    std::array<Mail, MC> m_mails;
    Mail m_spare_mail;

    size_t m_tower_idx;
    std::array<Tower, TC> m_towers;
    Tower m_spare_tower;

    size_t m_effect_idx;
    std::array<Effect, EC> m_effects;
    Effect m_spare_effect;

    Mail& get_free_mail() {
        return get_free<Mail, MC>(m_mails, m_mail_idx, m_spare_mail);
    }

    Tower& get_free_tower() {
        return get_free<Tower, TC>(m_towers, m_tower_idx, m_spare_tower);
    }

    Effect& get_free_effect() {
        return get_free<Effect, EC>(m_effects, m_effect_idx, m_spare_effect);
    }
};

}  // namespace sm