#pragma once

#include <variant>

#include "level.hpp"

namespace sm {

struct DefenceSceneArgs {
    const level::Level& level;
};

using SceneArgs = std::variant<DefenceSceneArgs>;

template <typename T>
concept IsScene = requires(T c, SceneArgs args) {
    c.load(args);
    c.free();
};

}  // namespace sm