#pragma once

#include <array>

#include "container.hpp"
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

    Container<70, 15, 30> m_objects;

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

    bool any_mail_active();
};

}  // namespace sm