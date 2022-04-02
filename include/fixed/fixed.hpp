#pragma once

#include <nds/arm9/math.h>
#include <nds/ndstypes.h>

namespace sm::math {

template <typename T>
class Fixed {
   public:
    Fixed(int val) : mVal(inttof32(val)) {}
    Fixed(int32 val) : mVal(inttof32(val)) {}
    Fixed(float val) : mVal(floattof32(val)) {}
    Fixed(double val) : Fixed(static_cast<float>(val)) {}
    Fixed(const Fixed<T>& other) : mVal(other.mVal) {}
    Fixed() : mVal(0) {}

    static Fixed<T> raw_initalise(int32 val) {
        Fixed<T> result;
        result.mVal = val;
        return result;
    }

    explicit operator int() const { return f32toint(mVal); }
    explicit operator long int() const { return f32toint(mVal); }
    explicit operator float() const { return f32tofloat(mVal); }

    // move assignment
    Fixed<T>& operator=(Fixed<T>& other) noexcept {
        // Guard self assignment
        if (this == &other) return *this;  // delete[]/size=0 would also be ok

        mVal = other.mVal;
        return *this;
    }

    Fixed<T>& operator=(const Fixed<T>& other) noexcept {
        mVal = other.mVal;
        return *this;
    }

    Fixed<T> operator+(const Fixed<T>& other) const {
        return raw_initalise(mVal + other.mVal);
    }

    Fixed<T>& operator+=(const Fixed<T>& other) noexcept {
        mVal += other.mVal;
        return *this;
    }

    Fixed<T> operator-(const Fixed<T>& other) const {
        return raw_initalise(mVal - other.mVal);
    }

    Fixed<T>& operator-=(const Fixed<T>& other) noexcept {
        mVal -= other.mVal;
        return *this;
    }

    Fixed<T> operator*(const Fixed<T>& other) const {
        return raw_initalise(mulf32(mVal, other.mVal));
    }

    Fixed<T>& operator*=(const Fixed<T>& other) noexcept {
        mVal = mVal * other.mVal;
        return *this;
    }

    Fixed<T> operator/(const Fixed<T>& other) const {
        return raw_initalise(divf32(mVal, other.mVal));
    }

    Fixed<T>& operator/=(const Fixed<T>& other) noexcept {
        mVal = mVal / other.mVal;
        return *this;
    }

    bool operator>(const Fixed<T>& other) const { return mVal > other.mVal; }
    bool operator<(const Fixed<T>& other) const { return mVal < other.mVal; }
    bool operator==(const Fixed<T>& other) const { return mVal == other.mVal; }
    bool operator!=(const Fixed<T>& other) const { return mVal != other.mVal; }

    int32 raw_value() const { return mVal; }

   private:
    int32 mVal;
};

}  // namespace sm::math
