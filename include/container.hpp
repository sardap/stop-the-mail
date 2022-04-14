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
    { c.get_free_mail() } -> std::convertible_to<Mail*>;
    c.free_mail(m, i);

    { c.get_free_effect() } -> std::convertible_to<Effect*>;
    c.free_effect(e, i);

    { c.get_free_tower() } -> std::convertible_to<Tower*>;
    c.free_tower(t, i);

    c.add_collsion(col);
    c.remove_collsion(col);
};

template <size_t MC = 70, size_t TC = 15, size_t EC = 20>
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
    T* get_free(std::array<T, N>& source) {
        for (size_t i = 0; i < source.size(); i++) {
            if (T* result = &source[i]; !result->active) {
                return result;
            }
        }

        return nullptr;
    }

    template <typename T>
    void free_resource(T& resource, size_t current_idx) {
        resource.active = false;
    }

   public:
    size_t m_mail_idx;
    std::array<Mail, MC> m_mails;

    size_t m_tower_idx;
    std::array<Tower, TC> m_towers;

    size_t m_effect_idx;
    std::array<Effect, EC> m_effects;

    std::array<Collsion*, 100> m_collisions;

    Mail* get_free_mail() { return get_free(m_mails); }

    void free_mail(Mail& m, size_t i) { free_resource(m, i); }

    Tower* get_free_tower() { return get_free(m_towers); }

    void free_tower(Tower& t, size_t i) { free_resource(t, i); }

    Effect* get_free_effect() { return get_free(m_effects); }

    void free_effect(Effect& e, size_t i) { free_resource(e, i); }

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