#pragma once

#include <array>
#include <container.hpp>
#include <functional>

namespace sm {

template <IsContainer T>
struct CreateTowerItem {
    int cost;
    std::function<Tower*(T&, const CreateTowerArg&)> func;
};

template <IsContainer T>
struct CreateTowerMenu {
    std::array<CreateTowerItem<T>, 24> entires;
};

}  // namespace sm