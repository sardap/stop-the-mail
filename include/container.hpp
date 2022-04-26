#pragma once

#include <array>
#include <bitset>
#include <collision.hpp>
#include <effect.hpp>
#include <enemies.hpp>
#include <optional>
#include <towers.hpp>

namespace sm {

template <typename T>
concept IsContainer = requires(T c, size_t i, Mail& m, Tower& t, Effect& e,
                               Collsion* col, size_t length,
                               const TextGroup& tg) {
    { c.get_free_mail() } -> std::convertible_to<Mail*>;
    c.free_mail(m);

    { c.get_free_effect() } -> std::convertible_to<Effect*>;
    c.free_effect(e, i);

    { c.get_free_tower() } -> std::convertible_to<Tower*>;
    c.free_tower(t, i);

    c.add_collsion(col);
    c.remove_collsion(col);

    { c.m_text_info } -> std::convertible_to<TextInfo>;
    { c.get_free_text(length) } -> std::convertible_to<TextGroup>;
    c.free_text(tg);
};

template <size_t MailCount, size_t TowerCount, size_t EffectCount,
          size_t ColCount>
class Container {
   public:
    Container() {
        // Zero out collisions
        for (size_t i = 0; i < m_collisions.size(); i++) {
            m_collisions[i] = nullptr;
        }

        m_texts = 0;

        m_text_info.oam = nullptr;
        m_text_info.sheet_offset = nullptr;
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

    std::bitset<127 - (MailCount + TowerCount + EffectCount)> m_texts;

   public:
    size_t m_mail_idx;
    std::array<Mail, MailCount> m_mails;

    size_t m_tower_idx;
    std::array<Tower, TowerCount> m_towers;

    size_t m_effect_idx;
    std::array<Effect, EffectCount> m_effects;

    std::array<Collsion*, ColCount> m_collisions;

    TextInfo m_text_info;

    Mail* get_free_mail() { return get_free(m_mails); }
    void free_mail(Mail& m) { free_resource(m, -1); }

    Tower* get_free_tower() { return get_free(m_towers); }
    void free_tower(Tower& t, size_t i) { free_resource(t, i); }

    Effect* get_free_effect() { return get_free(m_effects); }
    void free_effect(Effect& e, size_t i) { free_resource(e, i); }

    TextInfo& get_text_info() { return m_text_info; }
    TextGroup get_free_text(size_t length) {
        for (size_t i = 0; i < m_texts.size(); i++) {
            size_t start_offset = i;
            while (i < m_texts.size() && m_texts[i] == false) {
                if (i - start_offset == length) {
                    for (size_t k = start_offset; k < start_offset + length;
                         k++) {
                        m_texts[k] = true;
                    }
                    return TextGroup{.start = start_offset, .count = length};
                }
                i++;
            }
        }
        assert(true == false);
        return TextGroup{};
    }
    void free_text(const TextGroup& e) {
        for (size_t i = e.start; i < e.start + e.count; i++) {
            m_texts[i] = false;
        }
    }

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