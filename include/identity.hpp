#pragma once

namespace sm {

struct Identity {
    enum class Type {
        INVALID,
        MAIL,
        TOWER,
    };

    Type type;
};

}  // namespace sm