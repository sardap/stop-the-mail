#pragma once

#include <array>
#include <collision.hpp>
#include <effect.hpp>
#include <enemies.hpp>
#include <towers.hpp>

namespace sm {

template <typename T>
concept IsContainer = requires(T c, size_t i, Mail& m, Tower& t, Effect& e,
                               Collsion* col) {
    { c.get_free_mail() } -> std::convertible_to<Mail&>;
    c.free_mail(m, i);

    { c.get_free_effect() } -> std::convertible_to<Effect&>;
    c.free_effect(e, i);

    { c.get_free_tower() } -> std::convertible_to<Tower&>;
    c.free_tower(t, i);

    c.add_collsion(col);
    c.remove_collsion(col);
};

template <size_t MC = 70, size_t TC = 15, size_t EC = 30>
class Container {
   public:
    Container() {
        // Zero out collisions
        for (size_t i = 0; i < m_collisions.size(); i++) {
            m_collisions[i] = nullptr;
        }
    }

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

    template <typename T>
    void free_resource(T& resource, size_t current_idx, size_t& last_idx) {
        resource.active = false;
        last_idx = current_idx;
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

    std::array<Collsion*, 100> m_collisions;

    Mail& get_free_mail() {
        return get_free<Mail, MC>(m_mails, m_mail_idx, m_spare_mail);
    }

    void free_mail(Mail& m, size_t i) { free_resource(m, i, m_mail_idx); }

    Tower& get_free_tower() {
        return get_free<Tower, TC>(m_towers, m_tower_idx, m_spare_tower);
    }

    void free_tower(Tower& t, size_t i) { free_resource(t, i, m_tower_idx); }

    Effect& get_free_effect() {
        return get_free<Effect, EC>(m_effects, m_effect_idx, m_spare_effect);
    }

    void free_effect(Effect& e, size_t i) { free_resource(e, i, m_effect_idx); }

    void add_collsion(Collsion* collsion) {
        for (size_t i = 0; i < m_collisions.size(); i++) {
            if (m_collisions[i] == nullptr) {
                m_collisions[i] = collsion;
                break;
            }
        }
    }

    void remove_collsion(Collsion* collsion) {
        for (size_t i = 0; i < m_collisions.size(); i++) {
            if (m_collisions[i] == collsion) {
                m_collisions[i] = nullptr;
                break;
            }
        }
    }
};

}  // namespace sm