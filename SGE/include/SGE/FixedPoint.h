// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cassert>
#include <type_traits>

namespace s25edit {
template<typename T_Underlying, size_t T_decimals>
class FixedPoint
{
    using UnsignedData = std::make_unsigned_t<T_Underlying>;
    using SignedData = std::make_unsigned_t<T_Underlying>;
    static constexpr size_t factor = 1 << T_decimals;
    T_Underlying data;

public:
    constexpr explicit FixedPoint(T_Underlying value = 0) : data(value * factor) {}
    constexpr explicit operator T_Underlying() const noexcept { return data / factor; }
    constexpr SignedData toInt() const noexcept { return data / factor; }
    constexpr UnsignedData toUnsigned() const noexcept
    {
        assert(data / factor >= 0);
        return data / factor;
    }
    FixedPoint& operator+=(FixedPoint rhs) noexcept
    {
        data += rhs.data;
        return *this;
    }
    FixedPoint& operator-=(FixedPoint rhs) noexcept
    {
        data -= rhs.data;
        return *this;
    }
    FixedPoint& operator*=(T_Underlying rhs) noexcept
    {
        data *= rhs;
        return *this;
    }
    FixedPoint& operator/=(T_Underlying rhs) noexcept
    {
        data /= rhs;
        return *this;
    }
    friend FixedPoint operator+(FixedPoint lhs, FixedPoint rhs) { return lhs += rhs; }
    friend FixedPoint operator-(FixedPoint lhs, FixedPoint rhs) { return lhs -= rhs; }
    friend FixedPoint operator*(FixedPoint lhs, T_Underlying rhs) { return lhs *= rhs; }
    friend FixedPoint operator*(T_Underlying lhs, FixedPoint rhs) { return rhs *= lhs; }
    friend FixedPoint operator/(FixedPoint lhs, T_Underlying rhs) { return lhs /= rhs; }
};
} // namespace s25edit
