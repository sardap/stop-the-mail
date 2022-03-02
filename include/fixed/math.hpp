#pragma once

#include <nds/arm9/math.h>
#include <nds/ndstypes.h>

#include "fixed.hpp"

namespace sm::math {

template <typename T>
inline Fixed<T> sqrt(const Fixed<T> x) {
    return Fixed<T>::raw_initalise(sqrtf32(x.raw_value()));
}

template <typename T>
inline Fixed<T> pow(Fixed<T> x, int32 y) {
    Fixed<T> result = Fixed<T>(1);
    while (y > 0) {
        if (y % 2 == 0)  // y is even
        {
            x *= x;
            y = y / 2;
        } else  // y isn't even
        {
            result = result * x;
            y = y - 1;
        }
    }
    return result;
}

}  // namespace sm::math