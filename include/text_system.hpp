#pragma once

#include <common.hpp>
#include <container.hpp>
#include <string>

namespace sm {

void init_text(TextInfo& text);

template <IsContainer T>
TextGroup create_text(T& container, size_t length) {
    TextGroup result = container.get_free_text(length);
    return result;
}

void set_text(TextInfo& text_info, const TextGroup& text_group,
              const std::string& str, Position pos);

}  // namespace sm
