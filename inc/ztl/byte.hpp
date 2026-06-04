// SPDX-License-Identifier: BSL-1.0
//
// Copyright (c) 2026 Tobias Zindl
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying LICENSE file or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZTL_BYTE_HPP
#define ZTL_BYTE_HPP

#include "ztl/narrow.hpp"

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace ztl
{

// NOTE: backport of C++17 std::byte

// clang-format off
enum struct byte : std::uint8_t {};
// clang-format on

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr T to_integer(byte b) noexcept
{
    return static_cast<T>(b);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte operator<<(byte b, T shift) noexcept
{
    return byte(to_underlying(b) << shift);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte& operator<<=(byte& b, T shift) noexcept
{
    return b = (b << shift);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte operator>>(byte b, T shift) noexcept
{
    return byte(to_underlying(b) >> shift);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr byte& operator>>=(byte& b, T shift) noexcept
{
    return b = (b >> shift);
}

constexpr byte operator|(byte l, byte r) noexcept
{
    return byte(to_underlying(l) | to_underlying(r));
}

constexpr byte& operator|=(byte& l, byte r) noexcept { return l = (l | r); }

constexpr byte operator&(byte l, byte r) noexcept
{
    return byte(to_underlying(l) & to_underlying(r));
}

constexpr byte& operator&=(byte& l, byte r) noexcept { return l = (l & r); }

constexpr byte operator^(byte l, byte r) noexcept
{
    return byte(to_underlying(l) ^ to_underlying(r));
}

constexpr byte& operator^=(byte& l, byte r) noexcept { return l = (l ^ r); }

constexpr byte operator~(byte b) noexcept
{
    return byte(static_cast<std::uint8_t>(~to_underlying(b)));
}

// NOTE: backport of std::bit_cast (C++20)
template<typename TARGET_T, typename SOURCE_T>
inline typename std::enable_if<sizeof(TARGET_T) == sizeof(SOURCE_T)
                                   && std::is_trivially_copyable<SOURCE_T>::value
                                   && std::is_trivially_copyable<TARGET_T>::value,
                               TARGET_T>::type
bit_cast(const SOURCE_T& src) noexcept
{
    TARGET_T dst{};
    std::memcpy(&dst, &src, sizeof(TARGET_T));
    return dst;
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr bool get_bit(byte b, T pos) noexcept
{
    return ((b >> pos) & static_cast<byte>(1u)) == static_cast<byte>(1u);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void clear_bit(byte& b, T pos) noexcept
{
    b &= ~(static_cast<byte>(1u) << pos);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void set_bit(byte& b, T pos) noexcept
{
    b |= (static_cast<byte>(1u) << pos);
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void set_bit(byte& b, T pos, bool val) noexcept
{
    if (val)
    {
        set_bit(b, pos);
    }
    else
    {
        clear_bit(b, pos);
    }
}

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline constexpr void toggle_bit(byte& b, T pos) noexcept
{
    b ^= (static_cast<byte>(1u) << pos);
}

} // namespace ztl

#endif // ZTL_BYTE_HPP
